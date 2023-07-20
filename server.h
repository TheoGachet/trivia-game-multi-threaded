/* Code par Théo Gachet et Thimoté Dupuch - 2023 */

#ifndef SERVER_H
#define SERVER_H


typedef struct {
    int fd;      /* File descriptor pour le socket du client */
    int score;   /* Score du client */
    int buzzer;  /* Etat du buzzer du client (0 = éteint / 1 = allumé) */
    int etat;    /* Envoyeur, Receveur, En attente */
} Client;

void *connection_handler(void *socket_desc);
void run_server(int port);
void send_to_all_clients(char *message);
void poser_question();
void bloquer_buzzer(Client *client);
void liberer_buzzer(Client *client);
void modif_score(Client *client, int valeur);
void init_client(Client *C);
int verif_reponse(char *reponse, char *reponse_client, Client *client);
void affichage_scores(Client *client1, Client *client2, Client *client3);

#endif
