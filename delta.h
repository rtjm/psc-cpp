#ifndef _delta_h 
#define _delta_h
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

#include <iostream>
#include <chrono>

#include <functional>
#include <algorithm>

#include "playscenari.h"

using namespace std;

class DmxSender {
	private:		
		static bool _activesender;
		static int tick_interval, univ_qty;	
		static vector<int> vectorWholeDmxFrame;
		static vector<int> scen_ids;
		static map<int, PlayScenari> my_scens;

		ola::DmxBuffer WholeDmxFrame;
		ola::client::OlaClientWrapper wrapper;
		
		void timer_start(std::function<void(void)> func, unsigned int interval)
		{
			std::thread([func, interval]() {
				while (true)
				{
				    func();
				    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
				}
			}).detach();
		}

		int sqlFramerate()
		{
			int result = 25; // Default to 40 fps	
			MYSQL_RES *resultset = openRecordset("SELECT freq_ms FROM dmx_engine WHERE id=1;");
			MYSQL_ROW row;
			string sfreq_ms;
			while ((row = mysql_fetch_row(resultset))) 
			{ 
				sfreq_ms = row[0];
			}			
			if(hasOnlyDigits(sfreq_ms)) 
			{
				result = stoi(sfreq_ms);
			}
			return result;
		}

		int sqlUniverses()
		{			
			int result = 1; // Default to 1 universe
			MYSQL_RES *resultset = openRecordset("SELECT univ_qty FROM dmx_preferences WHERE id=1;");
			MYSQL_ROW row;
			string sfreq_ms;
			while ((row = mysql_fetch_row(resultset))) 
			{ 
				sfreq_ms = row[0];
			}			
			if(hasOnlyDigits(sfreq_ms)) 
			{
				result = stoi(sfreq_ms);
			}
			return result;
		}

    protected:
        static DmxSender *instance;
        DmxSender() 
		{ 
			tick_interval = sqlFramerate();		
			univ_qty = sqlUniverses();		
			_activesender = true;
			vectorWholeDmxFrame = InitVector(univ_qty);		// size vectorWholeDmxFrame to 512*univ_qty and fill 0
			WholeDmxFrame.Blackout();
			if(!wrapper.Setup())	
			{
				error("Wrapper Setup failed");
			}
			// Create a timeout and register it with the SelectServer
			//ola::io::SelectServer *ss = wrapper.GetSelectServer();
			//ss->RegisterRepeatingTimeout(250, ola::NewCallback(&SendDmxFrame, &wrapper));
			
			
			//SendDmxFrame(/*&wrapper*/);			
			timer_start(SendDmxFrame, 10000); // set to 10 secs for debug - put tick_interval in release
			
			// Start the main loop
			//ss->Run();

			cout << "CONSTRUCTOR" << endl;
		}
    public:			
    	
        static  DmxSender& getInstance() 
		{
			
            if (instance == 0)
                instance = new DmxSender();
            return *instance;
        }

		static  void SendDmxFrame(/*ola::client::OlaClientWrapper *wrapper*/)
		{
			log("Entering SendDmxFrame...");
			// Ask frame for each scenari and make the whole frame, repeated every tick_intervals
			if (_activesender)
			{				
				//****IMPLEMENTS self._wrapper.AddEvent..
			}	
			
			
			// Send data to universes
			for(int u = 0; u < univ_qty; u++)
			{
				vector<int> partialFrameVector;
				for(int i= u * 512; i < u*512 +512; i++)
				{
					partialFrameVector.push_back(vectorWholeDmxFrame[i]);		
				}
				// vectorWholeDmxFrame splitted in partialFrameVector
				
				//ssmsg.str("");
				//ssmsg << "FRAME_FOR_UNIV " << u;
				//dmxlog(ssmsg.str(), 0);
				string partialFrame = buildDmxFrame(partialFrameVector);
				ola::DmxBuffer buffer;
 				buffer.Blackout();
				buffer.SetFromString(partialFrame);
				try 
				{
 					//wrapper->GetClient()->SendDMX(u, buffer, ola::client::SendDMXArgs());
 					log("****buffer sent to OLA****");
				}
				catch (int e)
				{
					log("Dmx frame not sent. Reset all");
					ResetAll();
				}
				/*
				ssmsg.str("");
				ssmsg << "Before computing " << currentDateTime();
				dmxlog(ssmsg.str(), 1);*/

			}
			
			// For each scenarii in list
			for(int s = 0; s < scen_ids.size(); s++)			
			{
				try
				{
					// create scenari instance if needed
					int scenarid = scen_ids[s];
					auto search = my_scens.find(scenarid);
					if(search == my_scens.end())
					{
						log("Playscenarii creation");
						PlayScenari p(scenarid,tick_interval);						
						my_scens.insert(std::make_pair(scenarid, p));
						// Dump my_scens
						for(auto it = my_scens.cbegin(); it != my_scens.cend(); ++it)
						{
							std::cout << it->first << " " << &it->second << endl;
						}
						
						
					}
				}
				catch (int e)
				{
				}	
			}
						
		}
		
		void StartScenarii(int scenarid)
		{
			if(std::find(scen_ids.begin(),scen_ids.end(),scenarid)==scen_ids.end())			// Check not to add twice
			{				
				scen_ids.push_back(scenarid);		
				log("Added scen_id");
			}
			else
			{
				log("Scenarid already there");
			}
			cout << "StartScenarii CALLED IN DELTA:" << scenarid << " tick_interval "<< tick_interval  << " univ_qty "<< univ_qty << endl;
		}
		
		void ChangeUnivLogLevel()
		{
			cout << "ChangeUnivLogLevel CALLED IN DELTA:" << endl;
		}

		void ChangeLogLevel(int scenarid)
		{
			cout << "ChangeLogLevel CALLED IN DELTA:" << scenarid << endl;
		}

		void HaltDmxSender()
		{
			cout << "HaltDmxSender CALLED IN DELTA:" << endl;
		}

		void ResumeDmxSender()
		{
			cout << "ResumeDmxSender CALLED IN DELTA:" << endl;
		}

		void CloseDmxSender()
		{
			cout << "CloseDmxSender CALLED IN DELTA:" << endl;
		}

		void StopScenari(int scenarid)
		{
			cout << "StopScenari CALLED IN DELTA:" << scenarid << endl;
		}

		void StatusScenari(int scenarid)
		{
			auto search = my_scens.find(scenarid);
			if(search != my_scens.end())
			{
				cout << "STATUS FOR " << &search->second << endl;
				auto p = search->second;
				p.Status();				
			}
			cout << "StatusScenari CALLED IN DELTA:" << scenarid << endl;
		}

		void ResetScenari(int scenarid)
		{
			cout << "ResetScenari CALLED IN DELTA:" << scenarid << endl;
		}

		void StopAll()
		{
			cout << "StopAll CALLED IN DELTA:" << endl;
		}

		static void ResetAll()
		{
			cout << "ResetAll CALLED IN DELTA:" << endl;
		}

		void BlackOut()
		{
			cout << "BlackOut CALLED IN DELTA:" << endl;
		}
};

#endif
