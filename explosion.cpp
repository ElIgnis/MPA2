#include "explosion.h"

#define EXPLOSION_DIR "Images/Explosion.png"
#define EXPLOSION_IMAGE_SIZE 64

explosion::explosion()
: active(false)
, explosionTimer(0.f)
, x_(0.f)
, y_(0.f)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(EXPLOSION_DIR);
	hge->Release();
}

void explosion::Init()
{
	//sprite_.reset(new hgeSprite(tex_, 0, 0, EXPLOSION_IMAGE_SIZE, EXPLOSION_IMAGE_SIZE));
}

void explosion::SetPosition(int x, int y)
{
	x_ = x;
	y_ = y;
	sprite_.reset(new hgeSprite(tex_, 0, 0, EXPLOSION_IMAGE_SIZE, EXPLOSION_IMAGE_SIZE));
	sprite_->SetHotSpot(32, 32);
}

explosion::~explosion()
{
	sprite_.release();
}

void explosion::Render()
{
	if (active)
		sprite_->Render(x_, y_);
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
		this->explosionTimer += dt;

		if (explosionTimer > EXPLOSION_DURATION)
		{
			active = false;
			explosionTimer = 0.f;
		}
	}
}