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

#define MAX 2000
//#define PORT 8080
#define SA struct sockaddr

int PORT = 8080;
char *HOST = "127.0.0.1";

void clientSendUpdate(const char *instruccion, const char *fileContents)
{

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
    servaddr.sin_addr.s_addr = inet_addr(HOST);
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        syslog(LOG_NOTICE, "ERROR: connection with the server %s failed...\n", HOST);
        exit(0);
    }
    else
        syslog(LOG_NOTICE, "SUCCES: connected to the server..\n");

    //Sending data
    //==============================================================================

    char buff[MAX];
    int n;

    bzero(buff, sizeof(buff));

    n = 0;

    strcpy(buff, instruccion);

    syslog(LOG_NOTICE, "CLIENT: Sending Buffer of contents: %s\n", buff);

    int numbersWritten = write(sockfd, buff, sizeof(buff));

    syslog(LOG_NOTICE, "CLIENT: Number of bytes written in write(): %d\n", numbersWritten);
    bzero(buff, sizeof(buff));

    //sleep(1);
    //read(sockfd, buff, sizeof(buff));
    int res = 0;
    do{
        //sleep(0.01);
        bzero(buff, sizeof(buff));
        res = read(sockfd, buff, sizeof(buff));
    }while (res = 0 || buff == 0 || buff[0] == '\0');

    syslog(LOG_NOTICE, "CLIENT: Response from Server : %s", buff);

    bzero(buff, sizeof(buff));

    //when thre are file contents to send
    if (fileContents != NULL)
    {

        strcpy(buff, fileContents);
        syslog(LOG_NOTICE, "CLIENT: Sending File Contents: %s\n", buff);

        int numbersWritten = write(sockfd, buff, sizeof(buff));

        syslog(LOG_NOTICE, "CLIENT: Number of bytes written in write(): %d\n", numbersWritten);
        bzero(buff, sizeof(buff));

        int res = 0;
        do
        {
            //sleep(0.01);
            bzero(buff, sizeof(buff));
            res = read(sockfd, buff, sizeof(buff));
        } while (res = 0 || buff == 0 || buff[0] == '\0');

        syslog(LOG_NOTICE, "CLIENT: Response2 from Server : %s", buff);
        bzero(buff, sizeof(buff));
    }

    // close the socket
    close(sockfd);

    //syslog(LOG_NOTICE, "Client daemon terminated.");
    syslog(LOG_NOTICE, "SERVER EVENT FINISHED\n");
    closelog();
}

char *readFile(const char *fileName)
{

    FILE *file = fopen(fileName, "r");
    char *code;
    size_t n = 0;
    int c;

    if (file == NULL)
    {

        return NULL; //could not open file
    }

    fseek(file, 0, SEEK_END);
    long f_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    code = (char *)malloc(f_size);
    syslog(LOG_NOTICE, "Holaaaaa");

    while ((c = fgetc(file)) != EOF)
    {
        code[n++] = (char)c;
    }

    code[n] = '\0';

    return code;
}

void sendCreateFilePetition(const char *fileName, const char *directory)
{

    char *instruccion = (char *)malloc((strlen(fileName) + strlen(directory) + strlen("createFile") + 6) * sizeof(char));
    sprintf(instruccion, "createFile %s %s", fileName, directory);
    clientSendUpdate(instruccion, NULL);
}

//fileName the name of the modified file: eg. archivo.txt
//directory the name of the containing directory. eg. MonitoredClientFolder/NEW FOLDER/NEW FOLDER 2
void sendModifyFilePetition(const char *fileName, const char *fileContet, const char *directory)
{

    //convert the content lenght to a string
    long contentLenght = strlen(fileContet) + 1;
    char contLenght[32];

    sprintf(contLenght, "%ld", contentLenght);

    char *instruccion = (char *)malloc((strlen(contLenght) + strlen(fileName) + strlen(directory) + strlen("modifyFile") + 6) * sizeof(char));
    sprintf(instruccion, "modifyFile %s %s %s", contLenght, fileName, directory);

    syslog(LOG_NOTICE, "MODIFY PETITION TO BE SENT.\n");

    char *contents = (char *)malloc((strlen(fileContet) + 2) * sizeof(char));
    sprintf(contents, "$%s", fileContet);

    clientSendUpdate(instruccion, contents);
}

void sendDeleteFilePetition(const char *fileName, const char *directory)
{
    char *instruccion = (char *)malloc((strlen(fileName) + strlen(directory) + strlen("delete") + 6) * sizeof(char));
    sprintf(instruccion, "delete %s %s", fileName, directory);
    clientSendUpdate(instruccion, NULL);
}

void sendDeleteDirectoryPetition(const char *directory)
{

    char *instruccion = (char *)malloc((strlen(directory) + strlen("deleteDir") + 3) * sizeof(char));
    sprintf(instruccion, "deleteDir %s", directory);
    clientSendUpdate(instruccion, NULL);
}

void sendCreateDirectoryPetition(const char *directory, const char *dirName)
{
    syslog(LOG_NOTICE, "CREATE DIR PETITION TO BE SENT: %s/%s\n", directory, dirName);

    char *instruccion = (char *)malloc((strlen(directory) + strlen(dirName) + strlen("createDir") + 6) * sizeof(char));
    sprintf(instruccion, "createDir %s /%s", directory, dirName);
    clientSendUpdate(instruccion, NULL);
}

//main function for testing purposes
/*
int main(){
    
    //client();

    return 0;
}
*/
