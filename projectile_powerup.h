#ifndef _PROJECTILE_PWRUP_H_
#define _PROJECTILE_PWRUP_H_

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

#define INCREMENT 5
#define SPR_PROJ_PWRUP "Images/PowerUp.png"

class Projectile_PowerUp
{
	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the powerup
	float x_; //!< The x-coordinate of the powerup
	float y_; //!< The y-coordinate of the powerup
	float collision_X, collision_Y;
	hgeRect collidebox;
	bool active;

public:
	Projectile_PowerUp(char* filename);
	~Projectile_PowerUp();
	void Init(float x, float y);
	bool Update(std::vector<Ship*> &shiplist, float timedelta);
	void Render();
	bool HasCollided( Ship &ship );
	float GetCollisionX(void);
	float GetCollisionY(void);
	bool GetActive(void);

	void UpdateLoc( float x, float y)
	{
		x_ = x;
		y_ = y;
	}

	float GetX() const
	{
		return x_;
	}

	float GetY() const
	{
		return y_;
	}
};

#endif