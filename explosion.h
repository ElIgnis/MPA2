#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <hge.h>
#include <hgerect.h>
#include <hgeSprite.h>
#include <memory>
#include <vector>

class hgeSprite;
class hgeRect;

#define EXPLOSION_DURATION 0.5f

class explosion
{
	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the explosion
	float x_; //!< The x-coordinate of the explosion
	float y_; //!< The y-coordinate of the explosion
	bool active;
	float explosionTimer;

public:
	explosion();
	~explosion();
	void Init();
	void Update(double dt);
	void Render();
	void SetPosition(int x, int y);
	void SetActive(bool newActive);
	bool GetActive(void);
};

#endif