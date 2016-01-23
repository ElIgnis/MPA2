#include "projectile_powerup.h"
#include "ship.h"
#include <hge.h>
#include <hgeSprite.h>
#include <math.h>

extern float GetAbsoluteMag( float num );

Projectile_PowerUp::Projectile_PowerUp(char* filename, string ownerName) :
	angular_velocity(0)
	, collision_X(0.f)
	, collision_Y(0.f)
	, ownername("")
	, damage(20)
	, additional_damage(0)
	, selfDamage(false)
	, active(false)
	, delayTimer(0.f)
	, startTimer(false)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(filename);
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 32, 32));
	sprite_->SetHotSpot(16, 16);

	ownername = ownerName;
	selfDamage = false;
}

Projectile_PowerUp::~Projectile_PowerUp()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
}

void Projectile_PowerUp::Init(float x, float y, float w, float vel_X, float vel_Y, int ownerID)
{
	delayTimer = 0.f;
	ownerid = ownerID;
	startTimer = true;

	x_ = x;
	y_ = y;
	w_ = w;

	velocity_x_ = vel_X;
	velocity_y_ = vel_Y;
}

bool Projectile_PowerUp::Update(std::vector<Ship*> &shiplist, float timedelta)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	float pi = 3.141592654f * 2;
	float oldx, oldy;

	w_ += angular_velocity * timedelta;
	if (w_ > pi)
		w_ -= pi;

	if (w_ < 0.0f)
		w_ += pi;

	oldx = x_;
	oldy = y_;

	x_ += velocity_x_ * timedelta;
	y_ += velocity_y_ * timedelta;

	for (std::vector<Ship*>::iterator thisship = shiplist.begin(); thisship != shiplist.end(); thisship++)
	{
		if (active == true && HasCollided((*(*thisship))))
		{
			if ((*thisship)->GetID() == ownerid)
			{
				selfDamage = true;
			}
			collision_X = x_;
			collision_Y = y_;
			active = false;
			startTimer = false;
			return true;
		}
	}

	//Expiry timer
	if (startTimer)
	{
		delayTimer += timedelta;

		if (delayTimer > DELAY_DURATION)
		{
			active = true;
		}
	}

	//Wrap around
	float screenwidth = static_cast<float>(hge->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge->System_GetState(HGE_SCREENHEIGHT));
	float spritewidth = sprite_->GetWidth();
	float spriteheight = sprite_->GetHeight();
	if (x_ < -spritewidth / 2)
		x_ += screenwidth + spritewidth;
	else if (x_ > screenwidth + spritewidth / 2)
		x_ -= screenwidth + spritewidth;

	if (y_ < -spriteheight / 2)
		y_ += screenheight + spriteheight;
	else if (y_ > screenheight + spriteheight / 2)
		y_ -= screenheight + spriteheight;

	return false;
}

void Projectile_PowerUp::Render()
{
	sprite_->RenderEx(x_, y_, w_);
}

bool Projectile_PowerUp::HasCollided( Ship &ship )
{
	sprite_->GetBoundingBox( x_, y_, &collidebox);

	return collidebox.Intersect( ship.GetBoundingBox() );
}

float Projectile_PowerUp::GetCollisionX(void)
{
	return collision_X;
}
float Projectile_PowerUp::GetCollisionY(void)
{
	return collision_Y;
}

string Projectile_PowerUp::GetOwnerName(void)
{
	return ownername;
}

float Projectile_PowerUp::GetProjectile_PowerUpDmg(void)
{
	return (damage + additional_damage);
}
void Projectile_PowerUp::SetProjectile_PowerUpPower(int level)
{
	additional_damage = level * 10;
}

bool Projectile_PowerUp::GetSelfDamage(void)
{
	return selfDamage;
}

bool Projectile_PowerUp::GetActive(void)
{
	return startTimer;
}