#ifndef SERVERAPP_H_
#define SERVERAPP_H_

#include "RakNetTypes.h"
#include <map>
#include <RakString.h>
#include <string>
#include <time.h>
#include <stdlib.h>

class HGE;

#define NEW_POWERUP_DELAY_TIMER 15.f
#define FRAMETIME 0.01f

class RakPeerInterface;

using std::string;

struct GameObject 
{
	GameObject(unsigned int newid)
	: x_(0), y_(0), type_(1)
	{
		id = newid;
	}

	string name;
	unsigned int id;
	float x_;
	float y_;
	int type_;
};

class ServerApp
{
	RakPeerInterface* rakpeer_;
	typedef std::map<SystemAddress, GameObject> ClientMap;

	ClientMap clients_;

	unsigned int newID;
	RakNet::RakString rs;
	
	void SendWelcomePackage(SystemAddress& addr);
	void SendDisconnectionNotification(SystemAddress& addr);
	void ProcessInitialPosition( SystemAddress& addr, float x_, float y_, int type_, string name_);
	void UpdatePosition( SystemAddress& addr, float x_, float y_ );

	HGE* hge_;
	float pwrUp_SpawnTimer;
	float x, y;
	int sendCount;

public:
	ServerApp();
	~ServerApp();
	void Loop();
};

#endif