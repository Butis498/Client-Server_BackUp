
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include<string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/stat.h>
#include <syslog.h>

#include <netdb.h>
#include <sys/socket.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

char *readFile(const char *fileName)
{

    FILE *file = fopen(fileName, "r");
    char *code;
    size_t n = 0;
    int c;

    if (file == NULL)
        return NULL; //could not open file
    fseek(file, 0, SEEK_END);
    long f_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    code = (char *)malloc(f_size);

    while ((c = fgetc(file)) != EOF)
    {
        code[n++] = (char)c;
    }

    code[n] = '\0';

    return code;
}

void sendCreateFileOrModifyPetition(const char * fileName, const char * fileContet, const char *directory){

}

void sendDeleteFilePetition(const char *fileName, const char *directory){

}

void sendDeleteDirectoryPetition(const char *directory){

}

void sendCreateDirectoryPetition(const char *directory , const char * dirName){

}

void sendRenameDirectoryPetition(const char *directory, const char* oldName ,const char * newName){

}

void sendRenameFilePetition(const char *directory, const char* oldName ,const char * newName){

}



void func(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;)
    {
        
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;

        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0)
        {
            printf("Client Exit...\n");
            break;
        }
    }
}


void client(){

        int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    // function for chat
    func(sockfd);

    // close the socket
    close(sockfd);

    syslog(LOG_NOTICE, "First daemon terminated.");
    closelog();

}

    

