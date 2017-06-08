#ifndef _dmx_sender_h 
#define _dmx_sender_h
#include <thread>
#include <string>
#include <sstream>
#include <map>
#include <mysql/mysql.h>
#include <ola/DmxBuffer.h>
#include <ola/io/SelectServer.h>
#include <ola/Logging.h>
#include <ola/client/ClientWrapper.h>
#include <ola/Callback.h>

#include "playscenari.h"

using namespace std;

class dmxsender 
{
	public:
	ola::DmxBuffer WholeDmxFrame;
	ola::client::OlaClientWrapper wrapper;
	bool SendData(ola::client::OlaClientWrapper *wrapper) 
	{
 		static unsigned int universe = 1;
 		static unsigned int i = 0;
 		WholeDmxFrame.Blackout();
 
 		WholeDmxFrame.SetChannel(i, i);
 		//buffer.SetFromString(frame);
 		wrapper->GetClient()->SendDMX(universe, WholeDmxFrame, ola::client::SendDMXArgs());
 		if (++i == 10) 
 		{
 			wrapper->GetSelectServer()->Terminate();
 		}
		return true;
	}

//	public:	
	dmxsender()
	{
		/*
		ola::InitLogging(ola::OLA_LOG_WARN, ola::OLA_LOG_STDERR);
		if (!wrapper.Setup()) 
		{
 			error("Setup failed");
 		}
 		// Create a timeout and register it with the SelectServer
 		ola::io::SelectServer *ss = wrapper.GetSelectServer();
 		ss->RegisterRepeatingTimeout(250, ola::NewCallback(&SendData, &wrapper));
 		// Start the main loop
 		ss->Run();*/
		cout << "Constructor dmxsender object" << endl;
	}
	
	void StartScenarii(int scenarid)
	{
		cout << "StartScenarii" << endl;
	}
};

#endif
