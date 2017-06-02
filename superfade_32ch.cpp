#include <string>
#include <vector>
#include <time.h>
#include <ola/DmxBuffer.h>
#include <ola/io/SelectServer.h>
#include <ola/Logging.h>
#include <ola/client/ClientWrapper.h>
#include <ola/Callback.h>
using namespace std;
using std::cout;
using std::endl;

bool SendData(ola::client::OlaClientWrapper *wrapper) {
 static unsigned int universe = 1;
 static unsigned int i = 0;
 ola::DmxBuffer buffer;
 buffer.Blackout();
  stringstream ssf;
  ssf.str("");
 ssf << i<<"," <<i << ","<< i;
	string frame = ssf.str();
 cout << "frame " <<frame << endl;
 
 //buffer.SetChannel(i, frame);
 buffer.SetFromString(frame);
 wrapper->GetClient()->SendDMX(universe, buffer, ola::client::SendDMXArgs());
 if (++i == 10) {
 wrapper->GetSelectServer()->Terminate();
 }
 return true;
}
int main(int, char *[]) {
 ola::InitLogging(ola::OLA_LOG_WARN, ola::OLA_LOG_STDERR);
 ola::client::OlaClientWrapper wrapper;
 if (!wrapper.Setup()) {
 std::cerr << "Setup failed" << endl;
 exit(1);
 }
 // Create a timeout and register it with the SelectServer
 ola::io::SelectServer *ss = wrapper.GetSelectServer();
 ss->RegisterRepeatingTimeout(250, ola::NewCallback(&SendData, &wrapper));
 // Start the main loop
 ss->Run();
}


/*
g++ superfade_32ch.cpp $(pkg-config --cflags --libs libola) -std=c++11

unsigned int TICK_INTERVAL = 1000;  // in ms
unsigned int FADE_INTERVAL = 5000;// in ms
//string START = "15.15.215.100.255.255.255.100.0.100.0.0.0.0.0.0.0.255.100.0.100.0.0.255.215.100.255.255.255.100.0.100";
string START = "0.0.0.0";
//string END =   "255.250.25.100.255.255.255.100.0.100.0.0.0.0.0.0.0.255.100.0.100.0.55.255.215.100.255.255.255.100.0.100";
string END =   "255.255.255.255";
static vector<int> FRAME;
string m;

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}

void Log(const string& msg){
	cout << msg << endl;	
}

string BuildFrame(vector<int> frame) {
	string result="";
	for(int f=0; f<frame.size(); f++) {
		result.append(to_string(frame[f]));
		if (f<frame.size()-1) {
			result.append(",");
		}
	}
	return result;
}

string PrepareString(string stringinput) {
	string str = stringinput;
	for (int i=0; i<str.length(); i++)
	{
	    if (str[i] == '.')
        	str[i] = ' ';
	}
	return str;
}

vector<int> GetVector(string stringinput) {
	vector<int> array;
	stringstream ss(stringinput);
	int temp;
	while (ss >> temp)
	    array.push_back(temp); 
	return array;
}

bool SendData(ola::client::OlaClientWrapper *wrapper, unsigned int universe, vector<int> start, vector<int> end) {	
 	//static unsigned int universe = 1;
 	static unsigned int i = 0, _counter = 0, _ticks = 0;

	//validate _ticks>0 always
	if (FADE_INTERVAL == 0 || TICK_INTERVAL == 0) {
		Log("INVALID TICK: HALT !!!!!!");
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
 	Log(m);
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
		Log(m);
	}
	
	string _frame = BuildFrame(FRAME);
	m = "FRAME:";
	m.append(_frame);
	Log(m);
	

	ola::DmxBuffer buffer;
 	buffer.Blackout();
 	//buffer.SetChannel(0, i);
	buffer.SetFromString(_frame);
 	wrapper->GetClient()->SendDMX(universe, buffer, ola::client::SendDMXArgs());

	//compute next frame
	FRAME[0] = FRAME[0] + 1;
	for(int n= 0; n< FRAME.size(); n++) {
		//FRAME[n] = FRAME[n] + delta[n];		
		//FRAME[0] = FRAME[0] + 1;		
	}

	if (++i == start.size()) {
		wrapper->GetSelectServer()->Terminate();
	}
	return true;
}

int main(int, char *[]) {
	for(int i=0; i<5; i++) {
		Log("");
	}
	Log("-------------------------------------------------------------------------------------");
	Log("START PROGRAM: CHECK START AND END STRING CONVERSION INTO vector<int> AND vector SIZE");
	Log(currentDateTime());
	Log("-------------------------------------------------------------------------------------");
	// vector<int> is easier to handle than array

	START = PrepareString(START);
	Log(START);
	vector<int> arrStart = GetVector(START);
	m = "START vector size:";
	m.append(to_string(arrStart.size()));
	Log(m);
	
	END = PrepareString(END);
	Log(END);
	vector<int> arrEnd = GetVector(END);
	m = "END vector size:";
	m.append(to_string(arrEnd.size()));
	Log(m);

	FRAME = arrStart;
 	ola:InitLogging(ola::OLA_LOG_WARN, ola::OLA_LOG_STDERR);
	Log("InitLogging done");
 	
 	ola::client::OlaClientWrapper wrapper;
	Log("Client OlaClientWrapper done");
 	if (!wrapper.Setup()) { 		
 		Log("Setup failed");
 		exit(1);
 	}
	
 	ola::io::SelectServer *ss = wrapper.GetSelectServer();
	Log("SelectServer done");
	
	static unsigned int universe = 1;
	static unsigned int pause = 2500;
 	ss->RegisterRepeatingTimeout(pause, ola::NewCallback(&SendData, &wrapper, universe, arrStart, arrEnd));
	Log("RegisterRepeatingTimeout done");
 	// Start the main loop
 	Log("Run about to start");
 	ss->Run();
}
*/


