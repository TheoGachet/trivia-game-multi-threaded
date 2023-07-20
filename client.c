/* Code par Théo Gachet et Thimoté Dupuch - 2023 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "functions.h"
#include "client.h"
#include <fcntl.h>
#include <semaphore.h>

/* Variables globales permettant d'organiser la partie */

char server_message[2000];
int debut_partie = 0;
int fin_partie = 0;

/* Sémaphores fichiers et mutex pour les zones critiques */

sem_t question_recue;
pthread_mutex_t start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t start_cond = PTHREAD_COND_INITIALIZER;

/* Cette fonction s'exécute dans le thread client et reçoit les messages du serveur */

void *receive_messages(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    int read_size;

    while ((read_size = recv(sock, server_message, 2000, 0)) > 0)
    {
        /* Si on reçoit START, la partie commence */
        if (strcmp(server_message, "START") == 0)
        {
            printf("--- DEBUT DE LA PARTIE ---\n");
            pthread_mutex_lock(&start_mutex);
            debut_partie = 1;
            pthread_cond_signal(&start_cond);
            pthread_mutex_unlock(&start_mutex);
        }
        /* Si on reçoit STOP, la partie s'achève */
        else if (strcmp(server_message, "STOP") == 0)
        {
            printf("--- FIN DE LA PARTIE ---\n");
            pthread_mutex_lock(&start_mutex);
            fin_partie = 1;
            pthread_cond_signal(&start_cond);
            pthread_mutex_unlock(&start_mutex);
            /*close(sock);*/
            return 0;
        }
        /* Si on reçoit un message quelconque, c'est une question */
        else
        {
            sem_post(&question_recue);
        }
    }

    /* Gestion des erreurs */
    if (read_size == 0)
    {
        puts("Serveur déconnecté");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("Erreur de réception");
    }

    return 0;
}

/* Fonction de gestion du client */

int run_client(const char *ip, int port, int num_client)
{
    int sock;
    struct sockaddr_in server;
    pthread_t receive_thread;
    int *new_sock;
    char message[2000];
    char temps_reponse[2000];
    int nb_envoyes_client;
    time_t seconds;

    /* Ces sémaphores synchronisent l'envoi et la réception des 2 données (réponse + temps de réponse) des clients */
    sem_t *sem1;
    sem_t *sem2;
    sem_t *sem3;

    /* Ces sémaphores permettent de synchroniser l'ordre d'envoi des réponses des clients 2 et 3 */
    sem_t *passage2;
    sem_t *passage3;

    /* On utilise des sémaphores fichiers pour pouvoir les utiliser entre plusieurs programmes */
    sem1 = sem_open("/sem1", O_CREAT, 0644, 0);
    sem2 = sem_open("/sem2", O_CREAT, 0644, 0);
    sem3 = sem_open("/sem3", O_CREAT, 0644, 0);

    passage2 = sem_open("/passage2", O_CREAT, 0644, 0);
    passage3 = sem_open("/passage3", O_CREAT, 0644, 0);

    sem_init(&question_recue, 0, 0);

    /* La variable seconds est pseudo-aléatoire, utilisant time() et le PID du client */
    seconds = time(NULL)^getpid();

    /* Création du socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Erreur lors de la création du socket");
        return -1;
    }

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    /* Connexion au serveur distant */
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Echec de la connexion");
        return -1;
    }

    puts("Connecté au serveur");

    new_sock = malloc(1);
    *new_sock = sock;

    /* On créé un thread pour recevoir les messages du serveur */
    if (pthread_create(&receive_thread, NULL, receive_messages, (void *)new_sock) < 0)
    {
        perror("Erreur lors de la création du thread de réception");
        return -1;
    }

    printf("En attente du début de partie...\n");

    /* La partie débute lorsque l'on reçoit START du serveur */
    pthread_mutex_lock(&start_mutex);
    while (debut_partie == 0)
    {
        pthread_cond_wait(&start_cond, &start_mutex);
    }
    pthread_mutex_unlock(&start_mutex);

    /* Tant que l'on ne reçoit pas STOP du serveur, la partie continue */
    while (fin_partie == 0)
    {
        /* On attend de recevoir une question du serveur avant d'y répondre */
        sem_wait(&question_recue);
        sleep(1); /* synchronisation */

        /* Le client rentre dans une phase de réflexion de durée aléatoire */
        reflexion(seconds);

        /* Lorsque le client a fini de réflechir, il envoie buzz */
        strcpy(message, "");
        strcpy(message, reponse_aleatoire(seconds));
        printf("\nBUZZ -> Réponse proposée : %s\n", message);
        sprintf(temps_reponse, "%ld", reflexion(seconds));

        switch (num_client)
        {
        case 1:
            /* On indique au serveur qu'on va envoyer une info */
            sem_post(sem1);
            /* On envoie le temps de réponse */
            nb_envoyes_client = send(sock, temps_reponse, strlen(temps_reponse), 0);
            sleep(1);
            /* On indique au serveur qu'on va envoyer une info */
            sem_post(sem1);
            sleep(1);
            /* On envoie la réponse */
            nb_envoyes_client = send(sock, message, strlen(message), 0);
            printf("[ENVOI CLIENT 1]\n");
            break;
        case 2:
            /* Même fonctionnement pour le client 2 */
            sem_wait(passage2);
            sem_post(sem2);
            nb_envoyes_client = send(sock, temps_reponse, strlen(temps_reponse), 0);
            sleep(1);
            sem_post(sem2);
            sleep(1);
            nb_envoyes_client = send(sock, message, strlen(message), 0);
            printf("[ENVOI CLIENT 2]\n");
            break;
        case 3:
            /* Même fonctionnement pour le client 3 */
            sem_wait(passage3);
            sem_post(sem3);
            nb_envoyes_client = send(sock, temps_reponse, strlen(temps_reponse), 0);
            sleep(1);
            sem_post(sem3);
            sleep(1);
            nb_envoyes_client = send(sock, message, strlen(message), 0);
            printf("[ENVOI CLIENT 3]\n");
            break;
        }

        /* Gestion des erreurs */
        if (nb_envoyes_client < 0)
        {
            perror("Echec de l'envoi de la réponse");
            return -1;
        }
        else if (nb_envoyes_client == 0)
        {
            perror("reponse vide");
            return -1;
        }
    }
    return 0;
}

/* Fonction permettant de simuler la réflexion d'un joueur */

long int reflexion(int seconds)
{
    /* La fonction nanosleep() nécessite la création de cette structure */
    struct timespec ts;
    long int temps_reflexion;

    /* On génère une durée de réflexion aléatoire à l'aide de rand() et de seconds définie précédemment */
    ts.tv_sec = abs((rand() + seconds) % 8 + 1);
    ts.tv_nsec = (int)abs((rand() + seconds) % 1000000000);
    temps_reflexion = abs((rand() + seconds) % 100000 + 1);

    nanosleep(&ts, NULL);
    return (temps_reflexion * 10000);
}