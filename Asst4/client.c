#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "client.h"
#include <signal.h>




void error(char *msg)
{
    perror(msg);
    exit(0);
}

void inthandler(int sig){
  write(sock,"Client interrupted, goodbye",28);
  exit(0);
}

void *writethread(void *arg)
{
	int n=-1;
	int socket = (intptr_t)arg;
	int loop = 1;
	char buffer[256];
	while(loop)
	{
		usleep(20000);
	    //printf("Please enter the message: \n");

		// zero out the message buffer
	    bzero(buffer,256);
		// get a message from the client
	    fgets(buffer,255,stdin);

		sleep(2);
		// try to write it out to the server
		n = write(socket,buffer,strlen(buffer));

		// if we couldn't write to the server for some reason, complain and exit
	    if (n < 0)
		{
	         error("ERROR writing to socket");
	    }
	}
	pthread_exit(0);
}

int main(int argc, char *argv[])
{
	// Declare initial vars
    int sockfd = -1;																// file descriptor for our socket
	int portno = -1;																// server port to connect to
	int n = -1;																		// utility variable - for monitoring reading/writing from/to the socket
	char buffer[256];															// char array to store data going to and coming from the server
    struct sockaddr_in serverAddressInfo;						// Super-special secret C struct that holds address info for building our socket
    struct hostent *serverIPAddress;									// Super-special secret C struct that holds info about a machine's address
    pthread_t pth;


	// If the user didn't enter enough arguments, complain and exit
    if (argc < 2)
	{
       printf("ERROR: must specify server's location");
       exit(0);
    }



	/** If the user gave enough arguments, try to use them to get a port number and address **/

	// convert the text representation of the port number given by the user to an int
	portno =12321; //atoi(argv[2]);

	// look up the IP address that matches up with the name given - the name given might
	//    BE an IP address, which is fine, and store it in the 'serverIPAddress' struct
    serverIPAddress = gethostbyname(argv[1]);
    if (serverIPAddress == NULL)
	{
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

	// try to build a socket .. if it doesn't work, complain and exit
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
	{
        error("ERROR creating socket");
	}
    sock=sockfd;


	/** We now have the IP address and port to connect to on the server, we have to get    **/
	/**   that information into C's special address struct for connecting sockets                     **/

	// zero out the socket address info struct .. always initialize!
    bzero((char *) &serverAddressInfo, sizeof(serverAddressInfo));

	// set a flag to indicate the type of network address we'll be using
    serverAddressInfo.sin_family = AF_INET;

	// set the remote port .. translate from a 'normal' int to a super-special 'network-port-int'
	serverAddressInfo.sin_port = htons(portno);

	// do a raw copy of the bytes that represent the server's IP address in
	//   the 'serverIPAddress' struct into our serverIPAddressInfo struct
    bcopy((char *)serverIPAddress->h_addr, (char *)&serverAddressInfo.sin_addr.s_addr, serverIPAddress->h_length);



	/** We now have a blank socket and a fully parameterized address info struct .. time to connect **/

	// try to connect to the server using our blank socket and the address info struct
	//   if it doesn't work, complain and exit
    while(connect(sockfd,(struct sockaddr *)&serverAddressInfo,sizeof(serverAddressInfo)) < 0)
	{
        printf("Attempting to connect to server...\n" );
        sleep(3);
	}
	signal(SIGINT,inthandler);

	pthread_create(&pth,NULL,writethread,(void*)(intptr_t)sockfd);

	int loop=1;

	while(loop){
	/** If we're here, we're connected to the server .. w00t!  **/

	    bzero(buffer,256);

		// read a message from the server into the buffer
    	n = read(sockfd,buffer,255);

		// if we couldn't read from the server for some reason, complain and exit
    	if (n < 0)
		{
        	 error("ERROR reading from socket");
		}

		// print out server's message
    	printf("%s\n",buffer);

	    if(strcmp(buffer,"Exiting")==0)
	    {
		    return 0;
	    }
	}
    return 0;
}
