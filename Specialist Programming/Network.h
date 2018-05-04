#pragma once

#include <Windows.h>
#include <process.h>

#include <vector>

#include <stdint.h>



class DynamicObjects;
#include "game.h"

#define PORT (560)
#define IP ("172.16.1.33")



class Network
{
private:
	Network();
	// Singleton instance
	static Network* m_pInst;
	// Socket to be used
	int m_socket;
	// The address of the server
	sockaddr_in m_serverAddress;
	// The thread used which waits for any connecitons
	DWORD thread;
	// Vector of all clients
	std::vector<sockaddr_in> m_clients;
	// Socket data for windows
	WSADATA m_wsaData;
	// Is connected

	GameTimer m_timer;

	// Data which needs to be passed through
	// Used to see if data gets sent accross the network
	struct InitData
	{
		double m_dataSent;
	};
	// Closes the connections
	void CloseConnection();
	// Tells clients to finish up
	void ExitClents();
	// Start up windows sockets
	bool WSASetup();
public:
	// Singleton stuff
	// Create
	static Network* GetInstance()
	{
		if (!m_pInst)
			m_pInst = new Network;

		return m_pInst;
	}
	// Delete
	static void Release()
	{
		if (m_pInst)
		{
			// Close all connections before delete
			m_pInst->CloseConnection();
			// Delete and set to nullptr
			delete m_pInst;
			m_pInst = nullptr;
		}
	}
	//NetData m_frameData;
	// States if the program is connected
	bool m_isHost;
	// Non blocking flag
	u_long m_nonBlocking;

	double m_dataRecieved;
	char comData[sizeof(double)];

	// Says if the server is currently active
	static bool m_isActive;

	// HOST SERVER STUFF
	// Sets up the server for use
	bool SetupServer();

	// Thread loops until told to quick which welcomes
	//  new clients
	void CheckNewClients();

	// Saves the cleitns in m_clents
	void SaveClents(sockaddr_in _address);

	// Sends data to every client saved
	void Send();

	// CLIENT STUFF
	// Connects to a server
	bool ConnectToServer();
	// Collect any data sent from the host
	void Recieve();



};

