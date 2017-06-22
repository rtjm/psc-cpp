#ifndef _playscenari_h 
#define _playscenari_h

class Sequence
{
	public:
		int rank, id;
		float hold, fade;
		Sequence(int _rank, int _id, float _hold, float _fade)
		{
			rank = _rank;
			id = _id;
			hold = _hold;
			fade = _fade;
		}				
};

class PlayScenari
{
	private:
			
	protected:
	public:
		int scenari, tick_interval, loglevel = 1, current_i = -1;	
		bool _activescenari = true;
		int patch, patch_after, universe,  reverse, nextplay_stepid, hold_interval, fade_interval;
		float fade_ticks, hold_ticks, fade_counter, hold_counter;
		vector<int> playing, nextplay, _frame, pafter, new_frame;
		vector<float> _delta;
		vector<Sequence> sequence;				
		
		// CLASS CONSTRUCTOR. CALLED WHEN OBJECT CREATED						
		PlayScenari(int _scenari, int tickint)
		{
			scenari = _scenari;
			tick_interval = tickint;
			GetFixtureDetails();
			current_i = -1;
			GetNextStep();
			ChangeStep();
			cout << "PlayScenari INIT: scenari " << scenari << " - tickint " << tickint << endl;
			//exit(0);
		}			
		
		void GetFixtureDetails()
		{
			string query = "SELECT Id, Id_fixture FROM dmx_scensum WHERE Id=";
			query += to_string(scenari);
			// cout << query << endl;
			
			// SQL Scen infos
			MYSQL_RES *scendet = openRecordset(query);
			
			while (auto scenrow = mysql_fetch_row(scendet)) 
			{ 
				query ="SELECT patch,patch_after,univ FROM dmx_fixture WHERE Id=";
				query += scenrow[1];
				// SQL Fixture infos
				MYSQL_RES *fixtdet = openRecordset(query);
				while (auto fixtrow = mysql_fetch_row(fixtdet))
				{
					patch = stoi(fixtrow[0]);
					patch_after = stoi(fixtrow[1]);
					universe = stoi(fixtrow[2]);
				}
				
				cout << "PATCH:" << patch << " | PATCH_AFTER:" << patch_after <<  " | UNIVERSE:" << universe << endl;
			}
			
			// Change patch to meet universe zone
			if(universe > 1)
			{
				patch=patch + (512 * (universe - 1));
			}					
			pafter.clear();
			// Fill zeros if splitted fixture			
			for (int i = 0; i < patch_after; i++)
			{
				pafter.push_back(0);
			}
			log("buildDmxFrame from pafter");
			cout << buildDmxFrame(pafter);
		} 
		
		void GetNextStep()
		{
			if (loglevel > 0)
			{
				cout << "GetNextStep: Define the next step for scenari " << scenari << endl;
			}
			//exit(0);
			string query = "SELECT Id, reverse FROM dmx_scensum WHERE Id=";
			query += to_string(scenari);
						
			// SQL Scen infos
			MYSQL_RES *scendet = openRecordset(query);
			string way = "ASC";
		
			while (auto scenrow = mysql_fetch_row(scendet)) 
			{ 
				reverse = stoi(scenrow[1]);
				if (reverse != 0)
				{
					way = "DESC";
				}
				
				if (loglevel > 0)
				{
					cout << "Way: " << way << endl;
				}
				
				// SQL Sequence
				query = "SELECT id,hold,fade FROM dmx_scenseq WHERE disabled !=1 AND id_scenari=";
				query += to_string(scenari);
				query += " ORDER BY position ";
				query += way;
				query += " ,id ";
				query += way;
				
				cout << query << endl;
				
				// Get sequence resultset from database and populate vector of object
				MYSQL_RES *seq = openRecordset(query);
				sequence.clear();
				while(auto seqrow = mysql_fetch_row(seq))
				{
					int _id = stoi(seqrow[0]);
					float _hold = stof(seqrow[1]);
					float _fade = stof(seqrow[2]);
					Sequence _sequence(sequence.size(), _id, _hold, _fade);
					sequence.push_back(_sequence);
					cout << sequence.size() << " | " << _id  << " | " << _hold  << " | " << _fade << endl; 
				}
				
				int seqrows = sequence.size();
				cout << "SEQROWS " << seqrows << endl;
				if (seqrows != 0) 
				{
					// Each time we call this function, increase i to get the  next step of sequence
					current_i++;
					if (loglevel > 0)
					{
						cout << "Current i in seq:" << current_i << endl;
					}
					
					// Reloop if needed
					if (current_i >= seqrows)
					{
						current_i = 0;
						if (loglevel > 0)
						{
							cout << "Reloop" << endl;
						}
					}
					
					// Get stepid
					nextplay_stepid = (sequence.at(current_i)).id;
					if (loglevel > 0)
						{
							cout << "Nextplay:"<< nextplay_stepid << endl;
						}
					
					// Compose frame for step
					nextplay = GetDmxFrame(nextplay_stepid);
				}
			}
		}
		
		void GetNextTicks()
		{
			cout << "GetNextTicks " << sequence.size() << endl;
			//exit(0);
			int seqrows = sequence.size();
			if (seqrows != 0) 
			{
				int hold_time = (sequence.at(current_i)).hold;
				int fade_time = (sequence.at(current_i)).fade;
				cout << "hold_time " << hold_time << endl;
				cout << "fade_time " << fade_time << endl;
				//exit(0);
				try
				{
					// milliseconds
					hold_interval = abs((int)((float)(hold_time) * 1000)); 
					fade_interval = abs((int)((float)(fade_time) * 1000)); 
					cout << "hold_interval " << hold_interval << endl;
					cout << "hold_interval " << hold_interval << endl;
					//exit(0);
				}
				catch (int e)
				{
					cout << "Bad times" << endl;
					hold_interval = 0;
					fade_interval = 0;
				}
				
				// Hold: reset counter and define ticks
				hold_ticks = (float)(hold_interval / tick_interval);
				hold_counter = hold_ticks;
				cout << "hold_ticks " << hold_ticks << endl;
				cout << "hold_counter " << hold_counter << endl;
				// Fade: reset counter and define ticks
				fade_ticks = (float)(fade_interval / tick_interval);
				fade_counter = fade_ticks;
				cout << "fade_ticks " << fade_ticks << endl;
				cout << "fade_counter " << fade_counter << endl;
				//exit(0);
			}
		}
		
		void GetNextDelta()
		{
			// If not zero fade, define delta
			if (fade_ticks !=0)
			{
				int items_to_compute = playing.size();
				if (nextplay.size() < playing.size())
				{
					items_to_compute = nextplay.size();
				}
				_delta.clear();
				for (int i = 0; i < items_to_compute; i++)
				{
					float b = playing.at(i);
					float a = nextplay.at(i);
					float d = float(b - a) / fade_ticks;
					_delta.push_back(d);
				}
				
				if(loglevel > 0)
				{
					//cout << "Delta:" << buildDmxFrame((vector<int>)_delta) << endl;
					cout << "Iter:" << fade_ticks << endl;
				}
			}
		}
		
		void SwitchFrame()
		{			
			// Frame just reached become playing one
			playing = nextplay;
			
			// Define first frame
			_frame = playing;
		}
		
		void ChangeStep()
		{
			cout << "ChangeStep" << endl;
			GetNextTicks();
			SwitchFrame();
			GetNextStep();
			GetNextDelta();
		}
		
		
		vector<int> GetDmxFrame(int step_id)
		{
			// Compose frame for one step
			bool setzero = false;
			string stralldmx = "";
			vector<int> alldmx, dmxnum;
			string query = "SELECT ch_value FROM dmx_scenari WHERE id_scenari=";
			query += to_string(scenari);
			query += " AND step=";
			query += to_string(step_id);
			query += " ORDER BY id";
			
			cout << "FRAMEDMX Query: " << query << endl;
			
			// SQL scenari infos
			MYSQL_RES *framedmx = openRecordset(query);
			
			while (auto scenrow = mysql_fetch_row(framedmx)) 
			{
				stralldmx += scenrow[0];
				stralldmx += ".";
				//int data = stoi(scenrow[0]);
				//alldmx.push_back(data);
			}
			stralldmx.pop_back();
			alldmx = GetVector(PrepareString(stralldmx));
			for(int i = 0; i < pafter.size(); i++)
			{
				alldmx.push_back(pafter.at(i));
			}
			if (loglevel > 0)
			{
				string dump = buildDmxFrame(alldmx);
				cout << "ALLDMX:" << dump << endl;
			}
			
			for(int i = 0; i < alldmx.size(); i++)
			{
				int dmx = alldmx.at(i);
				if(dmx >= 0 && dmx <= 255)
				{
					// Add valid channel
					dmxnum.push_back(dmx);
				}
				else
				{
					setzero = true;
					break;	
				}
			}
			if (setzero)
			{
				cout << "Bad values" << endl;
				// Set channels to zero
				dmxnum.clear();
				for(int i = 0; i < alldmx.size(); i++)
				{
					dmxnum.push_back(0);
				}
			}			
			
			// Return valid frame
			return dmxnum;
		}
		
		void ComputeNextFrame()
		{
			if (loglevel > 1)
			{
				cout << "ComputeNextFrame.scen." << scenari << endl;
			}
			
			if (_activescenari)
			{
				// Play hold first
				if (hold_counter != 0)	
				{
					hold_counter --;
					if (loglevel > 0)
					{
						cout << "Hold counter:" << hold_counter << " _frame.size() " << _frame.size() << " current_i " << current_i << endl;
					}
					new_frame.clear();
					// Playing frame
					for(int i = 0; i < _frame.size(); i++)
					{					
						int newdmx = _frame.at(i);
						new_frame.push_back(round(newdmx));
					}
				}
				
				// Play fade after
				if (hold_counter == 0 && fade_counter != 0)
				{
					fade_counter --;
					if (loglevel > 0)
					{
						cout << "Fade counter:" << fade_counter << endl;
					}
					
					// Compute fade frame
					new_frame.clear();				
					for (int i= 0; i < _frame.size(); i++)
					{
						if (i < _delta.size())
						{
							_frame.at(i) = _frame.at(i) + _delta.at(i);
							new_frame.push_back(round(_frame.at(i)));
						}
					}
				}
				
				// If all completed, call the next step
				if (hold_counter == 0 && fade_counter == 0)
				{
					if (loglevel > 0)
					{
						cout << "NEXT STEP" << endl;
					}
					ChangeStep();
				}
				
				cout << "NEW_FRAME : " << new_frame.size() << endl;
				//exit(0);
			}
		}
		
		void StopScenari()
		{
			_activescenari = false;
		}
};


#endif
