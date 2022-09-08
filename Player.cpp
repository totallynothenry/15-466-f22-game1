#include "Player.hpp"


void Player::update(bool left, bool right, bool down, bool up, float elapsed) {
	if (left) pos.x -= speed * elapsed;
	if (right) pos.x += (speed / 2.0f) * elapsed;
	if (down) pos.y -= (speed / 2.0f) * elapsed;
	if (up) pos.y += (speed / 2.0f) * elapsed;

	//Bind the player to within the road and behind enemy
	pos.x = std::min(std::max(pos.x, 0.0f), 208.0f);
	pos.y = std::min(std::max(pos.y, 87.0f), 145.0f);

	if (invul > 0.0f) {
		invul = std::max(invul - elapsed, 0.0f);
	}
}

bool Player::check_collide(Projectile &projectile) {
	if (invul > 0 || projectile.hidden) {
		return false;
	}

	float player_offset = (float)(PPU466::ScreenHeight / 2 - 4) - pos.y;

	//Assumes player is 6x8 bounding box, offset by (1,0) from lower left corner
	float pl_min_x = pos.x;
	float pl_max_x = pos.x + 7;
	float pl_min_y = pos.y + 1 - player_offset;
	float pl_max_y = pos.y + 6 - player_offset;

	float min_x, max_x, min_y, max_y;
	switch (projectile.type) {
	case Projectile::Type::Bomb:
		// fall through
	case Projectile::Type::BombReflect:
		// Assume 4x4 bounding box, offset by (2,2) from lower left corner
		min_x = projectile.pos.x + 2;
		max_x = projectile.pos.x + 5;
		min_y = projectile.pos.y + 2;
		max_y = projectile.pos.y + 5;
		break;
	case Projectile::Type::Laser:
		// Assume 4x8 bounding box, offset by (0,2) from lower left corner
		min_x = projectile.pos.x;
		max_x = projectile.pos.x + 7;
		min_y = projectile.pos.y + 2;
		max_y = projectile.pos.y + 5;
		break;
	case Projectile::Type::HugeLaser:
		// Assume 8x8 bounding box, offset by (0,0) from lower left corner
		min_x = projectile.pos.x;
		max_x = projectile.pos.x + 7;
		min_y = projectile.pos.y;
		max_y = projectile.pos.y + 7;
		break;
	default:
		return false;
	}

	// Check for any overlap in bounding boxes
	bool x_overlap = pl_min_x <= max_x && pl_max_x >= min_x;
	bool y_overlap = pl_min_y <= max_y && pl_max_y >= min_y;
	bool collide = x_overlap && y_overlap;

	if (collide && projectile.type != Projectile::Type::BombReflect) {
		invul = 3;
		hitpoints--;
	}

	return collide;
}