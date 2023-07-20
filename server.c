/* Code par Théo Gachet et Thimoté Dupuch - 2023 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "server.h"
#include <fcntl.h>
#include <semaphore.h>

/* On définit ici les paramètres de la partie : */

#define NB_QUESTIONS 2
#define MAX_LENGTH 2000
#define MAX_CLIENTS 3
#define ENVOYEUR 1
#define RECEVEUR 2
#define EN_ATTENTE 3

/* Déclaration de variables globales pour le stockage des sockets clients et l'initialisation d'un mutex pour la synchronisation des threads */

int client_socks[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ----------------- Fonctions de gestion du serveur ----------------- */

/* Fonction de gestion des connexions pour chaque client (exécutée dans un nouveau thread pour chaque client) */

void *connection_handler(void *socket_desc)
{
    /* On récupère le descripteur de socket */
    int sock = *(int *)socket_desc;
    int read_size;
    char client_message[2000];
    char message_serveur[2000];
    int etat_client;

    etat_client = EN_ATTENTE;

    while (1)
    {
        /* Si le client est l'envoyeur, on lit le message reçu et on l'affiche */
        if (etat_client == ENVOYEUR)
        {
            while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
            {
                if (read_size == -1)
                {
                    perror("Erreur reception client");
                }
                printf("Message reçu : %s\n", client_message);
                etat_client = EN_ATTENTE;
            }
        }
        /* Si le client est le receveur, le serveur va envoyer un message */
        if (etat_client == RECEVEUR)
        {
            printf("Le serveur va envoyer un message au client %d \n Entrez un message : ", sock);
            fgets(message_serveur, 2000, stdin);
            send_to_all_clients(message_serveur);
            etat_client = EN_ATTENTE;
        }
    }
    /* Libération de la mémoire allouée pour le descripteur de socket */
    free(socket_desc);
    return 0;
}

/* Fonction principale pour l'exécution du serveur */

void run_server(int port)
{
    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client;
    pthread_t thread_id;
    Client clients_connectes[MAX_CLIENTS];

    /* On initialise les instances des clients */
    init_client(&clients_connectes[0]);
    init_client(&clients_connectes[1]);
    init_client(&clients_connectes[2]);

    /* Création du socket et gestion des erreurs */
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        puts("Impossible de créer le socket");
        return;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    /* Bind (lier le socket à un port sur l'ordinateur) */
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Bind échoué");
        return;
    }

    /* Le serveur ommence à écouter les connexions entrantes */
    listen(socket_desc, 3);
    puts("En attente de connexions entrantes...");
    c = sizeof(struct sockaddr_in);

    /* On accepte et on traite les connexions entrantes */
    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        int i;
        puts("Connexion acceptée");
        pthread_mutex_lock(&clients_mutex);
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_socks[i] == 0)
            {
                client_socks[i] = client_sock;
                /*printf("Ajout du client à l'indice %d\n", i);*/
                clients_connectes[i].fd = client_sock;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (i == 2)
        { /* 2 représente ici 3 clients */
            sleep(1);
            printf("3 clients connectés, début de la partie \n");
            printf("--------------------------\n");
            send_to_all_clients("START");
            sleep(2);

            /* Début du quizz */
            for (i = 0; i < NB_QUESTIONS; i++)
            {
                printf("\n--- NOUVELLE QUESTION ---\n");
                poser_question(&clients_connectes[0], &clients_connectes[1], &clients_connectes[2]);
            }
            printf("\n--- FIN DE LA PARTIE ---\n");
        }

        if (i == MAX_CLIENTS)
        {
            puts("Nombre maximum de clients atteint");
            close(client_sock);
            continue;
        }

        /* Crée un nouveau thread pour chaque connexion acceptée */
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&thread_id, NULL, connection_handler, (void *)new_sock) < 0)
        {
            perror("Erreur lors de la création du thread");
            return;
        }

        /*puts("Handler attribué");*/
    }

    if (client_sock < 0)
    {
        perror("Erreur accept");
        return;
    }
}

/* Fonction permettant d'envoyer un message à tous les clients (comme une question) */

void send_to_all_clients(char *message)
{
    int i;
    pthread_mutex_lock(&clients_mutex);

    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_socks[i] != 0)
        {
            int client_sock = client_socks[i];
            int write_size = write(client_sock, message, strlen(message));

            if (write_size == -1)
            {
                perror("Erreur d'envoi au client");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* ------ Fonctions propres à l'animateur qui pose les questions ------ */

/* Fonction qui pose une question aux clients et gère leurs réponses */

void poser_question(Client *client_1, Client *client_2, Client *client_3)
{
    char question[MAX_LENGTH];
    char reponse[MAX_LENGTH];
    char reponse_client_1[MAX_LENGTH];
    char reponse_client_2[MAX_LENGTH];
    char reponse_client_3[MAX_LENGTH];
    char temps_client_1[MAX_LENGTH];
    char temps_client_2[MAX_LENGTH];
    char temps_client_3[MAX_LENGTH];
    char ch;
    long int int_temps_client_1;
    long int int_temps_client_2;
    long int int_temps_client_3;
    int currentLine;
    int numLines;
    int randomLine;
    int question_terminee;
    int premier_buzz;

    FILE *questionsFile;
    FILE *reponsesFile;

    /* Ces sémaphores synchronisent l'envoi et la réception des 2 données (réponse + temps de réponse) des clients */
    sem_t *sem1;
    sem_t *sem2;
    sem_t *sem3;

    sem_t *passage2; /* Ce sémaphore permet de synchroniser l'ordre d'envoi des réponses du client 2 */
    sem_t *passage3; /* Ce sémaphore permet de synchroniser l'ordre d'envoi des réponses du client 3 */

    /* On utilise des sémaphores fichiers pour pouvoir les utiliser entre plusieurs programmes */
    sem1 = sem_open("/sem1", O_CREAT, 0644, 0);
    sem2 = sem_open("/sem2", O_CREAT, 0644, 0);
    sem3 = sem_open("/sem3", O_CREAT, 0644, 0);

    passage2 = sem_open("/passage2", O_CREAT, 0644, 0);
    passage3 = sem_open("/passage3", O_CREAT, 0644, 0);

    /* On vide les réponses clients avant chaque question, pour éviter les reliquats */
    strcpy(reponse_client_1, "");
    strcpy(reponse_client_2, "");
    strcpy(reponse_client_2, "");

    /* On ouvre les bases de connaissances */
    questionsFile = fopen("questions.txt", "r");
    reponsesFile = fopen("reponses.txt", "r");

    if (questionsFile == NULL || reponsesFile == NULL)
    {
        printf("Erreur lors de l'ouverture des fichiers.\n");
        return;
    }

    /* On compte le nombre de lignes dans le fichier des questions */
    numLines = 0;
    while ((ch = fgetc(questionsFile)) != EOF)
    {
        if (ch == '\n')
        {
            numLines++;
        }
    }

    /* On réinitialise du curseur de lecture au début du fichier */
    rewind(questionsFile);
    rewind(reponsesFile);

    /* On rénère d'un nombre aléatoire entre 0 et numLines-1 */
    srand(time(NULL));
    randomLine = rand() % numLines;

    /* On lit la question et la réponse correspondante */

    currentLine = 0;

    while (fgets(question, sizeof(question), questionsFile) != NULL &&
           fgets(reponse, sizeof(reponse), reponsesFile) != NULL)
    {
        if (currentLine == randomLine)
        {
            /* Suppression du saut de ligne à la fin de la question et de la réponse */
            question[strcspn(question, "\n")] = '\0';
            reponse[strcspn(reponse, "\n")] = '\0';

            /* Avant le début d'une question, on libère les buzzers */
            liberer_buzzer(client_1);
            liberer_buzzer(client_2);
            liberer_buzzer(client_3);

            printf("[BUZZERS LIBÉRÉS]\n");

            /* Envoi de la question aux clients */
            printf("[QUESTION] %s\n", question);

            /* Cette variable contrôle la sortie de la boucle ci-dessous */
            question_terminee = 0;

            while (question_terminee == 0)
            {
                /* On envoie la question aux clients et on attend des réponses*/
                send_to_all_clients(question);
                printf("On attend les réponses...\n");

                if (client_1->buzzer == 1)
                {
                    /* On attend que le client soit en mesure d'envoyer son temps de réponse */
                    sem_wait(sem1);
                    recv(client_1->fd, temps_client_1, MAX_LENGTH, 0);
                    int_temps_client_1 = strtol(temps_client_1, (char **)NULL, 10);
                    printf("\n-> [RECEP] Temps de réflexion client 1 : %ld", int_temps_client_1);

                    /* On attend que le client soit en mesure d'envoyer sa réponse */
                    sem_wait(sem1);
                    recv(client_1->fd, reponse_client_1, MAX_LENGTH, 0);
                    printf("\n-> [RECEP] Réponse client 1 : %s\n", reponse_client_1);
                }

                /* On assure la synchronisation à l'aide du sémaphore passage2 */
                sleep(1);
                sem_post(passage2);

                if (client_2->buzzer == 1)
                {
                    /* On attend que le client soit en mesure d'envoyer son temps de réponse */
                    sem_wait(sem2);
                    recv(client_2->fd, temps_client_2, MAX_LENGTH, 0);
                    int_temps_client_2 = strtol(temps_client_2, (char **)NULL, 10);
                    printf("\n-> [RECEP] Temps de réflexion client 2 : %ld", int_temps_client_2);

                    /* On attend que le client soit en mesure d'envoyer sa réponse */
                    sem_wait(sem2);
                    recv(client_2->fd, reponse_client_2, MAX_LENGTH, 0);
                    printf("\n-> [RECEP] Réponse client 2 : %s\n", reponse_client_2);
                }

                /* On assure la synchronisation à l'aide du sémaphore passage3 */
                sleep(1);
                sem_post(passage3);

                if (client_3->buzzer == 1)
                {
                    /* On attend que le client soit en mesure d'envoyer son temps de réponse */
                    sem_wait(sem3);
                    recv(client_3->fd, temps_client_3, MAX_LENGTH, 0);
                    int_temps_client_3 = strtol(temps_client_3, (char **)NULL, 10);
                    printf("\n-> [RECEP] Temps de réflexion client 3 : %ld", int_temps_client_3);

                    /* On attend que le client soit en mesure d'envoyer sa réponse */
                    sem_wait(sem3);
                    recv(client_3->fd, reponse_client_3, MAX_LENGTH, 0);
                    printf("\n-> [RECEP] Réponse client 3 : %s\n", reponse_client_3);
                }

                /* On gère désormais la chronologie des réponses reçues */
                
                if ((client_1->buzzer == 0) && (client_2->buzzer == 0) && (client_3->buzzer == 0))
                {
                    printf("TOUS LES BUZZERS SONT ETEINTS\n");
                    question_terminee = 1;
                }
                else /* On cherche à savoir quel client (au buzzer activé) a été le premier a buzzer */
                {
                    if ((client_1->buzzer == 1) && ((int_temps_client_1 < int_temps_client_2) || (client_2->buzzer == 0)) && ((int_temps_client_1 < int_temps_client_3) || (client_3->buzzer == 0)))
                    {
                        premier_buzz = 1;
                    }
                    else if ((client_2->buzzer == 1) && ((int_temps_client_2 < int_temps_client_1) || (client_1->buzzer == 0)) && ((int_temps_client_2 < int_temps_client_3) || (client_3->buzzer == 0)))
                    {
                        premier_buzz = 2;
                    }
                    else if ((client_3->buzzer == 1) && ((int_temps_client_3 < int_temps_client_1) || (client_1->buzzer == 0)) && ((int_temps_client_3 < int_temps_client_2) || (client_2->buzzer == 0)))
                    {
                        premier_buzz = 3;
                    }

                    /* Une fois que l'on a l'info du premier qui a buzzé, on vérifie sa réponse */
                    switch (premier_buzz)
                    {
                    case 1:
                        printf("\nLe client 1 a buzzé en premier -> %s\n", reponse_client_1);
                        question_terminee = verif_reponse(reponse, reponse_client_1, client_1);
                        break;
                    case 2:
                        printf("\nLe client 2 a buzzé en premier -> %s\n", reponse_client_2);
                        question_terminee = verif_reponse(reponse, reponse_client_2, client_2);
                        break;
                    case 3:
                        printf("\nLe client 3 a buzzé en premier -> %s\n", reponse_client_3);
                        question_terminee = verif_reponse(reponse, reponse_client_3, client_3);
                        break;
                    }
                }

                /* On efface les réponses précédentes pour éviter tout reliquat */
                strcpy(reponse_client_1, "");
                strcpy(reponse_client_2, "");
                strcpy(reponse_client_3, "");

                /* On affiche les scores et l'avancement de la partie */
                affichage_scores(client_1, client_2, client_3);
            }
            printf("\n--- QUESTION TERMINEE ---\n");
            break;
        }
        currentLine++;
    }

    /* Fermeture des fichiers */
    fclose(questionsFile);
    fclose(reponsesFile);

    /* On bloque les buzzers de tous les clients */
    bloquer_buzzer(client_1);
    bloquer_buzzer(client_2);
    bloquer_buzzer(client_3);
}

/* On vérifie que la réponse envoyée par un client est bonne et on gère le score et le buzzer en conséquence */

int verif_reponse(char *reponse, char *reponse_client, Client *client)
{
    reponse_client[strcspn(reponse_client, "\n")] = '\0';
    reponse[strcspn(reponse, "\n")] = '\0';

    if (strncmp(reponse, reponse_client, strlen(reponse)) == 0)
    {
        printf("[BONNE REPONSE] +5 points");
        modif_score(client, 5);
        return 1;
    }
    else
    {
        printf("[MAUVAISE REPONSE] -1 point");
        modif_score(client, -1);
        bloquer_buzzer(client);
        return 0;
    }
}

/* ----------------- Fonctions de gestion des clients ----------------- */

/* Initialisation d'un client */

void init_client(Client *C)
{
    C->buzzer = 0;
    C->score = 0;
    C->fd = 0;
}

/* On bloque le buzzer du client, il ne peut plus répondre */

void bloquer_buzzer(Client *client)
{
    client->buzzer = 0;
    printf("[BUZZER] bloqué \n");
}

/* On libère le buzzer du client, il peut à nouveau répondre */

void liberer_buzzer(Client *client)
{
    client->buzzer = 1;
}

/* On modifie le score d'un client selon la valeur en paramètre */

void modif_score(Client *client, int valeur)
{
    client->score += valeur;
    printf(" (nouveau score = %d)\n", client->score);
}

/* Cette fonction affiche les scores et établit le classement des joueurs au cours de la partie */

void affichage_scores(Client *client1, Client *client2, Client *client3)
{
    int premier;
    int deuxieme;
    int troisieme;

    /* Beaucoup de code pour pas grand chose mais c'est beau */
    if ((client1->score >= client2->score) && (client1->score >= client3->score))
    {
        premier = 1;
        if (client2->score >= client3->score)
        {
            deuxieme = 2;
            troisieme = 3;
        }
        else
        {
            deuxieme = 3;
            troisieme = 2;
        }
    }
    else if ((client2->score >= client1->score) && (client2->score >= client3->score))
    {
        premier = 2;
        if (client1->score >= client3->score)
        {
            deuxieme = 1;
            troisieme = 3;
        }
        else
        {
            deuxieme = 3;
            troisieme = 1;
        }
    }
    else
    {
        premier = 3;
        if (client2->score > client1->score)
        {
            deuxieme = 2;
            troisieme = 1;
        }
        else
        {
            deuxieme = 1;
            troisieme = 2;
        }
    }

    /* Affichage du classement et des scores à l'instant t */
    
    printf("\n--- CLASSEMENT ---\n");
    printf("Le joueur %d est en tête, suivi du joueur %d puis du joueur %d\n", premier, deuxieme, troisieme);

    printf("\n--- SCORES ---\n");
    printf("Joueur 1 : %d\n", client1->score);
    printf("Joueur 2 : %d\n", client2->score);
    printf("Joueur 3 : %d\n\n", client3->score);
}
