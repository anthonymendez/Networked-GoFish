/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */
#include "csapp.h"

#define DO_NOT_PRINT "DO_NOT_PRINT\n"

int main(int argc, char **argv) 
{
    int clientfd, n;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        /* Check if we need to send a response */
        if (strcmp(buf, DO_NOT_PRINT) != 0)
            printf("%s", buf);
        if (strcmp(buf, DO_NOT_PRINT) == 0) {
            /* Wait until user gives input */
            if (Fgets(buf, MAXLINE, stdin) != NULL) {
                Rio_writen(clientfd, buf, strlen(buf));
            } else {
                break;
            }
        }
    }
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}
/* $end echoclientmain */
