#pragma once

#include "PPU466.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct TileSet {
	TileSet(std::string filename);

	PPU466::Palette palette;
	std::vector< PPU466::Tile > tiles;
};