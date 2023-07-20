/* Code par Théo Gachet et Thimoté Dupuch - 2023 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "server.h"
#include "functions.h"
#include <fcntl.h>
#include <semaphore.h>

int main(int argc, char *argv[])
{
    int port;
    char *server_ip;
    int num_client;
    
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
    
    /* On gère les commandes saisies sur le terminal */

    if (argc < 2)
    {
        printf("Usage: %s <mode>\n", argv[0]);
        printf("Modes:\n");
        printf("\tserver <port>\n");
        printf("\tclient <server_ip> <port>\n");
        return 1;
    }

    if (strcmp(argv[1], "server") == 0)
    {
        if (argc != 3)
        {
            handle_error("Server mode requires port argument");
        }

        port = atoi(argv[2]);
        run_server(port);
    }
    else if (strcmp(argv[1], "client") == 0)
    {
        if (argc != 5)
        {
            handle_error("Client mode requires server_ip and port arguments");
        }

        server_ip = argv[2];
        port = atoi(argv[3]);

        num_client = atoi(argv[4]);
        
        run_client(server_ip, port, num_client);
    }
    else
    {
        handle_error("Invalid mode");
    }

    /* Jusqu'à la fin du code, on ferme juste les sémaphores fichiers ouverts ci-dessus */
    if (sem_close(sem1) < 0) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
    if (sem_unlink("/sem1") < 0) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem2) < 0) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
    if (sem_unlink("/sem2") < 0) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem3) < 0) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
    if (sem_unlink("/sem3") < 0) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
    if (sem_close(passage2) < 0) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
    if (sem_unlink("/passage2") < 0) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
    if (sem_close(passage3) < 0) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
    if (sem_unlink("/passage3") < 0) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

    return 0;
}
