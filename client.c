/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */
#include "csapp.h"

#define DO_NOT_PRINT "\xEA" //Capitol omega, should never be printed

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

    while ((n = Rio_readnb(&rio, buf, 1)) != 0) { // Read one byte/char
        /* Check if we need to send a response */
        if (buf[0] != '\xEA') {
            printf("%c", buf[0]); //Send character to stdout
        } else {
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
