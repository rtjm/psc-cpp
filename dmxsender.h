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
#include <ola/thread/Thread.h>
#include <iostream>
#include <chrono>
#include <functional>
#include <algorithm>
#include "playscenari.h"

using namespace std;

class DmxSender: public ola::thread::Thread  {
	private:		
	protected:
	 	void *Run() 
	 	{
	 		cout << "THREAD" << endl;
	 		Terminated = false;
			_activesender = true;
			univloglevel = 0;					
			
			
			
			// array to store full frame
			vectorWholeDmxFrame = InitVector(univ_qty);		// size vectorWholeDmxFrame to 512*univ_qty and fill 0
			WholeDmxFrame.Blackout();
	 		SendDmxFrame();	
	 		m_wrapper.GetSelectServer()->Run();
	 		return NULL;
	 	}
    
    public:	
    	bool _activesender, Terminated;
		int _tick_interval, univ_qty, univloglevel;	
		vector<int> vectorWholeDmxFrame;
		vector<int> scen_ids;
		map<int, PlayScenari*> my_scens;
		ola::DmxBuffer WholeDmxFrame;
		ola::client::OlaClientWrapper m_wrapper;

	 	bool Start() 
	 	{
		 	if (!m_wrapper.Setup()) 
		 	{
			 	cout << "OLA FAIL" << endl;
		 		return false;
		 	}
		 	// SQL Framerate
			_tick_interval = sqlFramerate();	
		 	cout << "OLA SUCCEED FREQ MS:" << _tick_interval << endl;
		 	// SQL Universe
			univ_qty = sqlUniverses();		
			cout << "univ_qty:" << univ_qty << endl;	
	 		return ola::thread::Thread::Start();
	 	}
	 	
	 	void Stop() 
	 	{
	 		m_wrapper.GetSelectServer()->Terminate();
	 	}
	 
		ola::io::SelectServer* GetSelectServer() 
	 	{
	 		return m_wrapper.GetSelectServer();
	 	} 	
	 	
	 	
    	DmxSender() 
		{ 			
			Terminated = false;			
			cout << "CONSTRUCTOR" << endl;
		}	
			
		bool SendDmxFrame(/*ola::client::OlaClientWrapper *_wrapper*/)
		{
			// Ask frame for each scenari and make the whole frame, repeated every _tick_interval
			
			
			// Send data to universes
			for(int u = 0; u < univ_qty; u++)
			{				
				vector<int> partialFrameVector;
				for(int i= u * 512; i < u*512 +512; i++)
				{
					partialFrameVector.push_back(vectorWholeDmxFrame[i]);		
				}
				// vectorWholeDmxFrame splitted in partialFrameVector: equivalent of FramePart in SplittedFrame (python)
				string partialFrame = buildDmxFrame(partialFrameVector);
				ola::DmxBuffer buffer;
 				buffer.Blackout();
				buffer.SetFromString(partialFrame);
				try 
				{
					int _u = u + 1;
 					m_wrapper.GetClient()->SendDMX(_u, buffer, ola::client::SendDMXArgs());	// OLA
				}
				catch (int e)
				{
					log("Dmx frame not sent. Reset all");
					ResetAll();
				}				
			}			
			
			// For each scenari in list
			for(int s = 0; s < scen_ids.size(); s++)			
			{				
				try
				{
					// store instance in dict, only once
					
					int scenarid = scen_ids[s];
					auto search = my_scens.find(scenarid);
					if(search == my_scens.end())
					{
						//PlayScenari p(scenarid,_tick_interval);					
						//cout << "Playscenari object created at address	" << &p << endl;
						//my_scens.insert(std::make_pair(scenarid, p));
						my_scens.insert(std::make_pair(scenarid, new PlayScenari(scenarid, _tick_interval)));
						
						
						// Dump my_scens
						for(auto it = my_scens.cbegin(); it != my_scens.cend(); ++it)
						{
							std::cout << it->first << " " << &it->second << endl;
						}
						//currentplay = &p;
					}
					PlayScenari *currentplay = my_scens.at(scenarid);
					cout << "current PlayScenari " << currentplay->scenari << "/" << currentplay->new_frame.size() << endl;
					AssignChannels(currentplay->patch, currentplay->new_frame);
					
				}
				catch (int e)
				{
					cout << "NOT STARTED" << endl;
				}	
			}

			// for each instance, compute frame
			if (my_scens.size() > 0)
			{
				for(auto it = my_scens.cbegin(); it != my_scens.cend(); ++it)
				{
					PlayScenari *p = it->second;
					p->ComputeNextFrame();
					
				}
			}				
			return true;
		}
		
		void AssignChannels(int offset, vector<int> values)
		{
			//vectorWholeDmxFrame.at(offset) = values.at(offset);
			cout << "AssignChannels " << values.size() << " OFFSET " << offset << endl;
			for (int i = offset; i < offset + values.size(); i++)
			{
				vectorWholeDmxFrame.at(i) = values.at(i-offset);
			}
		}
		
		void StartScenari(int scenarid)
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
			cout << "StartScenari CALLED IN DELTA:" << scenarid << " tick_interval "<< _tick_interval  << " univ_qty "<< univ_qty << endl;
		}
		
		void ChangeUnivLogLevel()
		{
			univloglevel++;
			if (univloglevel > 2)
			{
				univloglevel = 0;
			}
			cout << "ChangeUnivLogLevel CALLED IN DELTA:" << endl;
		}

		void ChangeLogLevel(int scenarid)
		{
			auto search = my_scens.find(scenarid);
			if(search != my_scens.end())
			{
				// Set loglevel for this instance
				search->second->loglevel++;
				if (search->second->loglevel > 2)		
				{
					search->second->loglevel = 0;
				}
			}			
			cout << "ChangeLogLevel CALLED IN DELTA:" << scenarid << endl;
		}

		void HaltDmxSender()
		{
			_activesender = false;
			cout << "HaltDmxSender CALLED IN DELTA:" << endl;
		}

		void ResumeDmxSender()
		{
			_activesender = true;
			SendDmxFrame();
			cout << "ResumeDmxSender CALLED IN DELTA:" << endl;
		}

		void CloseDmxSender()
		{
			m_wrapper.GetSelectServer()->Terminate();
			cout << "CloseDmxSender CALLED IN DELTA:" << endl;
		}

		void StopScenari(int scenarid)
		{
			/*auto search = my_scens.find(scenarid);
			if(search != my_scens.end())
			{
				// remove id from list
				my_scens
			}		*/
			for (int i = 0; i < scen_ids.size(); i++)
			{
				if (scen_ids.at(i) == scenarid)
				{
					scen_ids.erase(scen_ids.begin()+i);
					break;
				}
			}
			
			cout << "StopScenari CALLED IN DELTA:" << scenarid << endl;
		}

		void StatusScenari(int scenarid)
		{
			auto search = my_scens.find(scenarid);
			if(search != my_scens.end())
			{
				cout << "STATUS FOR " << &search->second << endl;
				auto p = search->second;
				//p.Status();				
			}
			cout << "StatusScenari CALLED IN DELTA:" << scenarid << endl;
		}

		void ResetScenari(int scenarid)
		{
			auto search = my_scens.find(scenarid);
			if(search != my_scens.end())
			{
				my_scens.erase(search);
			}
			cout << "ResetScenari CALLED IN DELTA:" << scenarid << endl;
		}

		void StopAll()
		{
			scen_ids.clear();
			cout << "StopAll CALLED IN DELTA:" << endl;
		}

		void ResetAll()
		{
			my_scens.clear();
			cout << "ResetAll CALLED IN DELTA:" << endl;
		}

		void BlackOut()
		{
			vectorWholeDmxFrame = InitVector(univ_qty);		// size vectorWholeDmxFrame to 512*univ_qty and fill 0
			cout << "BlackOut CALLED IN DELTA:" << endl;
		}
		
		
};

#endif
