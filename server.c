/* A simple server in the internet domain using TCP
  The port number is passed as an argument
 
  example:  ./server 3000
  */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
 
/*
This function is used to handle errors, basically just a wrapper of perror()
param const char *msg: the message to output in the error
*/
void error(const char *msg)
{
   /*as per documentation perror(msg) :
   "produces a short  error  message  on  the  standard
    error describing the last error encountered during a call to
    a system or library function"
   */
   perror(msg);
   exit(1);
}
 
int main(int argc, char *argv[])
{
   /*
   sockfd and newsockfd are subscripts into the file descriptor table
   and are used to store the returned file descriptor from the socket system call
   */
   int sockfd, newsockfd, portno; //portno stores the port number of the server that is provided by the user
 
   socklen_t clilen; //the clilen struct stores the size of the address of the client.
 
   char buffer[256]; //buffer used to read characters from the socket
 
   struct sockaddr_in serv_addr, cli_addr; //a server address and a client address respectively
 
   int n; //used to temporarily store the return of read() and write() calls, that is the number of chars read or written
 
   if (argc < 2) { //we must receive at least one argument
       fprintf(stderr,"ERROR, no port provided\n");
       exit(1);
   }
 
   /*
   The socket system call creates a new socket and returns an entry into the file descriptor table (an integer)
 
       socket(int domain, int type, int protocol);
 
   SOCK_STREAM => Stream Socket (characters are read in a continuous stream as if from a file or pipe)
   AF_INET => Address Format, Internet = IP Addresses
   */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
 
   if (sockfd < 0) { //if the socket system call returned -1, it failed
       error("ERROR opening socket");
   }
  
   bzero((char *) &serv_addr, sizeof(serv_addr)); //clears the data from the server address to prepare it to receive new data
 
   portno = atoi(argv[1]); //we save the user port number in portno
 
   //We set the properties of our server address
   serv_addr.sin_family = AF_INET; //we set the family to the Address Format IP Address
   serv_addr.sin_addr.s_addr = INADDR_ANY; //we set the IP address of the host, in this case that is our own ip
   serv_addr.sin_port = htons(portno); //we set the port number to the user's port number (after converting it to network byte order)
 
   /*
   We bind the socket to an address using:
       int bind(s, name, namelen)
 
   which will be saved in serv_addr
   */
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { //if it fails bind() returns -1
       error("ERROR on binding");
   }  
  
   listen(sockfd,5); //this system call sets the system to listen to the socket with a size of 5 for the backlog queue
   clilen = sizeof(cli_addr);
 
   /*
   with the system call accept() the process blocks until some client connects to the server
   and if so returns a new file descriptor and puts the client address in cli_addr
   */
   newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
 
   if (newsockfd < 0) { //if accept returns an error state
       error("ERROR on accept");
   }
  
   bzero(buffer,256);//we clear the buffer again
   n = read(newsockfd,buffer,255); //we read the socket using it's file descriptor for the client. It will block the process until there is something to read
 
   if (n < 0) {//if read returns an error state
       error("ERROR reading from socket");
   }
 
   printf("Here is the message: %s\n",buffer);//we print the received message
   n = write(newsockfd,"I got your message",18); //we write to the client socket
  
   if (n < 0) {//if write returns an error state
       error("ERROR writing to socket");
   }
 
   //we close the connection to the client socket and the originally opened socket so the system can allocate them to other process
   close(newsockfd);
   close(sockfd);
 
   return 0;
}