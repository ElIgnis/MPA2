#include "projectile_powerup.h"
#include "ship.h"
#include <hge.h>
#include <hgeSprite.h>
#include <math.h>

extern float GetAbsoluteMag( float num );

Projectile_PowerUp::Projectile_PowerUp(char* filename) 
: collision_X(0.f)
, collision_Y(0.f)
, active(false)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(filename);
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 64, 64));
	sprite_->SetHotSpot(32, 32);
}

Projectile_PowerUp::~Projectile_PowerUp()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
}

void Projectile_PowerUp::Init(float x, float y)
{
	active = true;
	x_ = x;
	y_ = y;
}

bool Projectile_PowerUp::Update(std::vector<Ship*> &shiplist, float timedelta)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	float oldx, oldy;

	oldx = x_;
	oldy = y_;

	for (std::vector<Ship*>::iterator thisship = shiplist.begin(); thisship != shiplist.end(); thisship++)
	{
		//Only update self
		if (HasCollided((*(*thisship))))
		{
			collision_X = x_;
			collision_Y = y_;
			if ((*thisship)->GetID() == shiplist.at(0)->GetID())
			{
				if ((*thisship)->IncreasePower())
				{
					std::cout << "Received power upgrade" << std::endl;
				}
				else
				{
					std::cout << "Power upgrade maxed, converted to score" << std::endl;
				}
			}
			
			active = false;
			return true;
		}
	}
	return false;
}

void Projectile_PowerUp::Render()
{
	sprite_->Render(x_, y_);
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

bool Projectile_PowerUp::GetActive(void)
{
	return active;
}