#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "ship.h"
#include "missile.h"
#include "explosion.h"

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
	
	// Lab 13 Task 1 : add variables for local missle
	Missile* mymissile;
	bool have_missile;
	bool keydown_enter;

	//Sprites
	std::auto_ptr<hgeSprite> background_;
	std::auto_ptr<hgeSprite> explosion_;
	HTEXTURE bg_tex_, explosion_tex_;

	// Lab 13 Task 8 : add variables to handle networked missiles
	typedef std::vector<Missile*> MissileList;
	MissileList missiles_;
	
	//Explosion effect
	vector <explosion*> explosion_list;
	explosion* explosion_efx;
	float collision_X, collision_Y;
	hgeAnimation* SA;
	
	//Tracking
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

	// Lab 13
	void CreateMissile(float x, float y, float w, int id, string name);
	bool RemoveMissile( float x, float y, float w, int id );
	void CreateExplosion(float pos_X, float pos_Y);

	void SendCollision( Ship* ship );
public:
	Application();
	~Application() throw();

	void Start();
	bool Update();
	bool UpdateKeypress();
	void UpdateLocal(float dt);
	bool UpdateNetwork(float dt);
	void Render();
};

#endif