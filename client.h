/* Code par Théo Gachet et Thimoté Dupuch - 2023 */

#ifndef CLIENT_H
#define CLIENT_H

#include <time.h>

void *receive_messages(void *socket_desc);
int run_client(const char *ip, int port, int num_client);
long int reflexion(int seconds);

#endif
