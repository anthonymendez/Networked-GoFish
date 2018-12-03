/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */
#include "csapp.h"

#define DO_NOT_PRINT "DO_NOT_PRINT"

int main(int argc, char **argv) 
{
    int clientfd, n;
    char *host, *port, *buf;
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];
    buf = malloc(MAXLINE * sizeof(char));

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        /* Check if we need to send a response */
        if (strstr(buf, DO_NOT_PRINT) == NULL) {
            printf("NOTFOUND: %s", buf);
        } else {
            // TODO: Why is it getting stuck here?
            printf("DO_NOT_PRINT IS HERE\n");
            printf("B:%s", buf);
            printf("A:%s", buf + strlen(DO_NOT_PRINT));
            /* Wait until user gives input */
            if (Fgets(buf, MAXLINE, stdin) != NULL) {
                Rio_writen(clientfd, buf, strlen(buf));
            } else {
                break;
            }
        }

        /* Clear buffer */
        for(int i = 0; i < MAXLINE; i++) {
            buf[i] = 0;
        }
    }
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}
/* $end echoclientmain */
