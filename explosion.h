#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <hge.h>
#include <hgerect.h>
#include <memory>
#include <vector>
#include <hgeanim.h>
#include <iostream>

class hgeSprite;
class hgeRect;

#define SPR_EXPLOSION "Images/Explosion.png"

class explosion
{
	HTEXTURE tex_; //!< Handle to the sprite's texture
	//std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the explosion
	float x_; //!< The x-coordinate of the explosion
	float y_; //!< The y-coordinate of the explosion
	bool active;
	hgeAnimation* SA_explosion;

public:
	explosion();
	~explosion();
	void Update(double dt);
	void Render();
	void SetPosition(int x, int y);
	void SetActive(bool newActive);
	bool GetActive(void);
};

#endif