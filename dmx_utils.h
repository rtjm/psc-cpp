#ifndef _dmx_utils_h 
#define _dmx_utils_h

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <time.h>
#include <mysql/mysql.h>

using namespace std;
using std::cout;
using std::endl;



void error(const char *msg)
{
	perror(msg);
	exit(1);
}


string buildDmxFrame(vector<int> frame) {
	string result="";
	for(int f=0; f<frame.size(); f++) {
		result.append(to_string(frame[f]));
		if (f<frame.size()-1) {
			result.append(",");
		}
	}
	return result;
}

string buildDmxFrame(vector<float> frame) {
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


vector<int> InitVector(int universes)
{
	vector<int> result;
	for(int i = 0; i< universes; i++)
	{
		for(int j = 0; j <512; j++)
		{
			result.push_back(0);
		}
	}
	return result;
}

vector<int> GetVector(string stringinput) {
	vector<int> array;
	stringstream ss(stringinput);
	int temp;
	while (ss >> temp)
	    array.push_back(temp); 
	return array;
}


bool hasOnlyDigits(const string s){
	if (s=="") return false;
  	return s.find_first_not_of( "0123456789" ) == string::npos;
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const string currentDateTime() 
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}

void log(const string& msg){
	cout << "CONSOLE LOG - " << currentDateTime() << "-:" << msg << endl;	
}

void stripNewlines(char* p) {
    char* q = p;
    while (p != 0 && *p != '\0') {
        if (*p == '\n') {
            p++;
            *q = *p;
        } 
        else {
            *q++ = *p++;
        }
    }
    *q = '\0';
}

vector<string> getParams()
{
	vector<string> result;
	std::ifstream fileStream("config.py");
	string param;
	if(!fileStream.good())
	{
	    std::cerr << "HOSTCONFIG: could not open config file." << std::endl;
		return result;
	}
	while(fileStream >> param)
	{           
		result.push_back(param);
	}
	return result;
}

string removeChar(string str, char c) 
{
   string result;
   for (size_t i = 0; i < str.size(); i++) 
   {
          char currentChar = str[i];
          if (currentChar != c)
              result += currentChar;
   }
       return result;
}

string GetKey(string key)
{
	vector<string> params = getParams();
	string result;
	for(int i=0; i<params.size(); i++) 
	{
		if(params[i].find(key)!= std::string::npos)
		{
			result = params[i].substr(key.length());
			result = removeChar(result,'=');
			result = removeChar(result,'"');
	        return result;
		}
	}
return result;
}

vector<string> explode(const string& str, const char& ch) {
    string next;
    vector<string> result;
    // For each character in the string
    for (string::const_iterator it = str.begin(); it != str.end(); it++) {
        // If we've hit the terminal character
        if (*it == ch) {
            // If we have some characters accumulated
            if (!next.empty()) {
                // Add them to the result vector
                result.push_back(next);
                next.clear();
            }
        } else {
            // Accumulate the next character into the sequence
            next += *it;
        }
    }
    if (!next.empty())
         result.push_back(next);
    return result;
}

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

MYSQL_RES* openRecordset(string query)
{	
	MYSQL_RES *resultset;
	MYSQL_ROW row;
	MYSQL *connection, mysql;
	string host = GetKey("DB_HOST");
	string usr = GetKey("DB_USER");
	string pswd = GetKey("DB_PASS");
	string database = GetKey("DB_BASE");
	mysql_init(&mysql);
	connection = mysql_real_connect(&mysql,host.c_str(),usr.c_str(),pswd.c_str(),database.c_str(),0,0,0);

   	if (mysql_query(connection, query.c_str())) 
	{
	  	finish_with_error(connection);
   	}

	resultset = mysql_store_result(connection);	
	return resultset;
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
	else
	{
		cout << "default to 40 fps" << endl;
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
	else
	{
		cout << "default to 1 universe" << endl;
	}
	return result;
}
#endif
