#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "ship.h"
#include "projectile.h"
#include "explosion.h"
#include "proximitymine.h"
#include "projectile_powerup.h"

#include <vector>
#include <hge.h>
#include <hgeSprite.h>
#include <string>
#include <RakString.h>

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

	bool keydown_fire;
	bool keydown_mine;

	//Sprites
	std::auto_ptr<hgeSprite> background_Left, background_Right;
	std::auto_ptr<hgeSprite> cursor_;
	std::auto_ptr<hgeSprite> explosion_;
	HTEXTURE bg_tex_, explosion_tex_, cursor_tex;
	bool showCursor;
	float Left_X;
	float Right_X;

	//Projectiles
	vector<Projectile*> local_projlist;
	vector<Projectile*> net_projlist;

	void CreateProjectile(float x, float y, float w, int id, int powerlevel, string name);

	//Proximity Mine
	vector<ProximityMine*> local_minelist;
	vector<ProximityMine*> net_minelist;

	void CreateMine(float x, float y, float w, int id, float vel_X, float vel_Y, string name);

	//Explosion effect
	vector <explosion*> explosion_list;
	float collision_X, collision_Y;

	void CreateExplosion(float pos_X, float pos_Y);

	//Power up
	vector <Projectile_PowerUp*> network_proj_powerup_list;
	void CreatePowerUp(float pos_X, float pos_Y);

	//Scoring
	int kills, deaths;
	bool sendKillCredits;

	//UI
	HTEXTURE tex_pwrlvl1, tex_pwrlvl2, tex_pwrlvl3, tex_pwrlvlmax, tex_mine_unready, tex_mine_ready;
	std::auto_ptr<hgeSprite> pwrlvl1_;
	std::auto_ptr<hgeSprite> pwrlvl2_;
	std::auto_ptr<hgeSprite> pwrlvl3_;
	std::auto_ptr<hgeSprite> pwrlvlmax_;
	std::auto_ptr<hgeSprite> mineready_;
	std::auto_ptr<hgeSprite> mineunready_;
	std::auto_ptr<hgeFont> font_;
	std::string playerHealth, playerKills, playerDeaths;


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

	void SendCollision(Ship* ship);
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
	void UpdatePowerups(float dt);
	bool UpdatePackets(float dt);
	void Render();
	void RenderUI(void);
};

#endif