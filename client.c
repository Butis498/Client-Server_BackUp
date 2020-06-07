/* A simple client that connects to a server in the internet domain using TCP
  The host name and port number are passed as an argument
 
  example: ./client 127.0.0.1 3000   
  or:      ./client localhost 3000  
  */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
 
#define h_addr h_addr_list[0] /* for backward compatibility */
 
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
   exit(0);
}
 
int main(int argc, char *argv[])
{
   /*
   sockfd is a subscript into the file descriptor table
   and is used to store the returned file descriptor from the socket system call
   */
   int sockfd, portno; //portno stores the port number of the server that is provided by the user
 
   int n; //used to temporarily store the return of read() and write() calls, that is the number of chars read or written
 
   struct sockaddr_in serv_addr; //of type socket address is the server address
  
   /*
   of type hostent is the server, wich  contains:
   h_name => a name
   h_aliases => an alias list
   h_addrtype => a host address type
   h_length => a length for such address
   h_addr_list => a list of addresses
 
   And as such defines a host computer on the internet
   */
   struct hostent *server;
 
   char buffer[256]; //buffer used to read characters from the socket
 
   if (argc < 3) { //we must receive at least two arguments
      fprintf(stderr,"usage %s hostname port\n", argv[0]); //prints to std error the error message
      exit(0);
   }
 
   portno = atoi(argv[2]);//second argument is the port number and we save it
 
   /*
   The socket system call creates a new socket and returns an entry into the file descriptor table (an integer)
 
       socket(int domain, int type, int protocol);
 
   SOCK_STREAM => Stream Socket (characters are read in a continuous stream as if from a file or pipe)
   AF_INET => Address Format, Internet = IP Addresses
   */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
 
   if (sockfd < 0) { //if the socket() system call returned error
       error("ERROR opening socket");
   }
 
   /*
    We use the function gethostbyname() defined as:
       struct hostent *gethostbyname(char *name)
    It takes the host name and returns a pointer to a hostent containing information about that host.
   */
   server = gethostbyname(argv[1]); //first argv argument is the host name as on the internet
 
   if (server == NULL) {//if gethostbyname could not locate the host with the received name
       fprintf(stderr,"ERROR, no such host\n");
       exit(0);
   }
 
   bzero((char *) &serv_addr, sizeof(serv_addr));//we clear the server address first in order to use it
 
   //we set the server address parameters
   serv_addr.sin_family = AF_INET; //we set the family to the Address Format IP Address
   bcopy((char *)server->h_addr,
       (char *)&serv_addr.sin_addr.s_addr,
       server->h_length); //we copy the server address from the found host to our server address struct
   serv_addr.sin_port = htons(portno);//we set the port number to the user's port number (after converting it to network byte order)
 
   /*
   We call connect() as to establish a connection to the server (as client)
       int connect(s, name, namelen)
   where s is the socket file descriptor, and name the server address struct as a sockaddr
   */
   if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) { //if connect returns -1 which means that it failed
       error("ERROR connecting");
   }
 
   //receives a user message and writes it to the socket that we established a connection with
   printf("Please enter the message: ");
   bzero(buffer,256); //clears the buffer
   fgets(buffer,255,stdin); //gets the user input
   n = write(sockfd,buffer,strlen(buffer)); //we write to the client socket
 
   if (n < 0) {//if write returns an error state
       error("ERROR writing to socket");
   }
 
   //reads from the socket
   bzero(buffer,256); //clears the buffer
   n = read(sockfd,buffer,255); //we read the socket using it's file descriptor for the client. It will block the process until there is something to read
 
   if (n < 0) {//if read returns an error state
       error("ERROR reading from socket");
   }
 
   //prints the server message received from the connection socket
   printf("%s\n",buffer);
   close(sockfd); //closes socket connection
  
   return 0;
}
