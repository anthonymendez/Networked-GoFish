/* 
 * echoservert.c - A concurrent echo server using threads
 */
/* $begin echoservertmain */
#include "csapp.h"
#include "gofish.h"
#include "server.h"

void echo(int connfd) {
    int n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    /* Waits for message from client, then writes same message back to client */
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", n);
        sendStringToClient(connfd, buf);
    }

    do {
        game_start(connfd);
        do {
            /* Play a round */
            if (game_loop(connfd)) {
                break;  /* If there is a winner, go to game_end */
            }
            current = next_player;
        } while(1);
    } while(game_end(connfd));
    Rio_writen(connfd, "Exiting\n", strlen(buf));
    printf("Exiting GoFish\n");
}

int main(int argc, char **argv) {
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid; 

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    playerCount = 0;
    listenfd = Open_listenfd(argv[1]);

    while (1) {
        clientlen=sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int)); //line:conc:echoservert:beginmalloc
        *connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen); //line:conc:echoservert:endmalloc
        Pthread_create(&tid, NULL, thread, connfdp);
    }
}

/* Thread routine */
void *thread(void *vargp) {
    int connfd = *((int *)vargp);

    playerCount++;
    printf("@Player%d joined the game\n", playerCount);
    printf("Ready to start!\n");
    /* Setting seed for random number generator */
    srand(time(NULL));

    Pthread_detach(pthread_self()); //line:conc:echoservert:detach
    Free(vargp);                    //line:conc:echoservert:free
    echo(connfd);

    printf("Player has left the game\n");
    playerCount--;

    Close(connfd);
    return NULL;
}

void sendStringToClient(int connfd, char *stringToClient) {
    return Rio_writen(connfd, stringToClient, strlen(stringToClient));
}
