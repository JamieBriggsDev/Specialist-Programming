#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>       // Windows networking
#include "Network.h"
//#include "dynamicObjects.h"
//#include "ErrorLogger.h"    // WriteLn & error reporting
//#include "game.h"           // GameTimer
#include <string>           // use of strings for debugging
#include <WS2tcpip.h>       // inet_pton()
#include <iostream>
// For getting IP address
#include <fstream>
#include <locale>
#include <codecvt>


Network* Network::m_pInst = nullptr;
bool Network::m_isActive = false;

using namespace std;

Network::Network()
{
	//memset(&data, 0, sizeof(NetData));
	m_isHost = true;
	m_isHost = 1;
	m_dataRecieved = 0;
	m_nonBlocking = 1;
	m_MyIP = L"";
}

wchar_t * Network::GetLocalIP()
{
	string line;
	ifstream IPFile;
	int offset;
	char* search0 = "IPv4 Address. . . . . . . . . . . :";      // search pattern

	system("ipconfig > ip.txt");

	IPFile.open("ip.txt");
	if (IPFile.is_open())
	{
		while (!IPFile.eof())
		{
			getline(IPFile, line);
			if ((offset = line.find(search0, 0)) != string::npos)
			{
				//   IPv4 Address. . . . . . . . . . . : 1
				//1234567890123456789012345678901234567890     
				line.erase(0, 39);
				cout << line << endl;
				IPFile.close();
			}
		}
	}
	//wstring = to_wstring(line);

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//std::string narrow = converter.to_bytes(wide_utf16_source_string);
	std::wstring wide = converter.from_bytes(line);


	wchar_t* l_output = wide.c_str;
	return l_output;
}

void Network::CloseConnection()
{
	ExitClents();
	m_isActive = false;
	closesocket(m_socket);
	WSACleanup();
}

void Network::ExitClents()
{
	char l_exit[5] = "exit";

	for (int i = 0; i < m_clients.size(); i++)
	{
		sendto(m_socket, l_exit, sizeof(l_exit), 0,
			(struct sockaddr*) &m_clients[i], sizeof(m_clients[i]));
	}


}

bool Network::WSASetup()
{
	bool l_error = true;

	// Start up WSA
	if (WSAStartup(MAKEWORD(2, 0), &m_wsaData) != 0)
	{
		l_error = false;
	}

	return l_error;
}

bool Network::SetupServer()
{
	bool l_error = true;

	m_MyIP = GetLocalIP();

	// Setup windows socket
	WSASetup();

	// Create UDP
	if ((m_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		cout << "Error with socket function!" << endl;
		l_error = false;
	}

	// 
	memset(&m_serverAddress, 0, sizeof(m_serverAddress));
	m_serverAddress.sin_family = AF_INET;
	m_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	m_serverAddress.sin_port = htons(PORT);

	ioctlsocket(m_socket, FIONBIO, &m_nonBlocking);

	// Binds socket to be used
	bind(m_socket, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress));
	//{
	//	cout << "Error with binding socket!" << endl;
	//	l_error = false;
	//}

	return l_error;


}

void Network::CheckNewClients()
{
	// First make an address for the new client
	sockaddr_in l_clientAddress;
	memset(&l_clientAddress, 0, sizeof(l_clientAddress));
	int l_clientLength = sizeof(l_clientAddress);
	// Message to send to new client

	// Connected message
	char l_buffer[3];
	memset(&l_buffer, 0, sizeof(l_buffer));


	recvfrom(m_socket, l_buffer, sizeof(l_buffer), 0,
		(struct sockaddr*)&l_clientAddress, &l_clientLength);


	char l_connectMessage[] = "hi";

	// If client connects, send connected message and proceed to add client
	if (strcmp(l_buffer, l_connectMessage) == 0)
	{
		std::cout << "New Client!" << std::endl;
		SaveClents(l_clientAddress);
		InitData l_dataToSend;
		l_dataToSend.m_dataSent = 6.9;
		// Setup initial data here to send to new clients
		if (!sendto(m_socket, (char*)&l_dataToSend, sizeof(InitData), 0,
			(struct sockaddr*) &l_clientAddress, sizeof(l_clientAddress)))
		{
			cout << "Failed to send initial data!" << endl;
		}
	}

}

void Network::SaveClents(sockaddr_in _address)
{
	// Saves the client address so the Network class
	//  remebers
	m_clients.push_back(_address);
}



void Network::Send(SendData _data)
{
	// Send data to all clients
	m_data = _data;
	for (unsigned int i = 0; i < m_clients.size(); i++)
	{

		if (!sendto(m_socket, (char*)&m_data, sizeof(SendData), 0, (struct sockaddr*) &m_clients[i], sizeof(m_clients[i])))
		{
			cout << "Failed to send data" << endl;
		}
		/*ErrorLogger::Writeln(L"Error: sendto() - Send() - Wrong size - Exiting");*/
	}
}


//bool Network::ConnectToServer()
//{ // Initialises the client and attempts to connect to server asking to
//  // retrieve the initial data the server would send
//
//	WSASetup();
//
//	// Create socket
//	if ((m_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
//	{
//		cout << "Error with socket function connect!" << endl;
//		m_isHost = true;
//		return false;
//	}
//
//	memset(&m_serverAddress, 0, sizeof(m_serverAddress));     // 0 structure
//	m_serverAddress.sin_family = AF_INET;                   // Internet address
//	m_serverAddress.sin_addr.s_addr = inet_addr(IP);        // Server IP
//	m_serverAddress.sin_port = htons(PORT);                 // Local port
//
//														  //ioctlsocket(sock, FIONBIO, &nonblocking);
//
//														  // Create the connect message and send to the server, checking for error
//	char connectStr[] = "hi";
//	if (sendto(m_socket, connectStr, strlen(connectStr), 0, (struct sockaddr*) &m_serverAddress, sizeof(m_serverAddress)) != strlen(connectStr))
//	{
//		//ErrorLogger::Writeln(L"Error: sendto() - ConnectToServer() - Exiting");
//		cout << "Error with sendto() function!" << endl;
//		m_isHost = true;
//		return false;
//	}
//
//	// Create variables ready for recieving initial data
//	int recvlen = 0;
//	int addrLen = sizeof(m_serverAddress);
//	char buffer[sizeof(InitData)];
//	memset(&buffer, 0, sizeof(InitData));
//
//	// Recieve the initial data
//	recvlen = recvfrom(m_socket, buffer, sizeof(InitData), 0,
//		(struct sockaddr*) &m_serverAddress, &addrLen);
//
//	// Create initial data object and init to 0
//	InitData initData;
//	if (recvlen != -1)
//	{
//		memcpy(&initData, &buffer, sizeof(InitData));
//
//		// Set the timer until the next lot of score is set
//		//DynamicObjects::GetInstance()->SetScoreTimer(initData.scoreUpdateTimer);
//
//		// Set the scores for the teams, filthy way of doing it however supports any
//		// number of teams this way, will change if have time
//		//for (int i = 0; i < NUMTEAMS; i++)
//		//	DynamicObjects::GetInstance()->m_rgTeams[i].m_iScore = initData.scores[i];
//
//		//// Sets the owners of the domination points, one again hacky method but 
//		//// supports as many domination points as it needs, will change if have time
//		//for (int i = 0; i < NUMDOMINATIONPOINTS; i++)
//		//	DynamicObjects::GetInstance()->m_rgDominationPoints[i].m_OwnerTeamNumber = initData.dpStates[i];
//
//		m_dataRecieved = initData.m_dataSent;
//
//		ioctlsocket(m_socket, FIONBIO, &m_nonBlocking);
//	}
//	else
//	{
//		return false;
//	}
//
//	return true;
//} // ConnectToServer()

bool Network::Recieve()
{
	char l_buffer[sizeof(SendData)];
	memset(&l_buffer, 0, sizeof(SendData));
	memset(&m_data, 0, sizeof(SendData));

	int l_addresLength = sizeof(m_serverAddress);
	int recvLen = 0;
	recvfrom(m_socket, l_buffer, sizeof(SendData), 0,
		(struct sockaddr*) &m_serverAddress, &l_addresLength);

	char exit[] = "exit";
	if (strcmp(l_buffer, exit) == 0)
	{
		cout << "Exiting" << endl;
		return false;
	}

	memcpy(&m_data, &l_buffer, sizeof(SendData));

	// Team Score
	//cout << "Team 0 Score: " << m_data.m_team[0].m_teamScore << endl;
	//cout << "Team 1 Score: " << m_data.m_team[1].m_teamScore << endl;
	for (int i = 0; i < NUMTEAMS; i++)
	{
		for (int j = 0; j < NUMBOTSPERTEAM; j++)
		{
			// Position data
			//cout << "Team " << i << " Bot " << j << "Position = (";
			//cout << m_data.m_team[i].m_bots[j].m_posX << ", " <<
				//m_data.m_team[i].m_bots[j].m_posY << ")\n";
		}
	}
	//system("cls");
}



bool Network::ConnectToServer()
{
	// Connect to the server
	bool l_error = true;
	// First set up windows sockets
	WSASetup();

	// Next create the socket
	if ((m_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		m_isHost = true;
		l_error = false;

		//return l_error;
	}

	// Setup memory
	memset(&m_serverAddress, 0, sizeof(m_serverAddress));
	m_serverAddress.sin_family = AF_INET;
	m_serverAddress.sin_addr.s_addr = inet_addr(IP);
	m_serverAddress.sin_port = htons(PORT);

	char l_connect[] = "hi";
	if (sendto(m_socket, l_connect, strlen(l_connect), 0,
		(struct sockaddr*) &m_serverAddress, sizeof(m_serverAddress))
		!= strlen(l_connect))
	{
		cout << "Could not make initial connection\n";
		m_isHost = true;
		l_error = false;

		//return l_error;
	}

	// Create initdata struct and fill it
	InitData l_initData;
	// Create anything needed to receive initdata
	int l_recv = 0;
	int l_addressLength = sizeof(m_serverAddress);
	char l_buffer[sizeof(l_initData)];
	memset(&l_buffer, 0, sizeof(l_initData));

	l_recv = recvfrom(m_socket, l_buffer, sizeof(l_initData), 0,
		(struct sockaddr*)&m_serverAddress, &l_addressLength);
	// Get data
	//if (l_recv != -1)
	//{
	//	cout << "Receive Data failed\n";
	//	//return false;
	//}

	if (l_recv != -1)
	{
		// Fill initdata with l_buffer
		memcpy(&l_initData, &l_buffer, sizeof(l_initData));

		m_dataRecieved = l_initData.m_dataSent;


		ioctlsocket(m_socket, FIONBIO, &m_nonBlocking);

		cout << "Receive Data failed\n";
	}
	else
	{
		l_error = false;
		//return l_error;
	}

	return l_error;
}
