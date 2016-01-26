#include "ServerApp.h"
#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "Bitstream.h"
#include "GetTime.h"
#include "../MyMsgIDs.h"
#include <iostream>

ServerApp::ServerApp() :
rakpeer_(RakNetworkFactory::GetRakPeerInterface()),
newID(0),
pwrUp_SpawnTimer(0.f),
sendCount(0),
x(rand() % 600 + 100),
y(rand() % 500 + 100)
{
	rakpeer_->Startup(100, 30, &SocketDescriptor(1691, 0), 1);
	rakpeer_->SetMaximumIncomingConnections(100);
	rakpeer_->SetOccasionalPing(true);
	std::cout << "Server Started" << std::endl;
	srand(time(NULL));
}

ServerApp::~ServerApp()
{
	rakpeer_->Shutdown(100);
	RakNetworkFactory::DestroyRakPeerInterface(rakpeer_);
}

void ServerApp::Loop()
{
	if (Packet* packet = rakpeer_->Receive())
	{
		RakNet::BitStream bs(packet->data, packet->length, false);

		unsigned char msgid = 0;
		RakNetTime timestamp = 0;

		bs.Read(msgid);

		if (msgid == ID_TIMESTAMP)
		{
			bs.Read(timestamp);
			bs.Read(msgid);
		}

		switch (msgid)
		{
		case ID_NEW_INCOMING_CONNECTION:
			SendWelcomePackage(packet->systemAddress);
			break;

		case ID_DISCONNECTION_NOTIFICATION:
		case ID_CONNECTION_LOST:
			SendDisconnectionNotification(packet->systemAddress);
			break;

		case ID_INITIALPOS:
		{
							  float x_, y_;
							  int type_;
							  std::cout << "ProcessInitialPosition" << std::endl;
							  bs.Read(rs);
							  bs.Read(x_);
							  bs.Read(y_);
							  bs.Read(type_);
							  ProcessInitialPosition(packet->systemAddress, x_, y_, type_, rs.C_String());
		}
			break;

		case ID_MOVEMENT:
		{
							float x, y;
							unsigned int shipid;
							bs.Read(shipid);
							bs.Read(x);
							bs.Read(y);
							UpdatePosition(packet->systemAddress, x, y);

							bs.ResetReadPointer();
							rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
		}
			break;

		case ID_COLLIDE:
		{
						   bs.ResetReadPointer();
						   rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
		}
			break;
		case ID_NEWPROJECTILE:
		{
								 bs.ResetReadPointer();
								 rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
		}
			break;
		case ID_UPDATEPROJECTILE:
		{
									bs.ResetReadPointer();
									rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
		}
			break;
		case ID_NEWPROXIMITYMINE:
		{
									bs.ResetReadPointer();
									rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
		}
			break;
		case ID_UPDATEPROXIMITYMINE:
		{
									   bs.ResetReadPointer();
									   rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
		}
			break;
		case ID_KILL_CREDIT:
			bs.ResetReadPointer();
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
			break;
		default:
			std::cout << "Unhandled Message Identifier: " << (int)msgid << std::endl;
		}

		if (pwrUp_SpawnTimer > NEW_POWERUP_DELAY_TIMER)
		{
			++sendCount;

			RakNet::BitStream bs;

			unsigned char msgid = ID_NEWPWRUP_PROJDMG;

			bs.Write(msgid);
			bs.Write(x);
			bs.Write(y);

			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
		}

		rakpeer_->DeallocatePacket(packet);
	}

	if (newID == 2)
	{
		pwrUp_SpawnTimer += FRAMETIME;

		if (sendCount == newID)
		{
			x = rand() % 600 + 100;
			y = rand() % 500 + 100;
			sendCount = 0;
			pwrUp_SpawnTimer = 0.f;
		}
	}
}

void ServerApp::SendWelcomePackage(SystemAddress& addr)
{
	//Only accept less than 2 clients
	++newID;
	if (newID <= 2)
	{
		unsigned int shipcount = static_cast<unsigned int>(clients_.size());
		unsigned char msgid = ID_WELCOME;

		RakNet::BitStream bs;
		bs.Write(msgid);
		bs.Write(newID);
		bs.Write(shipcount);

		for (ClientMap::iterator itr = clients_.begin(); itr != clients_.end(); ++itr)
		{
			std::cout << "Ship " << itr->second.id << " pos" << itr->second.x_ << " " << itr->second.y_ << std::endl;

			bs.Write(itr->second.id);
			bs.Write(itr->second.name.c_str());
			bs.Write(itr->second.x_);
			bs.Write(itr->second.y_);
			bs.Write(itr->second.type_);
		}

		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);

		bs.Reset();

		GameObject newobject(newID);

		clients_.insert(std::make_pair(addr, newobject));

		std::cout << "New guy, assigned id " << newID << std::endl;
	}
	//Reject new clients
	else
	{
		unsigned char msgid = ID_REJECTSHIP;

		RakNet::BitStream bs;
		bs.Write(msgid);

		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);

		bs.Reset();
	}
}

void ServerApp::SendDisconnectionNotification(SystemAddress& addr)
{
	--newID;
	ClientMap::iterator itr = clients_.find(addr);
	if (itr == clients_.end())
		return;

	unsigned char msgid = ID_LOSTSHIP;
	RakNet::BitStream bs;
	bs.Write(msgid);
	bs.Write(itr->second.id);

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, true);

	std::cout << itr->second.id << " has left the game" << std::endl;

	clients_.erase(itr);

}

void ServerApp::ProcessInitialPosition(SystemAddress& addr, float x_, float y_, int type_, string name_)
{
	unsigned char msgid;
	RakNet::BitStream bs;
	ClientMap::iterator itr = clients_.find(addr);
	if (itr == clients_.end())
		return;

	//itr->second.name = name_;
	itr->second.x_ = x_;
	itr->second.y_ = y_;
	itr->second.type_ = type_;
	itr->second.name = name_;

	//std::cout << "Received ship name" << itr->second.name << std::endl;
	std::cout << "Received pos" << itr->second.x_ << " " << itr->second.y_ << std::endl;
	std::cout << "Received type" << itr->second.type_ << std::endl;

	msgid = ID_NEWSHIP;
	//rs = itr->second.name.c_str();
	bs.Write(msgid);
	bs.Write(itr->second.id);
	bs.Write(itr->second.name.c_str());
	bs.Write(itr->second.x_);
	bs.Write(itr->second.y_);
	bs.Write(itr->second.type_);

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, true);
}

void ServerApp::UpdatePosition(SystemAddress& addr, float x_, float y_)
{
	ClientMap::iterator itr = clients_.find(addr);
	if (itr == clients_.end())
		return;

	itr->second.x_ = x_;
	itr->second.y_ = y_;
}