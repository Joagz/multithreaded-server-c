
#include <arpa/inet.h> // inet_addr()
#include <bits/pthreadtypes.h>
#include <linux/limits.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <pthread.h>

#define SA struct sockaddr
#define SERVER_ADDRESS "127.0.0.1"
#define PORT 8989
#define MAX 8192

void func(int connfd, char*path){

    char buff[MAX];

    write(connfd, path, strlen(path));
    bzero(buff, strlen(buff));

    read(connfd, buff, strlen(buff));

    printf("CONNECTION COMPLETED\n");

    close(connfd);
    
}


int Client(char ** pstr){
    
    char * args = *pstr;
    free(pstr);

    int sockfd; 
    struct sockaddr_in servaddr;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        return -1;
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
            != 0) {
        perror("connection with the server failed...\n");
        return -1;
    }
    func(sockfd, args);

    // close the socket
    close(sockfd);

    return 0;

}


int main(){
    
    int i = 0;

    while(i<100){

        pthread_t t;
        char ** pstr = malloc(sizeof(char) * PATH_MAX);
        *pstr = "./server.c";
        pthread_create(&t, NULL, Client, pstr);

        i++;
    }

}
