#include "explosion.h"


#define EXPLOSION_IMAGE_SIZE 64

explosion::explosion()
: active(false)
, x_(0.f)
, y_(0.f)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(SPR_EXPLOSION);
	hge->Release();
	SA_explosion = new hgeAnimation(tex_, 6, 25, 0, 0, EXPLOSION_IMAGE_SIZE, EXPLOSION_IMAGE_SIZE);
	SA_explosion->SetHotSpot(32, 32);
}

void explosion::SetPosition(int x, int y)
{
	x_ = x;
	y_ = y;
	SA_explosion->Play();
}

explosion::~explosion()
{
}

void explosion::Render()
{
	SA_explosion->Render(x_, y_);	
}

void explosion::SetActive(bool newActive)
{
	this->active = newActive;
}
bool explosion::GetActive(void)
{
	return active;
}

void explosion::Update(double dt)
{
	//Only update if active
	if (active)
	{
		SA_explosion->Update(dt);

		if (SA_explosion->GetFrame() == 5)
		{
			active = false;
		}
	}
}