
#include "fileManager.c"
#define MAX 200
#define PORT 8080
#define SA struct sockaddr
char *folderName = "UpdatedServerFolder";

void splitArgs(const char *string, char *args[])
{
    // Make a local copy of the string that we can manipulate.
    char *const copy = strdup(string);
    char *space = copy;
    int i = 1;
    // Find the next space in the string, and replace it with a newline.
    args[0] = copy;
    while (space = strchr(space, ' '))
    {

        *space = 0;
        space++;

        args[i] = space;
        i++;
    }
}

// Function designed for chat between client and server.
void func(int sockfd)
{
    char buff[MAX];
    int n;
    // infinite loop for chat
    for (;;)
    {
        bzero(buff, MAX);

        printf("Control 1 \n");

        // read the message from client and copy it in buffer
        read(sockfd, buff, sizeof(buff));
        // print buffer which contains the client contents
        printf("From client: %s <--------> To client : \n", buff);

        char *args[6];
        splitArgs(buff, args);

        //Si el mensaje contiene la palabra "eliminar" al principio, ejecutar lógica de eliminar archivo
        if (strcmp("delete", args[0]) == 0)
        {
            char *fileName = args[1];
            char *directory = (char *)malloc((strlen(folderName) + strlen(args[2])) * sizeof(char));
            sprintf(directory, "%s%s", folderName, args[2]);
            printf("Se eliminará archivo: \"%s%s\"\n", directory, fileName);

            deleteFile(fileName, directory);
            free(directory);
        }
        else if (strcmp("deleteDir", args[0]) == 0)
        {
            char *directory = (char *)malloc((strlen(args[1])+ strlen(folderName)+2) * sizeof(char));
            sprintf(directory, "%s%s", folderName, args[1]);
            printf("Se eliminará directorio: \"%s\"\n", directory);

            deleteDirectory(directory);
            free(directory);
        }
        else if (strcmp("createFile", args[0]) == 0)
        {
            char *fileName = args[1];
            char *directory = (char *)malloc((strlen(folderName) + strlen(args[2])) * sizeof(char));
            sprintf(directory, "%s%s", folderName, args[2]);
            printf("Se creara archivo: \"%s%s\"\n", directory, fileName);

            CreateFile(fileName, directory);
            free(directory);
        }
        else if (strcmp("createDir", args[0]) == 0)
        {
            char *DirName = args[2];
            char *directory = (char *)malloc((strlen(folderName) + strlen(args[1])) * sizeof(char));
            sprintf(directory, "%s%s", folderName, args[1]);
            printf("Se creara el directorio : \"%s%s\"\n", directory, DirName);

            createDirectory(directory , DirName);
            free(directory);
        }
        else if(strcmp("modifyFile", args[0]) == 0){
            
            char *fileName = args[2];
            char *directory = (char *)malloc((strlen(folderName) + strlen(args[3]) + 1) * sizeof(char));

            sprintf(directory, "%s%s", folderName, args[3]);
            printf("Se modificara el archivo: \"%s%s\"\n", directory, fileName);


            int contentSize = atoi(args[1]);
            char *fileContent = (char *)malloc((contentSize + 1) * sizeof(char)); //set content to the correct full size
            
            printf("SIZE SET\n");

            //send aknowlegment 
            strncpy(buff, "Preparado para recibir contenido", strlen("Preparado para recibir contenido"));
            printf("SERVER: Sending Aknowledgment: %s\n", buff);

            int numbersWritten = write(sockfd, buff, sizeof(buff));

            sleep(1);
            bzero(buff, sizeof(buff));

            //read content from socket 
            //read(sockfd, buff, sizeof(buff));
            int res = 0;
            do{
                sleep(0.01);
                bzero(buff, sizeof(buff));
                res = read(sockfd, buff, sizeof(buff));
            }while (res = 0 || buff == 0 || buff[0] == '\0');

            printf("SERVER: Response from client with content: %s", buff);
            //copy buffer to content
            strncpy(fileContent, buff, strlen(buff) + 1);

            bzero(buff, sizeof(buff)); 

            //Actually modify the file
            CreateOrModifyFile(fileName, fileContent, directory);
            free(directory);
        }
        

        bzero(buff, MAX);
        n = 0;
        // copy server message in the buffer
        /*while ((buff[n++] = getchar()) != '\n') 
            ; */

        //Si todo salió bien, mandar un mensaje de "operacionExitosa"
        strncpy(buff, "operacionExitosa", strlen("operacionExitosa"));

        // and send that buffer to client
        write(sockfd, buff, sizeof(buff));

        // if msg contains "Exit" then server exit and chat ended.
        /*if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        }*/

        //salir del loop
        break;
    }
}

// Driver function
int main()
{
    char pwd[1024];
    getcwd(pwd, sizeof(pwd));
    printf("INITIALIZING SERVER FROM DIR: %s\n", pwd);

    while (1)
    {

        int sockfd, connfd, len;
        struct sockaddr_in servaddr, cli;

        // socket create and verification
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
        {
            printf("socket creation failed...\n");
            exit(0);
        }
        else
            printf("Socket successfully created..\n");
        bzero(&servaddr, sizeof(servaddr));

        //Usamos las siguientes líneas de código para reusar el address del socket previamente creado en caso de que esté en cleanup state, para seguir escuchando por mensajes.
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
            printf("setsockopt(SO_REUSEADDR) failed\n");

        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(PORT);

        // Binding newly created socket to given IP and verification
        if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
        {
            printf("socket bind failed...\n");
            exit(0);
        }
        else
            printf("Socket successfully binded..\n");

        // Now server is ready to listen and verification
        if ((listen(sockfd, 5)) != 0)
        {
            printf("Listen failed...\n");
            exit(0);
        }
        else
            printf("Server listening..\n");
        len = sizeof(cli);

        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA *)&cli, &len);
        if (connfd < 0)
        {
            printf("server acccept failed...\n");
            exit(0);
        }
        else
            printf("server acccept the client...\n");

        // Function for chatting between client and server
        func(connfd);

        // After chatting close the socket
        close(sockfd);
        printf("Entro close(socket)\n");

        //sleep(5);
        printf("Volviendo a escuchar..\n\n");
    }
}
