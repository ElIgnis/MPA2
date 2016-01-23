#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "ship.h"
#include "projectile.h"
#include "explosion.h"
#include "proximitymine.h"

#include <vector>
#include <hge.h>
#include <hgeSprite.h>
#include <string>
#include <RakString.h>
#include <hgeanim.h>

class hgeSprite;
class hgeFont;
class HGE;
class RakPeerInterface;

using std::vector;
using std::string;

#define NEW_MINE_DELAY_TIMER 5.f

//! The default angular velocity of the ship when it is in motion
static const float DEFAULT_ANGULAR_VELOCITY = 3.0f; 
//! The default acceleration of the ship when powered
static const float DEFAULT_ACCELERATION = 50.0f;

/**
* The application class is the main body of the program. It will
* create an instance of the graphics engine and execute the
* Update/Render cycle.
*
*/

class Application
{
	HGE* hge_; //!< Instance of the internal graphics engine
	typedef std::vector<Ship*> ShipList;  //!< A list of ships
	ShipList ships_; //!< List of all the ships in the universe
	RakPeerInterface* rakpeer_;
	unsigned int timer_;
		
	bool keydown_fire;
	bool keydown_mine;

	//Sprites
	std::auto_ptr<hgeSprite> background_;
	std::auto_ptr<hgeSprite> explosion_;
	HTEXTURE bg_tex_, explosion_tex_;

	//Projectiles
	vector<Projectile*>  local_projlist;
	vector<Projectile*> net_projlist;
	
	void CreateProjectile(float x, float y, float w, int id, string name);

	//Proximity Mine
	vector<ProximityMine*> local_minelist;
	vector<ProximityMine*> net_minelist;
	bool NewMine;
	float NewMineTimer;

	void CreateMine(float x, float y, float w, int id, float vel_X, float vel_Y, string name);

	//Explosion effect
	vector <explosion*> explosion_list;
	explosion* explosion_efx;
	float collision_X, collision_Y;
	hgeAnimation* SA_explosion;

	void CreateExplosion(float pos_X, float pos_Y);
	
	//Player Tracking
	string ShipName;
	string Attacker;
	const char* ShipName_Const;
	RakNet::RakString rs;

	bool Init();
	static bool Loop();
	void Shutdown();
	bool checkCollisions(Ship* ship);
	void ProcessWelcomePackage();
	bool SendInitialPosition();

	void SendCollision( Ship* ship );
public:
	Application();
	~Application() throw();

	void Start();
	bool Update();
	bool UpdateKeypress();
	void UpdateShips(float dt);
	void UpdateProjectiles(float dt);
	void UpdateMines(float dt);
	void UpdateExplosions(float dt);
	bool UpdatePackets(float dt);
	void Render();
};

#endif