#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "dmx_utils.h"
using namespace std;
using std::cout;
using std::endl;



void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
	string host, port;
	port = GetKey("PORT");
	host = GetKey("HOST");
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;


    /*if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);*/
	portno = std::stoi(port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    //server = gethostbyname(argv[1]);
	server = gethostbyname(host.c_str());
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{        
		error("ERROR connecting");
	}

    char buffer[256];
	printf("Please enter the message: ");
	bzero(buffer,256);
	fgets(buffer,255,stdin);
	stripNewlines((char *)&buffer);
	
	log("Ready to Send");
	log(buffer);
	log("Sending...");
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0) 
	{
	     error("ERROR writing to socket");
	}
	bzero(buffer,256);
	n = read(sockfd,buffer,255);
	if (n < 0) 
	{
		error("ERROR reading from socket");
	}
	printf("Server replied: %s\n",buffer);		
	close(sockfd);
   return 0;
}
