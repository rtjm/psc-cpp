/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

/* Compilation directives
g++ -pthread -o server $(mysql_config --cflags)  server.cpp $(mysql_config --libs) $(pkg-config --cflags --libs libola)  -std=c++11
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sstream>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include "dmx_utils.h"
//#include "dmx_sender.h"
#include "delta.h"

using namespace std;
using std::cout;
using std::endl;

Old version - RTJM
Delta *Delta::instance = 0;
dmxsender d;

/*
void error(const char *msg)
{
	perror(msg);
	exit(1);
}
*/
void parseMessage(string msg) 
{
	singleton call
	Delta &d = Delta::getInstance();

	if(msg.size()==0)
	{
		return;
	}
	string command, data;
	std::vector<std::string> result = explode(msg, '.');
	if (result.size()>1)
	{
			
		command = result[0];	
		data = result[1];	
	}
	else
	{
		command=msg;
	}	
	cout << "COMMAND:" << command << " DATA: " << data << endl;
	if (command=="start" || command=="stop" || command=="status" || command=="reset" || command=="log")
	{
		if(data=="")
		{
			log("Specify scenarii iD");		
			return;
		}
	}
	int idata=-1;
	
	if (hasOnlyDigits(data))
	{	
		idata = stoi(data);
	}	
	/*
	if(command=="ulog")
	{		
		d.ChangeUnivLogLevel();
	}	

	if(command=="log")
	{		
		d.ChangeLogLevel(idata);
	}

	if(command=="halt")
	{		
		d.HaltDmxSender();
	}

	if(command=="resume")
	{		
		d.ResumeDmxSender();
	}

	if(command=="close")
	{		
		d.CloseDmxSender();
	}
	*/
	if(command=="start")
	{		
		d.StartScenarii(idata);
	}
	/*
	if(command=="stop")
	{		
		d.StopScenari(idata);
	}    

	if(command=="status")
	{		
		d.StatusScenari(idata);
	}    

	if(command=="reset")
	{		
		d.ResetScenari(idata);
	}    

	if(command=="list")
	{		
	//		d.ResetScenari(idata);
	}    

	if(command=="stopall")
	{		
		d.StopAll();
	}    

	if(command=="resetall")
	{		
		d.ResetAll();
	}    

	if(command=="bo")
	{		
		d.StopAll();
		d.BlackOut();
	}  */  
}


void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
    string smatrix ;
    int ind ;
    string tok ;
    int i = 0 ;


    //Receive a message from client
    while((read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
		log("___________________");
		log(currentDateTime());
        log("Message received");
		log(client_message);
		log("Parsing message");
		parseMessage(client_message);
		write(sock,"Message received",16);
    }

    if(read_size == 0)
    {
        log("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        //ERROR
    }

    //Free the socket pointer
    free(socket_desc);

}


int main(int argc, char *argv[])
{	
	/*
	// USED TO CHECK THE SINGLETON DESIGN PATTERN. ONLY ONE INSTANCE OF THE DELTA CLASS	
	Delta &d1 = Delta::getInstance();
    Delta &d2 = Delta::getInstance();
    std::cout << &d1 << '\n';
    std::cout << &d2 << '\n';
    return 0;
	*/

	

	int socket_desc , client_sock , c , *new_sock, portno;
	string port;
	port = GetKey("PORT");
    struct sockaddr_in server, client;
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc < 0) 
	{
		close(socket_desc);
	    error("ERROR opening socket");
	}
	
	//Prepare the sockaddr_in structure
	portno = std::stoi(port);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(portno);

	//Binding.
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server))< 0)
    {
        error("ERROR binding socket");
    }

	//Listen
    listen(socket_desc , 5);

	log("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	
	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        log("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = (int*) malloc(1);
        *new_sock = client_sock;

        if(pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            error("ERROR pthread_create");
        }

        log("Handler assigned");
    }


	if (client_sock < 0)
    {
        error("ERROR client_sock");
    }

}


/*
 * This will handle connection for each client you may have and read indefinitely
 * */
/*
int main(int argc, char *argv[])
{
	string port;
	port = GetKey("PORT");
	int sockfd, newsockfd, portno, c;
 	socklen_t clilen;
	char buffer[256];
 	struct sockaddr_in serv_addr, cli_addr, client_sock;
 	int n;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
 	if (sockfd < 0) 
	{
	    error("ERROR opening socket");
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));	
	portno = std::stoi(port);
	serv_addr.sin_family = AF_INET;
 	serv_addr.sin_addr.s_addr = INADDR_ANY;
 	serv_addr.sin_port = htons(portno);
 	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{
		log("Port Error:");
		log(port);
		error("ERROR on binding");
	}
 	listen(sockfd,5);
	log("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	while((client_sock = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = (int*) malloc(1);
        *new_sock = client_sock;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            //ERROR
        }

        printf("Handler assigned");
    }
	

 	clilen = sizeof(cli_addr);
 	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) 
	{
	      
		error("ERROR on accept");
	}
	bzero(buffer,256);
	n = read(newsockfd,buffer,255);
	if (n < 0) 
	{
		error("ERROR reading from socket");
	}
	printf("Here is the message: %s\n",buffer);
	n = write(newsockfd,"I got your message",18);
	if (n < 0) error("ERROR writing to socket");
	close(newsockfd);
	close(sockfd);
	return 0; 
}
*/

