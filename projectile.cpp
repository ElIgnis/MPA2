#include "projectile.h"
#include "ship.h"
#include <hge.h>
#include <hgeSprite.h>
#include <math.h>

extern float GetAbsoluteMag( float num );

Projectile::Projectile(char* filename, string ownerName) :
	angular_velocity(0)
	, collision_X(0.f)
	, collision_Y(0.f)
	, ownername("")
	, damage(20)
	, additional_damage(0)
	, selfDamage(false)
	, active(false)
	, activeTimer(0.f)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(filename);
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 16, 16));
	sprite_->SetHotSpot(8, 8);

	ownername = ownerName;
	selfDamage = false;
}

Projectile::~Projectile()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
}

void Projectile::Init(float x, float y, float w, int ownerID)
{
	active = true;
	activeTimer = 0.f;
	ownerid = ownerID;

	x_ = x;
	y_ = y;
	w_ = w;

	velocity_x_ = 135.0f * cosf(w_);
	velocity_y_ = 135.0f * sinf(w_);

	x_ += velocity_x_ * 0.5;
	y_ += velocity_y_ * 0.5;
}

bool Projectile::Update(std::vector<Ship*> &shiplist, float timedelta)
{
	//Dont update if projectile is inactive
	if (active)
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
			if (HasCollided((*(*thisship))))
			{
				if ((*thisship)->GetID() == ownerid)
				{
					selfDamage = true;
				}
				collision_X = x_;
				collision_Y = y_;
				active = false;
				return true;
			}
		}

		//Expiry timer
		activeTimer += timedelta;

		if (activeTimer > ACTIVE_DURATION)
		{
			active = false;
			return false;
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
	}
	return false;
}

void Projectile::Render()
{
	sprite_->RenderEx(x_, y_, w_);
}

bool Projectile::HasCollided( Ship &ship )
{
	sprite_->GetBoundingBox( x_, y_, &collidebox);

	return collidebox.Intersect( ship.GetBoundingBox() );
}

float Projectile::GetCollisionX(void)
{
	return collision_X;
}
float Projectile::GetCollisionY(void)
{
	return collision_Y;
}

string Projectile::GetOwnerName(void)
{
	return ownername;
}

float Projectile::GetProjectileDmg(void)
{
	return (damage + additional_damage);
}
void Projectile::SetProjectilePower(int level)
{
	additional_damage = level * 10;
}

bool Projectile::GetSelfDamage(void)
{
	return selfDamage;
}

bool Projectile::GetActive(void)
{
	return active;
}