#include "Player.hpp"

#include "data_path.hpp"


bool Player::check_collide(PPU466::Tile projectile, glm::vec2 ppos) {
	//Simple collision checker that overlaps the player tile with projectile tile
	//with appropriate offset, and returns true if any overlapping location has
	//non-zero for both.

	//This function assumes palettes for player and project always have the 0th
	//color as fully transparent.

	//TODO
	return false;
}