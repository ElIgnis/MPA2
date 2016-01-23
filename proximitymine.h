#ifndef _PROXIMITY_MINE_H_
#define _PROXIMITY_MINE_H_

#include <iostream>
#include <hge.h>
#include <hgerect.h>
#include <memory>
#include <vector>
#include <string>

class hgeSprite;
class hgeRect;
class Ship;

using std::string;

#define DELAY_DURATION 3.f
#define SPR_MINE "Images/proxmine.png"

class ProximityMine
{
	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the mine
	float x_; //!< The x-coordinate of the mine
	float y_; //!< The y-coordinate of the mine
	float w_; //!< The angular position of the mine
	float velocity_x_; //!< The resolved velocity of the mine along the x-axis
	float velocity_y_; //!< The resolved velocity of the mine along the y-axis
	hgeRect collidebox;
	int ownerid;
	string ownername;
	int damage, additional_damage;
	float collision_X, collision_Y;
	bool selfDamage;
	bool active;
	bool startTimer;
	float delayTimer;

public:
	float angular_velocity;
	ProximityMine(char* filename, string ownerName);
	~ProximityMine();
	void Init(float x, float y, float w, float vel_X, float vel_Y, int ownerID);
	bool Update(std::vector<Ship*> &shiplist, float timedelta);
	void Render();
	bool HasCollided( Ship &ship );
	float GetCollisionX(void);
	float GetCollisionY(void);
	string GetOwnerName(void);
	float GetProximityMineDmg(void);
	void SetProximityMinePower(int level);
	bool GetSelfDamage(void);
	bool GetActive(void);

	void UpdateLoc( float x, float y, float w )
	{
		x_ = x;
		y_ = y;
		w_ = w;
	}

	int GetOwnerID()
	{
		return ownerid;
	}

	float GetX() const
	{
		return x_;
	}

	float GetY() const
	{
		return y_;
	}

	float GetW() const
	{
		return w_;
	}
	
	float GetVelocityX() { return velocity_x_; }
	float GetVelocityY() { return velocity_y_; }

	void SetVelocityX( float velocity ) { velocity_x_ = velocity; }
	void SetVelocityY( float velocity ) { velocity_y_ = velocity; }

};

#endif