#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
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

void clientSendUpdate(const char *instruccion){

    //Server connection
    //==============================================================================

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        syslog(LOG_NOTICE, "ERROR: socket creation failed...\n");
        exit(0);
    }
    
    syslog(LOG_NOTICE, "SUCCES: Socket successfully created..\n");

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        syslog(LOG_NOTICE, "ERROR: connection with the server failed...\n");
        exit(0);
    }
    else
        syslog(LOG_NOTICE, "SUCCES: connected to the server..\n");

    //Sending data
    //==============================================================================

    char buff[MAX];
    int n;
        
        bzero(buff, sizeof(buff));
        //printf("Enter the string : ");
        n = 0;
        //while ((buff[n] = instruccion[n++]) != '\n')
            //;
        strcpy(buff, instruccion);

        //write(sockfd, buff, sizeof(buff));
        syslog(LOG_NOTICE,"CLIENT: Sending Buffer of contents: %s\n", buff);

        int numbersWritten = write(sockfd, buff, sizeof(buff));

        bzero(buff, sizeof(buff));    

    //int numbersWritten = write(sockfd, instruccion, sizeof(instruccion));
    syslog(LOG_NOTICE, "CLIENT: Number of bytes written in write(): %d\n", numbersWritten);

    // close the socket
    close(sockfd);

    //syslog(LOG_NOTICE, "Client daemon terminated.");
    syslog(LOG_NOTICE, "EVENT SENT SO SERVER\n");
    closelog();
}

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

void sendCreateFilePetition(const char * fileName, const char *directory){

    char *instruccion = (char *) malloc((strlen(fileName)+strlen(directory)+strlen("createFile")+ 6)* sizeof(char));
    sprintf(instruccion,"createFile %s %s",fileName,directory);
    clientSendUpdate(instruccion);

}

void sendModifyFilePetition(const char * fileName, const char * fileContet, const char *directory){

}

void sendDeleteFilePetition(const char *fileName, const char *directory){
    char *instruccion = (char *) malloc((strlen(fileName)+strlen(directory)+strlen("delete")+ 6)* sizeof(char));
    sprintf(instruccion,"delete %s %s",fileName,directory);
    clientSendUpdate(instruccion);
}

void sendDeleteDirectoryPetition(const char *directory){

    char *instruccion = (char *) malloc((strlen(directory)+strlen("deleteDir")+3)* sizeof(char));
    sprintf(instruccion,"deleteDir %s",directory);
    clientSendUpdate(instruccion);

}

void sendCreateDirectoryPetition(const char *directory , const char * dirName){

    char *instruccion = (char *) malloc((strlen(directory)+strlen("createDir")+ 6)* sizeof(char));
    sprintf(instruccion,"createDir %s /%s",directory,dirName);
    clientSendUpdate(instruccion);

}

void sendRenameDirectoryPetition(const char *directory, const char* oldName ,const char * newName){

}

void sendRenameFilePetition(const char *directory, const char* oldName ,const char * newName){

}


/*
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
        //if ((strncmp(buff, "exit", 4)) == 0)
        if ((strncmp(buff, "operacionExitosa", 16)) == 0)
        {
            printf("\noperacionExitosa... Client Exit...\n");
            break;
        }
    }
}


//main function for testing purposes
/*
int main(){
    
    //client();

    return 0;
}
*/



