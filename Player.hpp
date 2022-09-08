#pragma once

#include "Projectile.hpp"
#include "PPU466.hpp"
#include <glm/glm.hpp>


struct Player {
	virtual bool check_collide(Projectile &projectile);

	virtual void update(bool left, bool right, bool down, bool up, float elapsed);

	// pos is lower left corner of tile
	glm::vec2 pos = glm::vec2(0.0f, (float)(PPU466::ScreenHeight / 2 - 4));

	float speed = 45.0f;
	float invul = 0.0f;
	int hitpoints = 10;
};