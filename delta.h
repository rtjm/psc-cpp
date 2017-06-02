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

#include "playscenari.h"

using namespace std;

class Delta {
	private:		
		bool _activesender = true;
		int univloglevel = 0;
		int tick_interval, univ_qty;				
		stringstream ssmsg;
		vector<int> vectorWholeDmxFrame;
		// List of scens to play
		vector<int> scen_ids;	
		//PlayScenari *my_scens[]; 
		map<int, PlayScenari*> my_scens;
		ola::DmxBuffer WholeDmxFrame;
		ola::client::OlaClientWrapper wrapper;

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

		void dmxlog(string msg, int level)
		{
			if(univloglevel > level)
			{
				cout << msg << endl;
			}
		}

    protected:
        static Delta *instance;
        Delta() 
		{ 
			tick_interval = sqlFramerate();		
			univ_qty = sqlUniverses();		
			vectorWholeDmxFrame = InitVector(univ_qty);
			WholeDmxFrame.Blackout();
			if(!wrapper.Setup())	
			{
				error("Wrapper Setup failed");
			}
			// Create a timeout and register it with the SelectServer
			ola::io::SelectServer *ss = wrapper.GetSelectServer();
			//ss->RegisterRepeatingTimeout(250, ola::NewCallback(&SendDmxFrame, &wrapper));
			//SendDmxFrame(&wrapper);			
			// Start the main loop
			//ss->Run();

			cout << "CONSTRUCTOR" << endl;
		}
    public:

		// From superfade for TestDmx
		int TICK_INTERVAL;// = 25;  // in ms
		unsigned int FADE_INTERVAL;// = 5000;// in ms
		string START;// = "15.15.215.100.255.255.255.100.0.100.0.0.0.0.0.0.0.255.100.0.100.0.0.255.215.100.255.255.255.100.0.100";
		string END;// =   "250.250.25.100.255.255.255.100.0.100.0.0.0.0.0.0.0.255.100.0.100.0.55.255.215.100.255.255.255.100.0.100";
		vector<int> FRAME;
		string m;
		// End of TestDmx variables 
		
		
        static Delta& getInstance() 
		{
			
            if (instance == 0)
                instance = new Delta();
            return *instance;
        }

		void SendDmxFrame(ola::client::OlaClientWrapper *wrapper)
		{

			// Ask frame for each scenari and make the whole frame, repeated every tick_intervals
			if (_activesender)
			{
				dmxlog("Schedule Next", 0);
				//****IMPLEMENTS self._wrapper.AddEvent..
			}	
			ssmsg << "Before sending:" << currentDateTime();
			dmxlog(ssmsg.str(), 1);
			
			// Send data to universes
			for(int u = 0; u < univ_qty; u++)
			{
				vector<int> partialFrameVector;
				for(int i= u * 512; i < u*512 +512; i++)
				{
					partialFrameVector.push_back(vectorWholeDmxFrame[i]);		
				}
				// vectorWholeDmxFrame splitted in partialFrameVector
				
				ssmsg.str("");
				ssmsg << "FRAME_FOR_UNIV " << u;
				dmxlog(ssmsg.str(), 0);
				string partialFrame = buildDmxFrame(partialFrameVector);
				ola::DmxBuffer buffer;
 				buffer.Blackout();
				buffer.SetFromString(partialFrame);
				try 
				{
 					wrapper->GetClient()->SendDMX(u, buffer, ola::client::SendDMXArgs());
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
			/*
			// For each scenarii in list
			for(int s = 0; s < scen_ids.size(); s++)			
			{
				try
				{
					// create scenari instance if needed

					int scenarid = scen_ids[s];
					PlayScenari p = PlayScenari(scenarid,TICK_INTERVAL);
					//my_scens.push_back(&p);
				}
				catch (int e)
				{
				}	
			}
			
			
			for(int p = 0; p < my_scens.size(); p++)
			{
				cout << my_scens[p] << endl;
			}
			*/
			cout << "SendDmxFrame CALLED IN DELTA:" << endl;
		}
		
		void StartScenarii(int scenarid)
		{
			scen_ids.push_back(scenarid);
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

		void ResetAll()
		{
			cout << "ResetAll CALLED IN DELTA:" << endl;
		}

		void BlackOut()
		{
			cout << "BlackOut CALLED IN DELTA:" << endl;
		}


		void TestDmx()
		{
			TICK_INTERVAL = 25;  // in ms
			FADE_INTERVAL = 5000;// in ms
			START = "15.15.215.100.255.255.255.100.0.100.0.0.0.0.0.0.0.255.100.0.100.0.0.255.215.100.255.255.255.100.0.100";
			END =   "250.250.25.100.255.255.255.100.0.100.0.0.0.0.0.0.0.255.100.0.100.0.55.255.215.100.255.255.255.100.0.100";

			START = PrepareString(START);
			log(START);
			vector<int> arrStart = GetVector(START);
			m = "START vector size:";
			m.append(to_string(arrStart.size()));
			log(m);
	
			END = PrepareString(END);
			log(END);
			vector<int> arrEnd = GetVector(END);
			m = "END vector size:";
			m.append(to_string(arrEnd.size()));
			log(m);

			FRAME = arrStart;
			
		 	ola:InitLogging(ola::OLA_LOG_WARN, ola::OLA_LOG_STDERR);
			log("InitLogging done");
		 	
		 	ola::client::OlaClientWrapper wrapper;			
		 	if (!wrapper.Setup()) { 		
		 		log("OlaClientWrapper Setup failed");
		 		exit(1);
		 	}
			log("Client OlaClientWrapper done");
			
		 	ola::io::SelectServer *ss = wrapper.GetSelectServer();
			log("SelectServer done");
			
			static unsigned int universe = 1;
			static unsigned int pause = 25;
		 	// ss->RegisterRepeatingTimeout(pause, ola::NewCallback(&SendData, &wrapper, universe, arrStart, arrEnd));

			ss->RegisterRepeatingTimeout(25, ola::NewCallback(&SendDataOld, &wrapper));

			log("RegisterRepeatingTimeout done");
		 	// Start the main loop
		 	log("Run about to start");
		 	ss->Run();
		}

		static bool SendDataOld(ola::client::OlaClientWrapper *wrapper) 
		{
		 	static unsigned int universe = 1;
			static unsigned int i = 0;
			ola::DmxBuffer buffer;
			buffer.Blackout();
			buffer.SetChannel(0, i);
			wrapper->GetClient()->SendDMX(universe, buffer, ola::client::SendDMXArgs());
			if (++i == 100) 
			{
				 wrapper->GetSelectServer()->Terminate();
		 	}
		 	return true;
		}
/*
		static bool SendData(ola::client::OlaClientWrapper *wrapper, unsigned int universe, vector<int> start, vector<int> end) 
		{	
		 	//static unsigned int universe = 1;
		 	static unsigned int i = 0, _counter = 0, _ticks = 0;

			//validate _ticks>0 always
			if (FADE_INTERVAL == 0 || TICK_INTERVAL == 0) {
				log("INVALID TICK: HALT !!!!!!");
				wrapper->GetSelectServer()->Terminate();
				exit(0);
			}
			_ticks = FADE_INTERVAL / TICK_INTERVAL;
		 	m = "TICK_INTERVAL:";
		 	m.append(to_string(TICK_INTERVAL));
			m.append(" FADE_INTERVAL:");
		 	m.append(to_string(FADE_INTERVAL));
			m.append(" _ticks:");
		 	m.append(to_string(_ticks));
			m.append(" iterations count:");
			m.append(to_string(i));
		 	log(m);
			// delta compute
			vector<int> delta;
			for(int d=0; d<start.size(); d++) {
				int _delta = (float)(end[d] - start[d]) / _ticks;
				delta.push_back(_delta);
			}

			if (i==0) {
				string _sdelta = BuildFrame(delta);
				m = "DELTA:";
				m.append(_sdelta);
				log(m);
			}
	
			string _frame = BuildFrame(FRAME);
			m = "FRAME:";
			m.append(_frame);
			log(m);
	

			ola::DmxBuffer buffer;
		 	buffer.Blackout();
		 	//buffer.SetChannel(0, i);
			buffer.SetFromString(_frame);
		 	wrapper->GetClient()->SendDMX(universe, buffer, ola::client::SendDMXArgs());

			//compute next frame
			for(int n= 0; n< FRAME.size(); n++) {
				FRAME[n] = FRAME[n] + delta[n];		
			}

			if (++i == start.size()) {
				wrapper->GetSelectServer()->Terminate();
			}
			return true;
		}*/

		
};

#endif
