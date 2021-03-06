#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>

#include "GC_TS3_Attendance.h"
#include "socketRSall.h"

using namespace std;

SOCKET TS3;
ifstream channels, names;

bool SetUp();
bool Channels(vector<string>&, bool);
bool Clients();
void CleanUp();
void ReplaceAll(string&, const string&, const string&);

int main(void)
{
	cout << "Global Conflict TS3 Attendance Tracker\n" << endl;
	cout << "Do you need to set up for the first time this campaign?" << endl;
	cout << "yes or no: ";
	string setup;
	cin >> setup;
	if(setup == "yes" || setup == "y")
	{
		if(!SetUp())
		{
			cout << "Set up failure." << endl;
			cout << "Please check TS3 is running and Client Quiery is on" << endl;
			cout << "Type anything to quit" << endl;
			cin >> setup;
			return 0;
		}
	}

	setup = "hmm";
	float time;

	while(setup.substr(0, 1) != "y")
	{
		cout << "\nHow many hours do you need this program to run (nearest half): ";
		cin >> time;
		cout << "\nAre you sure you want to run for " << time << " hours?" << endl;
		cout << "yes or no: ";
		cin >> setup;
		cout << endl;
	}

	cout << "Program will close once it's finished." << endl;
	cout << "Please leave this window open." << endl;
	cout << "Running . . . ";

	ofstream cli;
	cli.open("Clients.txt");
	cli << "";
	cli.close();

	int runs = time*4;

	for(int i = 0; i < runs; i++)
	{
		if(!Clients())
		{
			cout << "Client failure: skipping" << endl;
		}
		Sleep(900000);		//wait 15mins
		//Sleep(5000);			//wait 5secs for debug
	}
	if(!Clients())	//call last iteration separatly so there is no wait for processing
	{
		cout << "Client failure: skipping" << endl;
	}

	cout << "Complete!" << endl;

	CleanUp();

	cout << "Done!" << endl;

	return 0;
}

bool SetUp()
{
	cout << "\nExample Army Tree:\n" << endl;
	cout << "Army Name" << endl;
	cout << "    Main Channel" << endl;
	cout << "    Officer Channel" << endl;
	cout << "    FC Channel" << endl;
	cout << "    Division 1 Channel" << endl;
	cout << "        Div 1 Squad 1" << endl;
	cout << "        ..." << endl;
	cout << "        Div 1 Squad n" << endl;
	cout << "    Division 2 Channel" << endl;
	cout << "        Div 2 Squad 1" << endl;
	cout << "        ..." << endl;
	cout << "        Div 2 Squad n" << endl;
	cout << "    Division 3 Channel" << endl;
	cout << "        Div 3 Squad 1" << endl;
	cout << "        ..." << endl;
	cout << "        Div 3 Squad n\n" << endl;

	if(!ConnectToHost(25639, "127.0.0.1", TS3))
	{
		return false;
	}

	bool iResult;
	char reci[181];
	memset(reci, 0, 181);
	SocketRSall io;

	iResult = io.recv_all(TS3, reci, 181 ,0);	//get TS3 Client...
	if (!iResult)
	{
		cout << "First Recieve Failure" << endl;
		CloseConnection(TS3);
		return false;
	}

	string cont;
	vector<string> cids;

	cout << "Please move to your equivalent of 'Div 3 Squad n'" << endl;
	cout << "When there, type ok: ";
	cin >> cont;
	cout << endl;
	if(!Channels(cids, false))
	{
		cout << "Channel List Failure" << endl;
		return false;
	}

	cout << "Please move to your equivalent of 'Div 2 Squad n'" << endl;
	cout << "When there, type ok: ";
	cin >> cont;
	cout << endl;
	if(!Channels(cids, false))
	{
		cout << "Channel List Failure" << endl;
		return false;
	}

	cout << "Please move to your equivalent of 'Div 1 Squad n'" << endl;
	cout << "When there, type ok: ";
	cin >> cont;
	cout << endl;
	if(!Channels(cids, false))
	{
		cout << "Channel List Failure" << endl;
		return false;
	}

	cout << "Please move to your equivalent of 'Division 3 Channel'" << endl;
	cout << "When there, type ok: ";
	cin >> cont;
	cout << endl;
	if(!Channels(cids, false))
	{
		cout << "Channel List Failure" << endl;
		return false;
	}

	cout << "Please move to your equivalent of 'Army Name'" << endl;
	cout << "When there, type ok: ";
	cin >> cont;
	cout << endl;
	if(!Channels(cids, true))
	{
		cout << "Channel List Failure" << endl;
		return false;
	}
	

	ofstream file;
	file.open("Channels.txt");
	for(int i = 0; i < static_cast<int>(cids.size()); i++)
	{
		file << cids[i] << endl;
	}
	file.close();

	CloseConnection(TS3);
	cout << "Set Up Complete\n" << endl;
	return true;
}

bool Channels(vector<string>& cids, bool single)
{
	bool iResult;
	char* whoami = "whoami\n";
	char reci1[64];
	char reci2[256];
	SocketRSall io;
	
	string identstart = "cid=";
	string identend = "\n";
	size_t startpos, endpos;

	string input, output, cid;

	iResult = io.send_all(TS3, whoami, (int)strlen(whoami), 0);	//io.send_all whoami
	if (!iResult)
	{
		cout << "whoami Send Failure" << endl;
		CloseConnection(TS3);
		return false;
	}
	iResult = io.recv_all(TS3, reci1, 64 ,0, "msg=");		//get whoami
	if (!iResult)
	{
		cout << "whoami Recieve Failure" << endl;
		CloseConnection(TS3);
		return false;
	}

	input = reci1;
	startpos = input.find(identstart);
	endpos = input.find(identend);
	input = input.substr(startpos, endpos - startpos);
	cid = input.substr(4);

	cids.push_back("cid=");
	int i = cids.size();
	cids[i-1].append(cid);

	if(single)
	{
		return true;
	}

	identstart = "channel_order=";

	while(cid != "0")
	{
		memset(reci2, 0, 256);

		output = "channelvariable cid=";
		output.append(cid);
		output.append(" channel_order\n");
		const char *order = output.c_str();

		iResult = io.send_all(TS3, order, (int)strlen(order) ,0);
		if (!iResult)
		{
			cout << "order Send Failure" << endl;
			CloseConnection(TS3);
			return false;
		}
		iResult = io.recv_all(TS3, reci2, 256 ,0, "msg=");		//recieve channel_order
		if (!iResult)
		{
			cout << "order Recieve Failure" << endl;
			CloseConnection(TS3);
			return false;
		}

		input = reci2;
		startpos = input.find(identstart);
		endpos = input.find(identend);
		input = input.substr(startpos, endpos - startpos);
		cid = input.substr(14);

		if(cid != "0")
		{
			cids.push_back("cid=");
			cids[i].append(cid);
			i++;
		}
	}

	return true;
}

bool Clients()
{
	ifstream chan;
	vector<string> cids;
	string nextcid = "filler";

	chan.open("Channels.txt");
	if(!chan.is_open())
	{
		cout << "Channels.txt is missing. Please run the setup." << endl;
		return false;
	}

	while(!nextcid.empty())
	{
		getline(chan, nextcid);
		if(!nextcid.empty())
		{
			cids.push_back(nextcid);
		}
	}
	chan.close();

	if(!ConnectToHost(25639, "127.0.0.1", TS3))
	{
		cout << "Clients connection failure" << endl;
		return false;
	}

	int iResult;
	char reci[181];
	char reci2[8192];
	memset(reci, 0, 181);
	memset(reci2, 0, 8192);
	SocketRSall io;

	iResult = io.recv_all(TS3, reci, 181 ,0);	//get TS3 Client...
	if (!iResult)
	{
		cout << "First Recieve Failure" << endl;
		CloseConnection(TS3);
		return false;
	}

	string input, output, name;
	string identstart = "client_nickname=";
	string identend = " client_type=";
	size_t startpos, endpos;

	ofstream file;
	file.open("Clients.txt", ios::out | ios::app);

	for(int i = 0; i < static_cast<int>(cids.size()); i++)
	{
		output = "channelclientlist ";
		output.append(cids[i]);
		output.append("\n");
		const char *nicks = output.c_str();

		iResult = io.send_all(TS3, nicks, (int)strlen(nicks) ,0);
		if (!iResult)
		{
			cout << "nicks Send Failure" << endl;
			CloseConnection(TS3);
			return false;
		}
		iResult = io.recv_all(TS3, reci2, 8192 ,0, "msg=");		//recieve channelcli...
		if (!iResult)
		{
			cout << "order Recieve Failure" << endl;
			CloseConnection(TS3);
			return false;
		}
		input = reci2;

		int count = 0;
		while(TRUE)
		{
			startpos = input.find(identstart);
			if(startpos == -1 || startpos > 8192)
			{
				break;
			}
			endpos = input.find(identend);
			if(endpos < 0 || endpos > 8192)
			{
				break;
			}
			name = input.substr(startpos + 16, endpos - startpos - 16);
			input = input.substr(endpos + 14);
			file << name << endl;
			count++;
		}
		memset(reci2, 0, 8192);
	}
	CloseConnection(TS3);
	file.close();

	return true;
}

void CleanUp()
{
	cout << "Condensing List . . . ";
	ifstream cli;
	string nextname;
	vector<string> names;

	cli.open("Clients.txt");

	getline(cli, nextname);
	names.push_back(nextname);

	int len;
	bool same = false;
	while(!nextname.empty())
	{
		getline(cli, nextname);
		if(!nextname.empty())
		{
			len = strlen(nextname.c_str());
			same = false;
			//check for #/#
			if(len > 1 && nextname.substr(len - 2, 1) == "/")
			{
				nextname = nextname.substr(0, len-4);
				Sleep(1);
				//check for extra \s
				len = strlen(nextname.c_str());
				if(len > 1 && nextname.substr(len - 2, 2) == "\\s")
				{
					nextname = nextname.substr(0, len-2);
					Sleep(1);
				}
			}
			//check for #/## or ##/##
			else if(len > 2 && nextname.substr(len - 3, 1) == "/")
			{
				nextname = nextname.substr(0, len-5);
				Sleep(1);
				//check for extra 1
				len = strlen(nextname.c_str());
				if(nextname.substr(len - 1, 1) == "1")
				{
					nextname = nextname.substr(0, len-1);
					Sleep(1);
					len = strlen(nextname.c_str());
				}
				//check for extra \s
				if(len > 1 && nextname.substr(len - 2, 2) == "\\s")
				{
					nextname = nextname.substr(0, len-2);
					Sleep(1);
				}
			}
			//check for \s#
			else if(len > 2 && nextname.substr(len - 3, 1) == "\\")
			{
				nextname = nextname.substr(0, len-3);
				Sleep(1);
			}
			//check for \s##
			else if(len > 3 && nextname.substr(len - 4, 1) == "\\")
			{
				nextname = nextname.substr(0, len-4);
				Sleep(1);
			}
			//check for (#)
			else if(nextname.substr(len-1, 1) == ")")
			{
				nextname = nextname.substr(0, len-3);
				Sleep(1);
				//check for extra ( if (##)
				len = strlen(nextname.c_str());
				if(nextname.substr(len - 1, 1) == "(")
				{
					nextname = nextname.substr(0, len-1);
					Sleep(1);
					len = strlen(nextname.c_str());
				}
				//check for extra \s
				if(len > 1 && nextname.substr(len - 2, 2) == "\\s")
				{
					nextname = nextname.substr(0, len-2);
					Sleep(1);
				}
			}
			//check for the same name
			for(int j = 0; j < static_cast<int>(names.size()); j++)
			{
				if(nextname == names[j])
				{
					same = true;
					break;
				}
			}
			//if not the same, add to vector
			if(!same)
			{
				names.push_back(nextname);
			}
		}
	}
	cli.close();

	//remove "\s" and replace with " "
	const string bad = "\\s";
	const string good = " ";
	for(int i = 0; i < static_cast<int>(names.size()); i++)
	{
		ReplaceAll(names[i], bad, good);
	}

	//Get Date
	time_t now = time(0);
	tm* localtm = localtime(&now);
	//and make file name
	stringstream filename;
	filename << "Attendance-"
			 << localtm->tm_year + 1900 << "-"
			 << localtm->tm_mon + 1 << "-"
			 << localtm->tm_mday
			 << ".txt";

	ofstream atten;
	atten.open(filename.str());
	for(int i = 0; i < static_cast<int>(names.size()); i++)
	{
		atten << names[i] << endl;
	}
	atten.close();
	cout << "Complete!" << endl;
	return;
}

void ReplaceAll(string &str, const string& from, const string& to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != string::npos)
	{
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return;
}