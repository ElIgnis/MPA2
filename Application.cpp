#include "Application.h"
#include "ship.h"
#include "Globals.h"
#include "MyMsgIDs.h"
#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "Bitstream.h"
#include "GetTime.h"
#include <hge.h>
#include <string>
#include <iostream>
#include <fstream>
#include <hgeFont.h>

// Lab 13 Task 9a : Uncomment the macro NETWORKPROJECTILE
#define NETWORKMISSLE


float GetAbsoluteMag(float num)
{
	if (num < 0)
	{
		return -num;
	}

	return num;
}

/**
* Constuctor
*
* Creates an instance of the graphics engine and network engine
*/

Application::Application()
: hge_(hgeCreate(HGE_VERSION))
, rakpeer_(RakNetworkFactory::GetRakPeerInterface())
, timer_(0)
, keydown_fire(false)
, keydown_mine(false)
, collision_X(0.f)
, collision_Y(0.f)
, showCursor(true)
, Left_X(0)
, Right_X(800)
, kills(0)
, deaths(0)
, sendKillCredits(false)
, connection_rejected(false)
, nameProcessed(false)
, nameTooShort(false)
, textDelay(0.f)
{
}

/**
* Destructor
*
* Does nothing in particular apart from calling Shutdown
*/

Application::~Application() throw()
{
	font_.release();
	Shutdown();
	rakpeer_->Shutdown(100);
	RakNetworkFactory::DestroyRakPeerInterface(rakpeer_);
}

/**
* Initialises the graphics system
* It should also initialise the network system
*/

bool Application::Init()
{
	std::ifstream inData;
	std::string serverip;

	inData.open("serverip.txt");

	inData >> serverip;

	srand(RakNet::GetTime());

	hge_->System_SetState(HGE_FRAMEFUNC, Application::Loop);
	hge_->System_SetState(HGE_WINDOWED, true);
	hge_->System_SetState(HGE_USESOUND, false);
	hge_->System_SetState(HGE_TITLE, "Movement");
	hge_->System_SetState(HGE_LOGFILE, "movement.log");
	hge_->System_SetState(HGE_DONTSUSPEND, true);

	if (hge_->System_Initiate())
	{
		//Load Images
		bg_tex_ = hge_->Texture_Load("Images/background.png");
		background_Left.reset(new hgeSprite(bg_tex_, 0, 0, 800, 600));
		background_Right.reset(new hgeSprite(bg_tex_, 0, 0, 800, 600));

		//Cursor
		cursor_tex = hge_->Texture_Load("Images/HUD/cursor.png");
		cursor_.reset(new hgeSprite(cursor_tex, 0, 0, 9, 17));

		//Initialise ships
		ships_.push_back(new Ship(rand() % 3 + 1, rand() % 500 + 100, rand() % 400 + 100));

		//Weapon icons
		tex_pwrlvl1 = hge_->Texture_Load(SPR_PROJ_PWRUP_LVLONE);
		tex_pwrlvl2 = hge_->Texture_Load(SPR_PROJ_PWRUP_LVLTWO);
		tex_pwrlvl3 = hge_->Texture_Load(SPR_PROJ_PWRUP_LVLTHREE);
		tex_pwrlvlmax = hge_->Texture_Load(SPR_PROJ_PWRUP_LVLMAX);

		tex_mine_unready = hge_->Texture_Load(SPR_MINE_UNREADY);
		tex_mine_ready = hge_->Texture_Load(SPR_MINE_READY);
		title_tex = hge_->Texture_Load(SPR_TITLE);

		pwrlvl1_.reset(new hgeSprite(tex_pwrlvl1, 0, 0, 64, 64));
		pwrlvl2_.reset(new hgeSprite(tex_pwrlvl2, 0, 0, 64, 64));
		pwrlvl3_.reset(new hgeSprite(tex_pwrlvl3, 0, 0, 64, 64));
		pwrlvlmax_.reset(new hgeSprite(tex_pwrlvlmax, 0, 0, 64, 64));

		mineunready_.reset(new hgeSprite(tex_mine_unready, 0, 0, 64, 64));
		mineready_.reset(new hgeSprite(tex_mine_ready, 0, 0, 64, 64));
		title_.reset(new hgeSprite(title_tex, 0, 0, 328, 263));

		//Initialise 50 explosion effects
		for (int i = 0; i < 50; ++i)
		{
			explosion_list.push_back(new explosion());
		}

		//Initialise 50 bullets
		for (int i = 0; i < 50; ++i)
		{
			local_projlist.push_back(new Projectile(SPR_PROJECTILE, ShipName));
		}

		//Initialise 5 mines
		for (int i = 0; i < 5; ++i)
		{
			local_minelist.push_back(new ProximityMine(ShipName));
		}

		//Font
		font_.reset(new hgeFont("font1.fnt"));
		font_->SetScale(1.0);

		//Connect to server
		if (rakpeer_->Startup(1, 30, &SocketDescriptor(), 1))
		{
			rakpeer_->SetOccasionalPing(true);
			return rakpeer_->Connect(serverip.c_str(), 1691, 0, 0);
		}
		
	}
	return false;
}

/**
* Update cycle
*
* Checks for keypresses:
*   - Esc - Quits the game
*   - Left - Rotates ship left
*   - Right - Rotates ship right
*   - Up - Accelerates the ship
*   - Down - Deccelerates the ship
*
* Also calls Update() on all the ships in the universe
*/
bool Application::Update()
{
	float timedelta = hge_->Timer_GetDelta();
	
	UpdateBG(timedelta);

	ProcessName();

	if (nameTooShort)
	{
		textDelay += timedelta;

		if (textDelay > TEXT_DELAY)
		{
			nameTooShort = false;
			textDelay = 0.f;
		}
	}

	if (hge_->Input_GetKeyState(HGEK_ESCAPE))
		return true;

	if (nameProcessed)
	{
		UpdateKeypress();
		UpdateShips(timedelta);
		UpdateProjectiles(timedelta);
		UpdateMines(timedelta);
		UpdateExplosions(timedelta);
		UpdatePowerups(timedelta);

		if (UpdatePackets(timedelta))
		{
			if (connection_rejected)
			{
				std::cin.ignore(255, '\n');
				std::cin.get();
			}
			return true;
		}
	}
	return false;
}

void Application::UpdateBG(float dt)
{
	//Background scrolling
	Left_X -= 20 * dt;
	if (Left_X < -800)
	{
		Left_X = 800;
	}
	if (Left_X > 800)
	{
		Left_X = -800;
	}

	Right_X -= 20 * dt;
	if (Right_X > 800)
	{
		Right_X = -800;
	}
	if (Right_X < -800)
	{
		Right_X = 800;
	}
}

void Application::UpdateKeypress()
{
	float timedelta = hge_->Timer_GetDelta();

	ships_.at(0)->SetAngularVelocity(0.0f);

	//Movement keys
	if (ships_.at(0)->GetAlive())
	{
		if (hge_->Input_GetKeyState(HGEK_LEFT))
		{
			ships_.at(0)->SetAngularVelocity(ships_.at(0)->GetAngularVelocity() - DEFAULT_ANGULAR_VELOCITY);
		}

		if (hge_->Input_GetKeyState(HGEK_RIGHT))
		{
			ships_.at(0)->SetAngularVelocity(ships_.at(0)->GetAngularVelocity() + DEFAULT_ANGULAR_VELOCITY);
		}

		if (hge_->Input_GetKeyState(HGEK_UP))
		{
			ships_.at(0)->Accelerate(DEFAULT_ACCELERATION, timedelta);
		}

		if (hge_->Input_GetKeyState(HGEK_DOWN))
		{
			ships_.at(0)->Accelerate(-DEFAULT_ACCELERATION, timedelta);
		}

		//Projectile firing
		if (hge_->Input_GetKeyState(HGEK_J))
		{
			if (!keydown_fire)
			{
				CreateProjectile(ships_.at(0)->GetX(), ships_.at(0)->GetY(), ships_.at(0)->GetW(), ships_.at(0)->GetID(), ships_.at(0)->GetPower(), ships_.at(0)->GetName());
				keydown_fire = true;
			}
		}
		else
		{
			if (keydown_fire)
			{
				keydown_fire = false;
			}
		}

		//Set Proximity mines
		if (hge_->Input_GetKeyState(HGEK_K))
		{
			if (!keydown_mine)
			{
				CreateMine(ships_.at(0)->GetX(), ships_.at(0)->GetY(), ships_.at(0)->GetW(), ships_.at(0)->GetID(), ships_.at(0)->GetServerVelocityX(), ships_.at(0)->GetServerVelocityY(), ships_.at(0)->GetName());
				keydown_mine = true;
			}
		}
		else
		{
			if (keydown_mine)
			{
				keydown_mine = false;
			}
		}
	}
}

void Application::UpdateShips(float dt)
{
	//Update ships collision
	for (ShipList::iterator ship = ships_.begin(); ship != ships_.end(); ship++)
	{
			(*ship)->Update(dt);

			//collisions
			if ((*ship) == ships_.at(0))
				checkCollisions((*ship));
	}
}
void Application::UpdateProjectiles(float dt)
{
	//Update local projectile list
	for (vector<Projectile*>::iterator itr = local_projlist.begin(); itr != local_projlist.end(); itr++)
	{
		if ((*itr)->Update(ships_, dt))
		{
			CreateExplosion((*itr)->GetCollisionX(), (*itr)->GetCollisionY());

			//Log and damage
			if ((*itr)->GetSelfDamage() == true)
			{
				std::cout << "Self inflicted damage of: " << (*itr)->GetProjectileDmg() << std::endl;
				ships_.at(0)->SetHealth(ships_.at(0)->GetHealth() - (*itr)->GetProjectileDmg());
				std::cout << ships_.at(0)->GetHealth() << std::endl;

				//Set alive to false when HP is <= 0
				if (ships_.at(0)->GetHealth() <= 0 && ships_.at(0)->GetAlive())
				{
					ships_.at(0)->SetAlive(false);
					ships_.at(0)->UpdateRespawnLocation();
					++deaths;
				}
			}
			break;
		}
	}

	//Update network projectile list
	for (vector<Projectile*>::iterator itr = net_projlist.begin(); itr != net_projlist.end(); itr++)
	{
		if ((*itr)->Update(ships_, dt))
		{
			CreateExplosion((*itr)->GetCollisionX(), (*itr)->GetCollisionY());

			//Log and damage
			if ((*itr)->GetSelfDamage() == false)
			{
				std::cout << "Received " << (*itr)->GetProjectileDmg() << " from: " << (*itr)->GetOwnerName() << std::endl;
				ships_.at(0)->SetHealth(ships_.at(0)->GetHealth() - (*itr)->GetProjectileDmg());
				std::cout << ships_.at(0)->GetHealth() << std::endl;

				//Set alive to false when HP is <= 0
				if (ships_.at(0)->GetHealth() <= 0 && ships_.at(0)->GetAlive())
				{
					ships_.at(0)->SetAlive(false);
					ships_.at(0)->UpdateRespawnLocation();
					++deaths;
					sendKillCredits = true;
				}
			}
			//delete projectile upon collision
			delete *itr;
			net_projlist.erase(itr);
			break;
		}
	}
}
void Application::UpdateMines(float dt)
{
	//Update local mines list
	for (vector<ProximityMine*>::iterator itr = local_minelist.begin(); itr != local_minelist.end(); itr++)
	{
		if ((*itr)->Update(ships_, dt))
		{
			CreateExplosion((*itr)->GetCollisionX(), (*itr)->GetCollisionY());

			//Log and damage
			if ((*itr)->GetSelfDamage() == true)
			{
				std::cout << "Self inflicted damage of: " << (*itr)->GetProximityMineDmg() << std::endl;
				ships_.at(0)->SetHealth(ships_.at(0)->GetHealth() - (*itr)->GetProximityMineDmg());
				std::cout << ships_.at(0)->GetHealth() << std::endl;

				//Set alive to false when HP is <= 0
				if (ships_.at(0)->GetHealth() <= 0 && ships_.at(0)->GetAlive())
				{
					ships_.at(0)->SetAlive(false);
					ships_.at(0)->UpdateRespawnLocation();
					++deaths;
				}
			}
			break;
		}
	}

	//Update network mines list
	for (vector<ProximityMine*>::iterator itr = net_minelist.begin(); itr != net_minelist.end(); itr++)
	{
		if ((*itr)->Update(ships_, dt))
		{
			CreateExplosion((*itr)->GetCollisionX(), (*itr)->GetCollisionY());

			//Log and damage
			if ((*itr)->GetSelfDamage() == false)
			{
				std::cout << "Received " << (*itr)->GetProximityMineDmg() << " from: " << (*itr)->GetOwnerName() << std::endl;
				ships_.at(0)->SetHealth(ships_.at(0)->GetHealth() - (*itr)->GetProximityMineDmg());
				std::cout << ships_.at(0)->GetHealth() << std::endl;

				//Set alive to false when HP is <= 0
				if (ships_.at(0)->GetHealth() <= 0 && ships_.at(0)->GetAlive())
				{
					ships_.at(0)->SetAlive(false);
					ships_.at(0)->UpdateRespawnLocation();
					++deaths;
					sendKillCredits = true;
				}
			}
			//delete mine upon collision
			delete *itr;
			net_minelist.erase(itr);
			break;
		}
	}
}
void Application::UpdateExplosions(float dt)
{
	//Update explosion effects
	for (int i = 0; i < explosion_list.size(); ++i)
	{
		explosion_list.at(i)->Update(dt);
	}
}
void Application::UpdatePowerups(float dt)
{
	//Update powerups
	for (vector<Projectile_PowerUp*>::iterator itr = network_proj_powerup_list.begin(); itr != network_proj_powerup_list.end(); itr++)
	{
		//Adds damage with increment
		if ((*itr)->Update(ships_, dt))
		{
			//delete power up upon collision
			delete *itr;
			network_proj_powerup_list.erase(itr);
			break;
		}
	}
}

bool Application::UpdatePackets(float dt)
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
		case ID_CONNECTION_REQUEST_ACCEPTED:
			std::cout << "Connected to Server" << std::endl;
			break;

		case ID_NO_FREE_INCOMING_CONNECTIONS:
		case ID_CONNECTION_LOST:
		case ID_DISCONNECTION_NOTIFICATION:
			std::cout << "Lost Connection to Server" << std::endl;
			rakpeer_->DeallocatePacket(packet);
			return true;

		case ID_WELCOME:
		{
						   unsigned int shipcount, id;
						   float x_, y_;
						   int type_;
						   std::string temp;

						   bs.Read(id);
						   ships_.at(0)->setID(id);
						   bs.Read(shipcount);

						   for (unsigned int i = 0; i < shipcount; ++i)
						   {
							   bs.Read(id);
							   bs.Read(rs);
							   bs.Read(x_);
							   bs.Read(y_);
							   bs.Read(type_);
							   std::cout << "New Ship pos" << x_ << " " << y_ << std::endl;
							   Ship* ship = new Ship(type_, x_, y_);
							   ship->SetName(rs.C_String());
							   ship->setID(id);
							   ships_.push_back(ship);
						   }

						   SendInitialPosition();
		}
			break;

		case ID_NEWSHIP:
		{
						   unsigned int id;
						   bs.Read(id);

						   if (id == ships_.at(0)->GetID())
						   {
							   // if it is me
							   break;
						   }
						   else
						   {
							   float x_, y_;
							   int type_;
							   std::string temp;
							   
							   bs.Read(rs);
							   bs.Read(x_);
							   bs.Read(y_);
							   bs.Read(type_);
							   std::cout << "New Ship pos" << x_ << " " << y_ << std::endl;
							   Ship* ship = new Ship(type_, x_, y_);
							   ship->SetName(rs.C_String());
							   ship->setID(id);
							   ships_.push_back(ship);
						   }

		}
			break;
		//Close client
		case ID_REJECTSHIP:
			std::cout << "Maximum players detected, connection rejected" << std::endl;
			connection_rejected = true;
			return true;
			break;
		case ID_LOSTSHIP:
		{
							unsigned int shipid;
							bs.Read(shipid);
							for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr)
							{
								if ((*itr)->GetID() == shipid)
								{
									delete *itr;
									ships_.erase(itr);
									break;
								}
							}
		}
			break;

		case ID_INITIALPOS:
			break;

		case ID_MOVEMENT:
		{
							unsigned int shipid;
							float temp;
							float x, y, w;
							int health;
							bs.Read(shipid);
							for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr)
							{
								if ((*itr)->GetID() == shipid)
								{
									// this portion needs to be changed for it to work
#ifdef INTERPOLATEMOVEMENT
									bs.Read(x);
									bs.Read(y);
									bs.Read(w);
									(*itr)->SetServerLocation(x, y, w);
									
									bs.Read(health);
									(*itr)->SetHealth(health);

									if ((*itr)->GetHealth() <= 0)
									{
										(*itr)->SetAlive(false);
									}

									bs.Read(temp);
									(*itr)->SetServerVelocityX(temp);
									bs.Read(temp);
									(*itr)->SetServerVelocityY(temp);
									bs.Read(temp);
									(*itr)->SetAngularVelocity(temp);

									(*itr)->DoInterpolateUpdate();
#else
									bs.Read(x);
									bs.Read(y);
									bs.Read(w);
									(*itr)->setLocation(x, y, w);

									// Lab 7 Task 1 : Read Extrapolation Data velocity x, velocity y & angular velocity
									bs.Read(temp);
									(*itr)->SetVelocityX(temp);
									bs.Read(temp);
									(*itr)->SetVelocityY(temp);
									bs.Read(temp);
									(*itr)->SetAngularVelocity(temp);
#endif

									break;
								}
							}
		}
			break;

		case ID_COLLIDE:
		{
						   unsigned int shipid;
						   float x, y;
						   bs.Read(shipid);

						   if (shipid == ships_.at(0)->GetID())
						   {
							   std::cout << "collided with: " << Attacker << std::endl;
							   //Position
							   bs.Read(x);
							   bs.Read(y);
							   ships_.at(0)->SetX(x);
							   ships_.at(0)->SetY(y);
							   //Velocity
							   bs.Read(x);
							   bs.Read(y);
							   ships_.at(0)->SetVelocityX(x);
							   ships_.at(0)->SetVelocityY(y);

							   //Explosion
							   bs.Read(x);
							   bs.Read(y);
							   CreateExplosion(x, y);
#ifdef INTERPOLATEMOVEMENT
							   bs.Read(x);
							   bs.Read(y);
							   ships_.at(0)->SetServerVelocityX(x);
							   ships_.at(0)->SetServerVelocityY(y);
#endif				   
						   }
		}
			break;
		case ID_NEWPROJECTILE:
		{
							  float x, y, w;
							  int id, powerlevel;
							  string owner;

							  bs.Read(id);
							  bs.Read(rs);
							  bs.Read(x);
							  bs.Read(y);
							  bs.Read(w);
							  bs.Read(powerlevel);
							  
							  net_projlist.push_back(new Projectile(SPR_PROJECTILE, rs.C_String()));
							  net_projlist.back()->Init(x, y, w, id);
							  net_projlist.back()->SetProjectilePower(powerlevel);
		}
			break;
		case ID_UPDATEPROJECTILE:
		{
								 float x, y, w;
								 int id;
								 bool active;

								 bs.Read(id);

								 for (vector<Projectile*>::iterator itr = net_projlist.begin(); itr != net_projlist.end(); ++itr)
								 {
									 if ((*itr)->GetOwnerID() == id)
									 {
										 bs.Read(x);
										 bs.Read(y);
										 bs.Read(w);
										 (*itr)->UpdateLoc(x, y, w);
										 bs.Read(x);
										 (*itr)->SetVelocityX(x);
										 bs.Read(y);
										 (*itr)->SetVelocityY(y);
										 break;
									 }
								 }
		}
			break;
		case ID_NEWPROXIMITYMINE:
		{
								 float x, y, w, vel_x, vel_y;
								 int id;
								 string owner;

								 bs.Read(id);
								 bs.Read(rs);
								 bs.Read(x);
								 bs.Read(y);
								 bs.Read(w);
								 bs.Read(vel_x);
								 bs.Read(vel_y);

								 net_minelist.push_back(new ProximityMine(rs.C_String()));
								 net_minelist.back()->Init(x, y, w, vel_x, vel_y, id);
		}
			break;
		case ID_UPDATEPROXIMITYMINE:
		{
									float x, y, w;
									int id;
									bool active;

									bs.Read(id);
									for (vector<ProximityMine*>::iterator itr = net_minelist.begin(); itr != net_minelist.end(); ++itr)
									{
										if ((*itr)->GetOwnerID() == id)
										{
											bs.Read(x);
											bs.Read(y);
											bs.Read(w);
											(*itr)->UpdateLoc(x, y, w);
											bs.Read(x);
											(*itr)->SetVelocityX(x);
											bs.Read(y);
											(*itr)->SetVelocityY(y);
											break;
										}
									}
		}
			break;
		case ID_NEWPWRUP_PROJDMG:
		{
									float x, y;
									bs.Read(x);
									bs.Read(y);
									network_proj_powerup_list.push_back(new Projectile_PowerUp(SPR_PROJ_PWRUP));
									network_proj_powerup_list.back()->Init(x, y);
		}
			break;
		case ID_KILL_CREDIT:
			++kills;
			break;
		default:
			std::cout << "Unhandled Message Identifier: " << (int)msgid << std::endl;
		}
		rakpeer_->DeallocatePacket(packet);
	}

	if (RakNet::GetTime() - timer_ > 250)
	{
		timer_ = RakNet::GetTime();
		RakNet::BitStream bs2;
		unsigned char msgid = ID_MOVEMENT;
		bs2.Write(msgid);

#ifdef INTERPOLATEMOVEMENT
		bs2.Write(ships_.at(0)->GetID());
		bs2.Write(ships_.at(0)->GetServerX());
		bs2.Write(ships_.at(0)->GetServerY());
		bs2.Write(ships_.at(0)->GetServerW());
		bs2.Write(ships_.at(0)->GetHealth());
		bs2.Write(ships_.at(0)->GetServerVelocityX());
		bs2.Write(ships_.at(0)->GetServerVelocityY());
		bs2.Write(ships_.at(0)->GetAngularVelocity());
#else
		bs2.Write(ships_.at(0)->GetID());
		bs2.Write(ships_.at(0)->GetX());
		bs2.Write(ships_.at(0)->GetY());
		bs2.Write(ships_.at(0)->GetW());
		// Lab 7 Task 1 : Add Extrapolation Data velocity x, velocity y & angular velocity
		bs2.Write(ships_.at(0)->GetVelocityX());
		bs2.Write(ships_.at(0)->GetVelocityY());
		bs2.Write(ships_.at(0)->GetAngularVelocity());
#endif

		rakpeer_->Send(&bs2, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

		////Sending of updated local projectiles to network
		//for (vector<Projectile*>::iterator itr = local_projlist.begin(); itr != local_projlist.end(); itr++)
		//{
		//	if ((*itr)->GetActive())
		//	{
		//		RakNet::BitStream bs_projectile;
		//		unsigned char msgid2 = ID_UPDATEPROJECTILE;

		//		bs_projectile.Write(msgid2);
		//		bs_projectile.Write((*itr)->GetOwnerID());
		//		bs_projectile.Write((*itr)->GetX());
		//		bs_projectile.Write((*itr)->GetY());
		//		bs_projectile.Write((*itr)->GetW());
		//		bs_projectile.Write((*itr)->GetVelocityX());
		//		bs_projectile.Write((*itr)->GetVelocityY());

		//		rakpeer_->Send(&bs_projectile, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
		//	}
		//}
		
		////Sending of updated local mines to network
		//for (vector<ProximityMine*>::iterator itr = local_minelist.begin(); itr != local_minelist.end(); itr++)
		//{
		//	if ((*itr)->GetActive())
		//	{
		//		RakNet::BitStream bs_mine;
		//		unsigned char msgid2 = ID_UPDATEPROXIMITYMINE;

		//		bs_mine.Write(msgid2);
		//		bs_mine.Write((*itr)->GetOwnerID());
		//		bs_mine.Write((*itr)->GetX());
		//		bs_mine.Write((*itr)->GetY());
		//		bs_mine.Write((*itr)->GetW());
		//		bs_mine.Write((*itr)->GetVelocityX());
		//		bs_mine.Write((*itr)->GetVelocityY());

		//		rakpeer_->Send(&bs_mine, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
		//	}
		//}
	}

	if (sendKillCredits)
	{
		sendKillCredits = false;

		RakNet::BitStream bs;
		unsigned char msgid = ID_KILL_CREDIT;
		bs.Write(msgid);

		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	}
	return false;
}

/**
* Render Cycle
*
* Clear the screen and render all the ships
*/
void Application::Render()
{
	hge_->Gfx_BeginScene();
	hge_->Gfx_Clear(0);

	background_Left->Render(Left_X, 0);
	background_Right->Render(Right_X, 0);
	if (nameProcessed)
	{
		ShipList::iterator itr;
		for (itr = ships_.begin(); itr != ships_.end(); itr++)
		{
			(*itr)->Render();
		}

		for (vector<explosion*>::iterator itr = explosion_list.begin(); itr != explosion_list.end(); itr++)
		{
			if ((*itr)->GetActive())
			{
				(*itr)->Render();
			}
		}

		//Render Local list projectiles
		for (vector<Projectile*>::iterator itr = local_projlist.begin(); itr != local_projlist.end(); itr++)
		{
			if ((*itr)->GetActive())
			{
				(*itr)->Render();
			}
		}

		//Render Network list projectiles
		for (vector<Projectile*>::iterator itr = net_projlist.begin(); itr != net_projlist.end(); itr++)
		{
			if ((*itr)->GetActive())
			{
				(*itr)->Render();
			}
		}

		//Render Local list mines
		for (vector<ProximityMine*>::iterator itr = local_minelist.begin(); itr != local_minelist.end(); itr++)
		{
			if ((*itr)->GetActive())
			{
				(*itr)->Render();
			}
		}

		//Render Network list mines
		for (vector<ProximityMine*>::iterator itr = net_minelist.begin(); itr != net_minelist.end(); itr++)
		{
			if ((*itr)->GetActive())
			{
				(*itr)->Render();
			}
		}

		//Render Powerups
		for (vector<Projectile_PowerUp*>::iterator itr = network_proj_powerup_list.begin(); itr != network_proj_powerup_list.end(); itr++)
		{
			if ((*itr)->GetActive())
			{
				(*itr)->Render();
			}
		}
	}

	//Cursor
	//if (showCursor)
	//{
	//	float x, y;
	//	hge_->Input_GetMousePos(&x, &y);
	//	cursor_->Render(x, y);
	//}
	RenderUI();
	
	hge_->Gfx_EndScene();
}

void Application::RenderUI(void)
{
	if (!nameProcessed)
	{
		title_->Render(250, 10);
		if (nameTooShort)
		{
			font_->printf(200, 450, HGETEXT_LEFT, "%s", "Error, name is too short");
		}
		//Entering name here
		std::ostringstream HighScoreName;
		for (unsigned i = 0; i < Player_NameProcessor.GetNameChar().size(); ++i)
		{
			HighScoreName << Player_NameProcessor.GetNameChar()[i];
		}
		font_->printf(120, 350, HGETEXT_LEFT, "%s", "Please enter a name between 4 to 14 characters");
		font_->printf(200, 400, HGETEXT_LEFT, "%s%s", "Player name: ", HighScoreName.str().c_str());
		font_->printf(200, 500, HGETEXT_LEFT, "%s", "Press enter to confirm, esc to quit");
	}
	else
	{
		//Health
		font_->printf(20, 550, HGETEXT_LEFT, "%s%d", "Player Health: ", ships_.at(0)->GetHealth());

		//Projectile power
		switch (ships_.at(0)->GetPower())
		{
		case 0:
			pwrlvl1_->Render(450, 530);
			break;
		case 1:
			pwrlvl2_->Render(460, 530);
			break;
		case 2:
			pwrlvl3_->Render(460, 530);
			break;
		case 3:
			pwrlvlmax_->Render(460, 530);
			break;
		default:
			break;
		}

		font_->printf(300, 550, HGETEXT_LEFT, "%s", "Power Level:");

		int UsableMines = 0;

		//Mine state
		for (vector<ProximityMine*>::iterator itr = local_minelist.begin(); itr != local_minelist.end(); ++itr)
		{
			if ((*itr)->GetActive() == false)
			{
				++UsableMines;
			}
		}
		if (UsableMines > 0)
		{
			mineready_->Render(620, 530);
		}
		else
		{
			mineunready_->Render(620, 530);
		}

		font_->printf(550, 550, HGETEXT_LEFT, "%s        x%d", "Mines: ", UsableMines);

		//KD
		font_->printf(20, 500, HGETEXT_LEFT, "%s%d", "Kills: ", kills);
		font_->printf(150, 500, HGETEXT_LEFT, "%s%d", "Deaths: ", deaths);
	}
}

/**
* Main game loop
*
* Processes user input events
* Supposed to process network events
* Renders the ships
*
* This is a static function that is called by the graphics
* engine every frame, hence the need to loop through the
* global namespace to find itself.
*/
bool Application::Loop()
{
	Global::application->Render();
	return Global::application->Update();
}

/**
* Shuts down the graphics and network system
*/

void Application::Shutdown()
{
	hge_->System_Shutdown();
	hge_->Release();
	for (int i = 0; i < explosion_list.size(); ++i)
	{
		delete explosion_list.at(i);
		explosion_list.at(i) = NULL;
	}
	//for (int i = 0; i < local_projlist.size(); ++i)
	//{
	//	delete local_projlist.at(i);
	//	local_projlist.at(i) = NULL;
	//}
	//for (int i = 0; i < net_projlist.size(); ++i)
	//{
	//	delete net_projlist.at(i);
	//	net_projlist.at(i) = NULL;
	//}
	//for (int i = 0; i < local_minelist.size(); ++i)
	//{
	//	delete local_minelist.at(i);
	//	local_minelist.at(i) = NULL;
	//}
	//for (int i = 0; i < net_minelist.size(); ++i)
	//{
	//	delete net_minelist.at(i);
	//	net_minelist.at(i) = NULL;
	//}
}

/**
* Kick starts the everything, called from main.
*/
void Application::Start()
{
	if (Init())
	{
		hge_->System_Start();
	}
}

void Application::ProcessName(void)
{
	//Name input for high scores
	if (Player_NameProcessor.GetNameCharCount() < MAXNAMELENGTH)
	{
		if (hge_->Input_GetKeyState(VK_SHIFT))
			Player_NameProcessor.SetCapitalLetter(true);
		else
			Player_NameProcessor.SetCapitalLetter(false);

		//Alpha key states
		static bool b_AKeyState = false;
		static bool b_BKeyState = false;
		static bool b_CKeyState = false;
		static bool b_DKeyState = false;
		static bool b_EKeyState = false;
		static bool b_FKeyState = false;
		static bool b_GKeyState = false;
		static bool b_HKeyState = false;
		static bool b_IKeyState = false;
		static bool b_JKeyState = false;
		static bool b_KKeyState = false;
		static bool b_LKeyState = false;
		static bool b_MKeyState = false;
		static bool b_NKeyState = false;
		static bool b_OKeyState = false;
		static bool b_PKeyState = false;
		static bool b_QKeyState = false;
		static bool b_RKeyState = false;
		static bool b_SKeyState = false;
		static bool b_TKeyState = false;
		static bool b_UKeyState = false;
		static bool b_VKeyState = false;
		static bool b_WKeyState = false;
		static bool b_XKeyState = false;
		static bool b_YKeyState = false;
		static bool b_ZKeyState = false;

		//A Key
		if (hge_->Input_GetKeyState('A') && !b_AKeyState)
		{
			b_AKeyState = true;
		}
		if (!hge_->Input_GetKeyState('A') && b_AKeyState)
		{
			b_AKeyState = false;
			Player_NameProcessor.SetCharToAdd('a');
		}

		//B Key
		if (hge_->Input_GetKeyState('B') && !b_BKeyState)
		{
			b_BKeyState = true;
		}
		if (!hge_->Input_GetKeyState('B') && b_BKeyState)
		{
			b_BKeyState = false;
			Player_NameProcessor.SetCharToAdd('b');
		}

		//C Key
		if (hge_->Input_GetKeyState('C') && !b_CKeyState)
		{
			b_CKeyState = true;
		}
		if (!hge_->Input_GetKeyState('C') && b_CKeyState)
		{
			b_CKeyState = false;
			Player_NameProcessor.SetCharToAdd('c');
		}

		//D Key
		if (hge_->Input_GetKeyState('D') && !b_DKeyState)
		{
			b_DKeyState = true;
		}
		if (!hge_->Input_GetKeyState('D') && b_DKeyState)
		{
			b_DKeyState = false;
			Player_NameProcessor.SetCharToAdd('d');
		}

		//E Key
		if (hge_->Input_GetKeyState('E') && !b_EKeyState)
		{
			b_EKeyState = true;
		}
		if (!hge_->Input_GetKeyState('E') && b_EKeyState)
		{
			b_EKeyState = false;
			Player_NameProcessor.SetCharToAdd('e');
		}

		//F Key
		if (hge_->Input_GetKeyState('F') && !b_FKeyState)
		{
			b_FKeyState = true;
		}
		if (!hge_->Input_GetKeyState('F') && b_FKeyState)
		{
			b_FKeyState = false;
			Player_NameProcessor.SetCharToAdd('f');
		}

		//G Key
		if (hge_->Input_GetKeyState('G') && !b_GKeyState)
		{
			b_GKeyState = true;
		}
		if (!hge_->Input_GetKeyState('G') && b_GKeyState)
		{
			b_GKeyState = false;
			Player_NameProcessor.SetCharToAdd('g');
		}

		//H Key
		if (hge_->Input_GetKeyState('H') && !b_HKeyState)
		{
			b_HKeyState = true;
		}
		if (!hge_->Input_GetKeyState('H') && b_HKeyState)
		{
			b_HKeyState = false;
			Player_NameProcessor.SetCharToAdd('h');
		}

		//I Key
		if (hge_->Input_GetKeyState('I') && !b_IKeyState)
		{
			b_IKeyState = true;
		}
		if (!hge_->Input_GetKeyState('I') && b_IKeyState)
		{
			b_IKeyState = false;
			Player_NameProcessor.SetCharToAdd('i');
		}

		//J Key
		if (hge_->Input_GetKeyState('J') && !b_JKeyState)
		{
			b_JKeyState = true;
		}
		if (!hge_->Input_GetKeyState('J') && b_JKeyState)
		{
			b_JKeyState = false;
			Player_NameProcessor.SetCharToAdd('j');
		}

		//K Key
		if (hge_->Input_GetKeyState('K') && !b_KKeyState)
		{
			b_KKeyState = true;
		}
		if (!hge_->Input_GetKeyState('K') && b_KKeyState)
		{
			b_KKeyState = false;
			Player_NameProcessor.SetCharToAdd('k');
		}

		//L Key
		if (hge_->Input_GetKeyState('L') && !b_LKeyState)
		{
			b_LKeyState = true;
		}
		if (!hge_->Input_GetKeyState('L') && b_LKeyState)
		{
			b_LKeyState = false;
			Player_NameProcessor.SetCharToAdd('l');
		}

		//M Key
		if (hge_->Input_GetKeyState('M') && !b_MKeyState)
		{
			b_MKeyState = true;
		}
		if (!hge_->Input_GetKeyState('M') && b_MKeyState)
		{
			b_MKeyState = false;
			Player_NameProcessor.SetCharToAdd('m');
		}

		//N Key
		if (hge_->Input_GetKeyState('N') && !b_NKeyState)
		{
			b_NKeyState = true;
		}
		if (!hge_->Input_GetKeyState('N') && b_NKeyState)
		{
			b_NKeyState = false;
			Player_NameProcessor.SetCharToAdd('n');
		}

		//O Key
		if (hge_->Input_GetKeyState('O') && !b_OKeyState)
		{
			b_OKeyState = true;
		}
		if (!hge_->Input_GetKeyState('O') && b_OKeyState)
		{
			b_OKeyState = false;
			Player_NameProcessor.SetCharToAdd('o');
		}

		//P Key
		if (hge_->Input_GetKeyState('P') && !b_PKeyState)
		{
			b_PKeyState = true;
		}
		if (!hge_->Input_GetKeyState('P') && b_PKeyState)
		{
			b_PKeyState = false;
			Player_NameProcessor.SetCharToAdd('p');
		}

		//Q Key
		if (hge_->Input_GetKeyState('Q') && !b_QKeyState)
		{
			b_QKeyState = true;
		}
		if (!hge_->Input_GetKeyState('Q') && b_QKeyState)
		{
			b_QKeyState = false;
			Player_NameProcessor.SetCharToAdd('q');
		}

		//R Key
		if (hge_->Input_GetKeyState('R') && !b_RKeyState)
		{
			b_RKeyState = true;
		}
		if (!hge_->Input_GetKeyState('R') && b_RKeyState)
		{
			b_RKeyState = false;
			Player_NameProcessor.SetCharToAdd('r');
		}

		//S Key
		if (hge_->Input_GetKeyState('S') && !b_SKeyState)
		{
			b_SKeyState = true;
		}
		if (!hge_->Input_GetKeyState('S') && b_SKeyState)
		{
			b_SKeyState = false;
			Player_NameProcessor.SetCharToAdd('s');
		}

		//T Key
		if (hge_->Input_GetKeyState('T') && !b_TKeyState)
		{
			b_TKeyState = true;
		}
		if (!hge_->Input_GetKeyState('T') && b_TKeyState)
		{
			b_TKeyState = false;
			Player_NameProcessor.SetCharToAdd('t');
		}

		//U Key
		if (hge_->Input_GetKeyState('U') && !b_UKeyState)
		{
			b_UKeyState = true;
		}
		if (!hge_->Input_GetKeyState('U') && b_UKeyState)
		{
			b_UKeyState = false;
			Player_NameProcessor.SetCharToAdd('u');
		}

		//V Key
		if (hge_->Input_GetKeyState('V') && !b_VKeyState)
		{
			b_VKeyState = true;
		}
		if (!hge_->Input_GetKeyState('V') && b_VKeyState)
		{
			b_VKeyState = false;
			Player_NameProcessor.SetCharToAdd('v');
		}

		//W Key
		if (hge_->Input_GetKeyState('W') && !b_WKeyState)
		{
			b_WKeyState = true;
		}
		if (!hge_->Input_GetKeyState('W') && b_WKeyState)
		{
			b_WKeyState = false;
			Player_NameProcessor.SetCharToAdd('w');
		}

		//X Key
		if (hge_->Input_GetKeyState('X') && !b_XKeyState)
		{
			b_XKeyState = true;
		}
		if (!hge_->Input_GetKeyState('X') && b_XKeyState)
		{
			b_XKeyState = false;
			Player_NameProcessor.SetCharToAdd('x');
		}

		//Y Key
		if (hge_->Input_GetKeyState('Y') && !b_YKeyState)
		{
			b_YKeyState = true;
		}
		if (!hge_->Input_GetKeyState('Y') && b_YKeyState)
		{
			b_YKeyState = false;
			Player_NameProcessor.SetCharToAdd('y');
		}

		//Z Key
		if (hge_->Input_GetKeyState('Z') && !b_ZKeyState)
		{
			b_ZKeyState = true;
		}
		if (!hge_->Input_GetKeyState('Z') && b_ZKeyState)
		{
			b_ZKeyState = false;
			Player_NameProcessor.SetCharToAdd('z');
		}

		//Only adds non empty character
		if (Player_NameProcessor.GetCharToAdd() != ' ')
		{
			if (Player_NameProcessor.GetCapitalLetter())
			{
				Player_NameProcessor.SetCharToAdd(Player_NameProcessor.GetCharToAdd() - 32);
			}
			Player_NameProcessor.AddNameCharCount();
			Player_NameProcessor.SetNameInput();
		}
	}

	//Backspace
	if (hge_->Input_GetKeyState(VK_BACK) && Player_NameProcessor.GetNameCharCount() > 0)
	{
		Player_NameProcessor.SetCharToRemove();
		Player_NameProcessor.SubtractNameCharCount();
	}

	//Confirm name
	if (hge_->Input_GetKeyState(VK_RETURN))
	{
		//If 4 characters and above
		if (Player_NameProcessor.GetNameCharCount() > 3)
		{
			std::stringstream name;
			for (unsigned i = 0; i < Player_NameProcessor.GetNameChar().size(); ++i)
			{
				name << Player_NameProcessor.GetNameChar().at(i);
			}
			Player_NameProcessor.SetName(name.str());
			nameProcessed = true;
		}
		else
		{
			nameTooShort = true;
		}
	}
}

bool Application::SendInitialPosition()
{
	RakNet::BitStream bs;
	unsigned char msgid = ID_INITIALPOS;
	//rs = Player_NameProcessor.GetName().c_str();
	rs.Set(Player_NameProcessor.GetName().c_str());
	ships_.at(0)->SetName(rs);
	bs.Write(msgid);
	bs.Write(rs);
	bs.Write(ships_.at(0)->GetX());
	bs.Write(ships_.at(0)->GetY());
	bs.Write(ships_.at(0)->GetType());

	std::cout << "Sending pos" << ships_.at(0)->GetX() << " " << ships_.at(0)->GetY() << std::endl;

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	return true;
}

bool Application::checkCollisions(Ship* ship)
{
	for (std::vector<Ship*>::iterator thisship = ships_.begin();
		thisship != ships_.end(); thisship++)
	{
		if ((*thisship) == ship) continue;	//skip if it is the same ship

		if (ship->HasCollided((*thisship)))
		{
			if ((*thisship)->CanCollide(RakNet::GetTime()) && ship->CanCollide(RakNet::GetTime()))
			{
				Attacker = (*thisship)->GetName();

				collision_X = ((*thisship)->GetX() + ship->GetX()) * 0.5f;
				collision_Y = ((*thisship)->GetY() + ship->GetY()) * 0.5f;

#ifdef INTERPOLATEMOVEMENT
				if (GetAbsoluteMag(ship->GetVelocityY()) > GetAbsoluteMag((*thisship)->GetVelocityY()))
				{
					// this transfers vel to thisship
					(*thisship)->SetVelocityY((*thisship)->GetVelocityY() + ship->GetVelocityY() / 3);
					ship->SetVelocityY(-ship->GetVelocityY());

					(*thisship)->SetServerVelocityY((*thisship)->GetServerVelocityY() + ship->GetServerVelocityY() / 3);
					ship->SetServerVelocityY(-ship->GetServerVelocityY());
				}
				else
				{
					ship->SetVelocityY(ship->GetVelocityY() + (*thisship)->GetVelocityY() / 3);
					(*thisship)->SetVelocityY(-(*thisship)->GetVelocityY() / 2);

					ship->SetServerVelocityY(ship->GetServerVelocityY() + (*thisship)->GetServerVelocityY() / 3);
					(*thisship)->SetServerVelocityY(-(*thisship)->GetServerVelocityY() / 2);
				}

				if (GetAbsoluteMag(ship->GetVelocityX()) > GetAbsoluteMag((*thisship)->GetVelocityX()))
				{
					// this transfers vel to thisship
					(*thisship)->SetVelocityX((*thisship)->GetVelocityX() + ship->GetVelocityX() / 3);
					ship->SetVelocityX(-ship->GetVelocityX());

					(*thisship)->SetServerVelocityX((*thisship)->GetServerVelocityX() + ship->GetServerVelocityX() / 3);
					ship->SetServerVelocityX(-ship->GetServerVelocityX());
				}
				else
				{
					// ship transfers vel to asteroid
					ship->SetVelocityX(ship->GetVelocityX() + (*thisship)->GetVelocityX() / 3);
					(*thisship)->SetVelocityX(-(*thisship)->GetVelocityX() / 2);

					ship->SetServerVelocityX(ship->GetServerVelocityX() + (*thisship)->GetServerVelocityX() / 3);
					(*thisship)->SetServerVelocityX(-(*thisship)->GetServerVelocityX() / 2);
				}

				ship->SetPreviousLocation();
#else
				if (GetAbsoluteMag(ship->GetVelocityY()) > GetAbsoluteMag((*thisship)->GetVelocityY()))
				{
					// this transfers vel to thisship
					(*thisship)->SetVelocityY((*thisship)->GetVelocityY() + ship->GetVelocityY() / 3);
					ship->SetVelocityY(-ship->GetVelocityY());
				}
				else
				{
					ship->SetVelocityY(ship->GetVelocityY() + (*thisship)->GetVelocityY() / 3);
					(*thisship)->SetVelocityY(-(*thisship)->GetVelocityY() / 2);
				}

				if( GetAbsoluteMag( ship->GetVelocityX() ) > GetAbsoluteMag( (*thisship)->GetVelocityX() ) )
				{
					// this transfers vel to thisship
					(*thisship)->SetVelocityX( (*thisship)->GetVelocityX() + ship->GetVelocityX()/3 );
					ship->SetVelocityX( - ship->GetVelocityX() );
				}
				else
				{
					// ship transfers vel to asteroid
					ship->SetVelocityX( ship->GetVelocityX() + (*thisship)->GetVelocityX()/3 ); 
					(*thisship)->SetVelocityX( -(*thisship)->GetVelocityX()/2 );
				}


				//				ship->SetVelocityY( -ship->GetVelocityY() );
				//				ship->SetVelocityX( -ship->GetVelocityX() );

				ship->SetPreviousLocation();
#endif
				SendCollision((*thisship));

				return true;
			}

		}

	}

	return false;
}

void Application::SendCollision(Ship* ship)
{
	RakNet::BitStream bs;
	unsigned char msgid = ID_COLLIDE;
	bs.Write(msgid);
	bs.Write(ship->GetID());
	bs.Write(ship->GetX());
	bs.Write(ship->GetY());
	bs.Write(ship->GetVelocityX());
	bs.Write(ship->GetVelocityY());
	//Ship collision boom
	bs.Write(collision_X);
	bs.Write(collision_Y);
#ifdef INTERPOLATEMOVEMENT
	bs.Write(ship->GetServerVelocityX());
	bs.Write(ship->GetServerVelocityY());
#endif

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Application::CreateProjectile(float x, float y, float w, int id, int powerlevel, string name)
{
	RakNet::BitStream bs;
	unsigned char msgid;
	bool active = false;

	//Loop through list to get inactive projectile
	for (vector<Projectile*>::iterator itr = local_projlist.begin(); itr != local_projlist.end(); ++itr)
	{
		if ((*itr)->GetActive() == false)
		{
			(*itr)->Init(x, y, w, id);
			active = true;
			break;
		}
	}
	//Did not get any inactive projectile
	if (!active)
	{
		//Create new projectiles in 10s to use
		for (int i = 0; i < 10; ++i)
		{
			local_projlist.push_back(new Projectile(SPR_PROJECTILE, ShipName));
		}
		local_projlist.back()->Init(x, y, w, id);
	}

	//Add new projectile to network
	bs.Reset();

	msgid = ID_NEWPROJECTILE; 
	bs.Write(msgid); 
	bs.Write(id);
	bs.Write(rs);
	bs.Write(x); 
	bs.Write(y); 
	bs.Write(w);
	bs.Write(powerlevel);

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Application::CreateMine(float x, float y, float w, int id, float vel_X, float vel_Y, string name)
{
	RakNet::BitStream bs;
	unsigned char msgid;
	bool created = false;

	//Loop through list to get inactive mine
	for (vector<ProximityMine*>::iterator itr = local_minelist.begin(); itr != local_minelist.end(); ++itr)
	{
		if ((*itr)->GetActive() == false)
		{
			(*itr)->Init(x, y, w, vel_X, vel_Y, id);
			created = true;
			break;
		}
	}
	//Add new mine to network
	if (created)
	{
		bs.Reset();

		msgid = ID_NEWPROXIMITYMINE;
		bs.Write(msgid);
		bs.Write(id);
		bs.Write(rs);
		bs.Write(x);
		bs.Write(y);
		bs.Write(w);
		bs.Write(vel_X);
		bs.Write(vel_Y);

		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	}
}

void Application::CreateExplosion(float pos_X, float pos_Y)
{
	for (int i = 0; i < explosion_list.size(); ++i)
	{
		//Grab and empty explosion object
		if (explosion_list.at(i)->GetActive() == false)
		{
			explosion_list.at(i)->SetActive(true);
			explosion_list.at(i)->SetPosition(pos_X, pos_Y);
			break;
		}
	}
}
