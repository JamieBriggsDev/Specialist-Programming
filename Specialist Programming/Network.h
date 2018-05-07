#pragma once

#include <Windows.h>
#include <process.h>
#include <vector>
#include <stdint.h>
#include "rules.h"




class DynamicObjects;
//#include "game.h"

#define PORT (8645)
 #define IP ("172.16.1.37")
//#define IP ("172.16.1.35")



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
	wchar_t* GetLocalIP();
	//GameTimer m_timer;

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
	struct BotData
	{
		double m_posX;
		double m_posY;
		double m_velX;
		double m_velY;
		double m_dir;
		bool m_isAlive;
		bool m_isAiming;
		double m_shootCooldown;
		int m_ammo;
	};

	struct ShootData
	{
		int m_targetTeam;
		int m_botNumber;
		int m_damage;
		bool m_isFiring;
	};

	struct TeamData
	{
		int m_teamScore;
		ShootData m_shootData[NUMBOTSPERTEAM];
		BotData m_bots[NUMBOTSPERTEAM];
	};

	struct SendData
	{
		TeamData m_team[NUMTEAMS];
	};
	// Singleton stuff
	// Create
	static Network* GetInstance()
	{
		if (!m_pInst)
		{
			m_pInst = new Network;
			m_isActive = true;
		}

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
	// Data to be sent
	SendData m_data;
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
	void Send(SendData _data);

	// CLIENT STUFF
	// Connects to a server
	bool ConnectToServer();
	// Collect any data sent from the host
	bool Recieve();
	// Returns how many clients there are
	int GetNumberOfClients() { return m_clients.size(); }

	wchar_t* m_MyIP;


	const char* m_hostIP = "172.16.1.37";
};

