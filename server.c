
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define SERVER_PORT 8989
#define BUFSIZE 8192
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void * handle_connection(void *p_client_socket);
int check(int exp, const char *msg);

int main(int argc, char **argv) {

    int server_socket, client_socket, addr_size;

    SA_IN server_addr, client_addr;

    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)),
            "FAILED TO CREATE SOCKET");

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_family = AF_INET;

    int optval = 1;

    check(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)), "FAILED SETTING OPTION (SO_REUSEADDR)");

    check(bind(server_socket, (SA*) &server_addr, sizeof(server_addr)),
            "BINDING FAILED");

    check(listen(server_socket, SERVER_BACKLOG),
            "LISTENING FAILED");

    // Run forever
    for(;;) {

        printf("AWAITING CONNECTION IN PORT %d...\n", SERVER_PORT);

        addr_size = sizeof(SA_IN);
        check(client_socket = accept(server_socket, (SA*)&client_addr,
                    (socklen_t*) &addr_size), "ACCEPT FAILED");

        printf("CONNECTION FROM %s\n", inet_ntoa(client_addr.sin_addr));

        pthread_t t;
        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;

        pthread_create(&t, NULL, handle_connection, pclient);

    }

    return 0;

}

int check(int exp, const char *msg) {
    if(exp == SOCKETERROR){
        perror(msg);
        exit(1);
    }
    return exp;
}

void * handle_connection(void *p_client_socket) {


    int client_socket = *((int*)p_client_socket);
    free(p_client_socket); // Don't need it anymore

    char * buffer = (char*) malloc(BUFSIZE);
    size_t bytes_read;
    int msgsize = 0;
    char actualpath[PATH_MAX];

    bzero(&actualpath, sizeof(actualpath));

    while((bytes_read = read(client_socket, buffer, BUFSIZE)) > 0) {
        msgsize += bytes_read;
        printf("READING BYTES (%zu)\n", bytes_read);
        if(msgsize > BUFSIZE-1 || buffer[msgsize-1] == '\n') break;
    }

    check(bytes_read, "RECV ERROR");
    fflush(stdout);

   if(realpath(buffer, actualpath) == NULL){
       printf("ERROR(bad path); %s\n", buffer);
       close(client_socket);
       return NULL;
   }

    FILE *fp = fopen(actualpath, "r");
    if(fp == NULL) {
        printf("ERROR(open): %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    sleep(1);
    printf("ACTUAL PATH: %s\n",actualpath);

    while((bytes_read = fread(buffer, 1, BUFSIZE-bytes_read, fp)) > 0){
        printf("WRITING %zu BYTES TO CLIENT\n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }

    printf("FREEING (THIS IS DEBUG ONLY)\n");
    free(buffer);
    close(client_socket);
    fclose(fp);
    printf("SUCCESSFULLY SENT FILES TO CLIENT\nCLOSING CONNECTION.\n");

    return NULL;
}
