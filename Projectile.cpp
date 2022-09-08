#include "Projectile.hpp"

#include "PPU466.hpp"


Projectile::Projectile(float x, float y, float v, Projectile::Type t, bool h) {
	pos = glm::vec2(x, y);
	vel = v;
	type = t;
	update_cnt = 0;
	hidden = h;
}

bool Projectile::update(float elapsed, bool contact, float drift) {
	if (hidden) return false;

	update_cnt++;
	if (contact && type == BombReflect) {
		vel = 160.0f;
		contact = false;
	}
	pos.x += (vel + drift) * elapsed;
	return contact
		|| (pos.x < 0)
		|| (pos.x > PPU466::ScreenWidth - 4)
		|| (type == Projectile::Type::Explosion && update_cnt >= 300);
}

bool Projectile::can_explode() {
	return type != Projectile::Type::Explosion && type != Projectile::Type::Warn;
}