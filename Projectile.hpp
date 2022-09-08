#pragma once

#include <glm/glm.hpp>


struct Projectile {
	enum Type : int {
		Bomb,
		BombReflect,
		Laser,
		HugeLaser,
		Warn,
		Explosion
	};

	Projectile(float x, float y, float v, Type t, bool h);

	//update position of projectile, if contact is true and the projectile type
	//supports reflection, then velocity is reversed
	bool update(float elapsed, bool contact, float drift);

	//true if the projectile should explode upon being hidden
	bool can_explode();

	//tracks the number of calls to update(), used for animated projectiles
	uint8_t update_cnt;

	//pos is lower left corner of tile
	glm::vec2 pos;

	//horizontal velocity, negative means left
	float vel;

	//whether the projectile is hidden or not
	bool hidden;

	//special flag indicating if this is a projectile that can be reflected by
	//the player when colliding
	Type type;
};