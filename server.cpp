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

#include <ola/DmxBuffer.h>
#include <ola/io/SelectServer.h>
#include <ola/Logging.h>
#include <ola/client/ClientWrapper.h>
#include <ola/Callback.h>
#include <ola/thread/Thread.h>

#include "dmx_utils.h"
#include "dmxsender.h"		// instanciated object approach
//#include "delta.h"		// singleton approach

using namespace std;
using std::cout;
using std::endl;

// This function is run from the OLA Thread, if you use variables in the main
// program then you'll need to add locking.

DmxSender ola_thread;

bool SendData(ola::client::OlaClientWrapper *wrapper, DmxSender *d) 
{
	d->SendDmxFrame(/*wrapper*/);	
	return true;
	static unsigned int universe = 1;
	static unsigned int i = 0;
	ola::DmxBuffer buffer;
 
 	buffer.Blackout();
 	buffer.SetChannel(0, i);
 	wrapper->GetClient()->SendDMX(universe, buffer, ola::client::SendDMXArgs());
 	if (++i == 100) 
 	{
 		//wrapper->GetSelectServer()->Terminate();
 		i=0;
 	}
 	return true;
}

void parseMessage(string msg) 
{
	// singleton call
	// DmxSender &d = DmxSender::getInstance();

	if(msg.size() == 0)
	{
		return;
	}
	string command, data;
	std::vector<std::string> result = explode(msg, '.');
	if (result.size() > 1)
	{			
		command = result[0];	
		data = result[1];	
	}
	else
	{
		command = msg;
	}	
	cout << "COMMAND:" << command << " DATA: " << data << endl;
	if (command == "start" || command == "stop" || command == "status" || command == "reset" || command == "log")
	{
		if(data == "")
		{
			log("Specify scenarii id");		
			return;
		}
	}
	int idata = -1;
	
	if (hasOnlyDigits(data))
	{	
		idata = stoi(data);
	}	
	/*
	if(command == "ulog")
	{		
		d.ChangeUnivLogLevel();
	}	

	if(command == "log")
	{		
		d.ChangeLogLevel(idata);
	}

	if(command == "halt")
	{		
		d.HaltDmxSender();
	}

	if(command == "resume")
	{		
		d.ResumeDmxSender();
	}

	if(command == "close")
	{		
		d.CloseDmxSender();
	}
	*/
	if(command == "start")
	{	
		cout << "Calling d.StartScenari(idata);" << endl;	
		//d->StartScenari(idata);
		ola_thread.StartScenari(idata);
		cout << "Called d.StartScenari(idata);" << endl;
	}
	/*
	if(command == "stop")
	{		
		d.StopScenari(idata);
	}    
	*/
	if(command == "status")
	{		
		//d.StatusScenari(idata);
	}    
	/*
	if(command == "reset")
	{		
		d.ResetScenari(idata);
	}    

	if(command == "list")
	{		
	//		d.ResetScenari(idata);
	}    

	if(command == "stopall")
	{		
		d.StopAll();
	}    

	if(command == "resetall")
	{		
		d.ResetAll();
	}    

	if(command == "bo")
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
        log("Message received");
		log(client_message);
		log("Parsing message");
		parseMessage(client_message);
		write(sock, "Message received", 16);
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
 	//OlaThread ola_thread;
 	//DmxSender ola_thread;
 	if (!ola_thread.Start()) 
 	{
 		std::cerr << "Failed to start OLA thread" << endl;
 		exit(1);
 	}
 	// Control is returned to the main program here.
 	ola::io::SelectServer *ss = ola_thread.GetSelectServer();
 	//ss->RegisterRepeatingTimeout(ola_thread._tick_interval, ola::NewCallback(&SendData, &ola_thread.m_wrapper));	// WORKING
 	ss->RegisterRepeatingTimeout(ola_thread._tick_interval, ola::NewCallback(&SendData, &ola_thread.m_wrapper, &ola_thread));	// WORKING
 	//ss->RegisterRepeatingTimeout(10, ola::NewCallback(&DmxSender::SendDmxFrame, &ola_thread.m_wrapper));
 	
 	// SendDmxFrame
 // The main program continues...
 	
	/*
	// USED TO CHECK THE SINGLETON DESIGN PATTERN. ONLY ONE INSTANCE OF THE DELTA CLASS	
	Delta &d1 = Delta::getInstance();
    Delta &d2 = Delta::getInstance();
    std::cout << &d1 << '\n';
    std::cout << &d2 << '\n';
    return 0;
	*/

	//DmxSender dmxSender;
	
	
	
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
	/*if(!wrapper.Setup())	
	{
		error("Wrapper Setup failed");
	}
	cout << d._tick_interval << endl;*/
	// Create a timeout and register it with the SelectServer
	//ola::io::SelectServer *ss = wrapper.GetSelectServer();
	//ss->RegisterRepeatingTimeout(d._tick_interval, ola::NewCallback(&SendData, &wrapper));
	//ss->RegisterRepeatingTimeout(d._tick_interval, ola::NewCallback(&d.SendDmxFrame, &wrapper));
	// Start the main loop
	//cout <<	"// Start the main loop" << endl;
	//ss->Run();
	
	log("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	
	while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
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


