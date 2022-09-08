#pragma once

#include "TileSet.hpp"
#include "PPU466.hpp"
#include <glm/glm.hpp>


struct Player {
	virtual bool check_collide(PPU466::Tile projectile, glm::vec2 ppos);

	// pos is lower left corner of tile
	glm::vec2 pos = glm::vec2(0.0f, (float)(PPU466::ScreenHeight / 2 - 4));

	int hitpoints = 3;
};