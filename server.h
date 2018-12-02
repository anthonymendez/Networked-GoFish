#ifndef SERVER_H
#define SERVER_H

int playerCount;

void echo(int connfd);
void *thread(void *vargp);
void sendStringToClient(int connfd, char *stringToClient);

#endif
