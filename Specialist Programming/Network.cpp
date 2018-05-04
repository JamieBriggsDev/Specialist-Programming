#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>       // Windows networking
#include "Network.h"
#include "ErrorLogger.h"    // WriteLn & error reporting
#include "game.h"           // GameTimer
#include <string>           // use of strings for debugging
#include <WS2tcpip.h>       // inet_pton()



Network* Network::m_pInst = nullptr;
bool Network::m_isActive = false;

Network::Network()
{
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
	char l_exit[5] = "EXIT";

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

	// Setup windows socket
	WSASetup();

	// Create UDP
	if ((m_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		l_error = false;
	}

	// 
	memset(&m_serverAddress, 0, sizeof(m_serverAddress));
	m_serverAddress.sin_family = AF_INET;
	m_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	m_serverAddress.sin_port = htons(PORT);

	ioctlsocket(m_socket, FIONBIO, &m_nonBlocking);

	// Binds socket to be used
	if (bind(m_socket, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress)) < 0)
	{
		l_error = false;
	}

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

	
	char l_connectMessage[] = "Connected!";

	// If client connects, send connected message and proceed to add client
	if (strcmp(l_buffer, l_connectMessage) == 0)
	{
		SaveClents(l_clientAddress);
		InitData l_dataToSend;
		l_dataToSend.m_dataSent = 6.9;
		// Setup initial data here to send to new clients
		sendto(m_socket, (char*)&l_dataToSend, sizeof(InitData), 0,
			(struct sockaddr*) &l_clientAddress, sizeof(&l_clientAddress));
	}

}

void Network::SaveClents(sockaddr_in _address)
{
	// Saves the client address so the Network class
	//  remebers
	m_clients.push_back(_address);
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

		return l_error;
	}

	// Setup memory
	memset(&m_serverAddress, 0, sizeof(m_serverAddress));
	m_serverAddress.sin_family = AF_INET;
	m_serverAddress.sin_addr.s_addr = inet_addr(IP);
	m_serverAddress.sin_port = htons(PORT);

	char l_connect[] = "Hi Host";
	if (sendto(m_socket, l_connect, strlen(l_connect), 0,
		(struct sockaddr*) &m_serverAddress, sizeof(m_serverAddress))
		!= strlen(l_connect))
	{
		m_isHost = true;
		l_error = false;

		return l_error;
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
	if (l_recv < 0)
	{
		return false;
	}

	if (l_recv != -1)
	{
		// Fill initdata with l_buffer
		memcpy(&l_initData, &l_buffer, sizeof(l_initData));

		m_dataRecieved = l_initData.m_dataSent;


		ioctlsocket(m_socket, FIONBIO, &m_nonBlocking);
	}
	else
	{
		l_error = false;
		return l_error;
	}

	return l_error; 
}
