#ifndef _SHIP_H_
#define _SHIP_H_

#include <hge.h>
#include <hgerect.h>
#include <memory>
#include <string>
#include <hgeanim.h>
#include <iostream>

class hgeSprite;
class hgeFont;

using std::string;

#define RESPAWN_DELAY 3.f
#define SPR_RESPAWN "Images/respawn.png"
#define RESPAWN_IMAGE_SIZE 256
#define INTERPOLATEMOVEMENT 

/**
* The Ship class represents a single spaceship floating in space. It obeys
* 2D physics in terms of displacement, velocity and acceleration, as well
* as angular position and displacement. The size of the current art is
* 128*128 pixels
*/

class Ship
{
	HTEXTURE tex_, respawn_tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the ship
	std::auto_ptr<hgeFont> font_;
	hgeRect collidebox;
	hgeAnimation* SA_respawn;

	std::string shipName_;
	float x_; //!< The x-coordinate of the ship
	float y_; //!< The y-coordinate of the ship
	float w_; //!< The angular position of the ship
	float velocity_x_; //!< The resolved velocity of the ship along the x-axis
	float velocity_y_; //!< The resolved velocity of the ship along the y-axis

	float oldx, oldy;	// for reset back to previous location if collision detected
	int health;
	bool alive;
	bool updateSprite;
	bool updatePos;
	int powerLevel;
	float respawnTimer;

	// Lab Task 2 : add for interpolation
#ifdef INTERPOLATEMOVEMENT
	float server_x_;
	float server_y_;
	float server_w_;
	float client_x_;
	float client_y_;
	float client_w_;
	float server_velx_;
	float server_vely_;
	float ratio_;
#endif

	unsigned int id;
	int type_;
	float angular_velocity;
	unsigned int collidetimer;
public:

	Ship(int type, float locx_, float locy_);
	~Ship();
	void Update(float timedelta);
	void Render();
	void Accelerate(float acceleration, float timedelta);
	void SetName(const char * text);
	string GetName(void);
	void SetHealth(int newHealth);
	int GetHealth(void);
	bool GetAlive(void);
	void SetAlive(bool newAlive);
	bool IncreasePower(void);
	int GetPower(void);
	bool GetRespawn(void);
	
	hgeRect* GetBoundingBox();
	bool HasCollided( Ship *ship );

	float GetVelocityX() { return velocity_x_; }
	float GetVelocityY() { return velocity_y_; }

	void SetVelocityX( float velocity ) { velocity_x_ = velocity; }
	void SetVelocityY( float velocity ) { velocity_y_ = velocity; }

	float GetAngularVelocity() { return angular_velocity; }

	void SetAngularVelocity( float av ) { angular_velocity = av; }

	void SetPreviousLocation()
	{
		x_ = oldx;
		y_ = oldy;
	}

	unsigned int GetID() { return id; }

	void setID(unsigned int newid ) { id = newid; }

	void setLocation( float x, float y, float w ) 
	{ 
		x_ = x; 
		y_ = y; 
		w_ = w; 
	}

	void SetX( float x ) { x_ = x; }
	void SetY( float y ) { y_ = y; }

	float GetX() { return x_; }
	float GetY() { return y_; }
	float GetW() { return w_; }

	int GetType() { return type_; }

	bool CanCollide( unsigned int timer ) 
	{
		if( timer - collidetimer > 2000 )
		{
			collidetimer = timer;

			return true;
		}

		return false;
	}

	// Lab Task 2 : add new member functions here
#ifdef INTERPOLATEMOVEMENT
	void SetServerLocation( float x, float y, float w ) { 
		server_x_ = x; 
		server_y_ = y;
		server_w_ = w;
	}

	void UpdateRespawnLocation(void) {
		server_x_ = rand() % 500 + 100;
		server_y_ = rand() % 400 + 100;
		client_x_ = server_x_;
		client_y_ = server_y_;
		x_ = server_x_;
		y_ = server_y_;
	}

	void SetServerVelocityX( float velocity ) { server_velx_ = velocity; }
	void SetServerVelocityY( float velocity ) { server_vely_ = velocity; }

	float GetServerVelocityX() { return server_velx_; }
	float GetServerVelocityY() { return server_vely_; }

	float GetServerX() { return server_x_; }
	float GetServerY() { return server_y_; }
	float GetServerW() { return server_w_; }

	void DoInterpolateUpdate()
	{
		client_x_ = x_;
		client_y_ = y_;
		client_w_ = w_;
		velocity_x_ = server_velx_;
		velocity_y_ = server_vely_;
		ratio_ = 0;
	}
#endif

};

#endif