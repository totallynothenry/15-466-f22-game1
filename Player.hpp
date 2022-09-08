#pragma once

#include "TileSet.hpp"
#include "PPU466.hpp"
#include <glm/glm.hpp>

#define PLAYER_TILE_IDX 32
#define PLAYER_PALETTE_IDX 7


struct Player {
	virtual ~Player();

	virtual bool check_collide(PPU466::Tile projectile, glm::vec2 ppos);

	// Not centered, lower left corner
	glm::vec2 pos = glm::vec2(0.0f);

	int hitpoints = 3;
};

const TileSet *player_load_function();