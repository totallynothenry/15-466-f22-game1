#include "PlayMode.hpp"

#include "data_path.hpp"
#include "Load.hpp"
#include "TileSet.hpp"


//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <random>
#include <unordered_set>

#define MISC_TILE_IDX 0
#define MISC_PALETTE_IDX 0

#define LASERS_TILE_IDX 1
#define LASERS_PALETTE_IDX 2

#define BOMBS_TILE_IDX 3
#define BOMBS_PALETTE_IDX 1

#define EXPLOSION_TILE_IDX 7
#define EXPLOSION_PALETTE_IDX 3

#define PLAYER_TILE_IDX 32
#define PLAYER_PALETTE_IDX 4

#define ENEMY_TILE_IDX 64
#define ENEMY_PALETTE_IDX 5

#define ROAD_TILE_IDX 128
#define ROAD_PALETTE_IDX 6
#define ROAD_PALETTE_MAX (ROAD_PALETTE_IDX << 8)

#define WATER_TILE_IDX 192
#define WATER_PALETTE_IDX 7
#define WATER_PALETTE_MAX (WATER_PALETTE_IDX << 8)

#define HIDE_X 0
#define HIDE_Y 400
#define UPDATES_PER_FRAME 50

#define LOG( X ) std::cerr << X << std::endl


static Load< TileSet > player_tileset(LoadTagDefault, [&](){
	//Load the player tileset
	const TileSet *ts(new TileSet(data_path("resources/ppu4_player.csv")));
	return ts;
});

static Load< TileSet > enemy_tileset(LoadTagDefault, [&](){
	//Load the road tileset
	const TileSet *ts(new TileSet(data_path("resources/ppu4_enemy.csv")));
	return ts;
});

static Load< TileSet > bombs_tileset(LoadTagDefault, [&](){
	//Load the road tileset
	const TileSet *ts(new TileSet(data_path("resources/ppu4_bombs.csv")));
	return ts;
});

static Load< TileSet > lasers_tileset(LoadTagDefault, [&](){
	//Load the road tileset
	const TileSet *ts(new TileSet(data_path("resources/ppu4_lasers.csv")));
	return ts;
});

static Load< TileSet > misc_tileset(LoadTagDefault, [&](){
	//Load the road tileset
	const TileSet *ts(new TileSet(data_path("resources/ppu4_misc.csv")));
	return ts;
});

static Load< TileSet > explosion_tileset(LoadTagDefault, [&](){
	//Load the road tileset
	const TileSet *ts(new TileSet(data_path("resources/ppu4_explosion.csv")));
	return ts;
});

static Load< TileSet > road_tileset(LoadTagDefault, [&](){
	//Load the road tileset
	const TileSet *ts(new TileSet(data_path("resources/ppu4_road_tiles.csv")));
	return ts;
});

static Load< TileSet > water_tileset(LoadTagDefault, [&](){
	//Load the water tileset
	const TileSet *ts(new TileSet(data_path("resources/ppu4_water_tiles.csv")));
	return ts;
});


static float background_x = 0;

static std::unordered_set< uint8_t > enemy_ignore_idx;

static int attack_count = 0;


static uint8_t get_projectile_tile_idx(Projectile::Type type, uint8_t frame) {
	switch (type) {
	default:
		//fall through
	case Projectile::Type::Warn:
		return MISC_TILE_IDX;
	case Projectile::Type::Laser:
		return LASERS_TILE_IDX;
	case Projectile::Type::HugeLaser:
		return LASERS_TILE_IDX + 1;
	case Projectile::Type::BombReflect:
		return BOMBS_TILE_IDX + (frame % 2);
	case Projectile::Type::Bomb:
		return BOMBS_TILE_IDX + 2 + (frame % 2);
	case Projectile::Type::Explosion:
		return EXPLOSION_TILE_IDX + (frame % 6);
	}
}

static uint8_t get_projectile_palette_idx(Projectile::Type type) {
	switch (type) {
	default:
		//fall through
	case Projectile::Type::Warn:
		return MISC_TILE_IDX;
	case Projectile::Type::Laser:
		//fall through
	case Projectile::Type::HugeLaser:
		return LASERS_PALETTE_IDX;
	case Projectile::Type::BombReflect:
		//fall through
	case Projectile::Type::Bomb:
		return BOMBS_PALETTE_IDX;
	case Projectile::Type::Explosion:
		return EXPLOSION_PALETTE_IDX;
	}
}

static float get_projectile_vel(Projectile::Type type) {
	switch (type) {
	case Projectile::Type::Warn:
		//fall through
	case Projectile::Type::Laser:
		//fall through
	case Projectile::Type::HugeLaser:
		return -80.0f;
	case Projectile::Type::BombReflect:
		//fall through
	case Projectile::Type::Bomb:
		return -20.0f;
	default:
		return 0.0f;
	}
}

PlayMode::PlayMode() {
	{ //Setup player tile and palette
		ppu.palette_table[PLAYER_PALETTE_IDX] = player_tileset->palette;
		ppu.tile_table[PLAYER_TILE_IDX] = player_tileset->tiles[0];
	}

	{ //Setup enemy (this uses up 64 tiles and 56 sprites!)
		ppu.palette_table[ENEMY_PALETTE_IDX] = enemy_tileset->palette;
		int idx = ENEMY_TILE_IDX;
		for (auto &tile : enemy_tileset->tiles) {
			ppu.tile_table[idx] = tile;
			idx++;
		}

		enemy_ignore_idx.insert(0);
		enemy_ignore_idx.insert(1);
		enemy_ignore_idx.insert(4);
		enemy_ignore_idx.insert(8);

		enemy_ignore_idx.insert(52);
		enemy_ignore_idx.insert(56);
		enemy_ignore_idx.insert(60);
		enemy_ignore_idx.insert(61);
	}

	{ //Setup bombs, lasers, misc, and explosion
		Projectile::Type type = Projectile::Type::Warn;
		ppu.palette_table[MISC_PALETTE_IDX] = misc_tileset->palette;
		ppu.tile_table[MISC_TILE_IDX] = misc_tileset->tiles[0];
		projectiles.emplace_back(Projectile(HIDE_X, HIDE_Y, get_projectile_vel(type), type, true));

		type = Projectile::Type::Laser;
		ppu.palette_table[LASERS_PALETTE_IDX] = lasers_tileset->palette;
		ppu.tile_table[LASERS_TILE_IDX] = lasers_tileset->tiles[0];
		ppu.tile_table[LASERS_TILE_IDX + 1] = lasers_tileset->tiles[1];
		projectiles.emplace_back(Projectile(HIDE_X, HIDE_Y, get_projectile_vel(type), type, true));
		projectiles.emplace_back(Projectile(HIDE_X, HIDE_Y, get_projectile_vel(type), type, true));
		type = Projectile::Type::HugeLaser;
		for (int i = 0; i < 7; i++) {
			huge_laser.emplace_back(Projectile(HIDE_X, HIDE_Y, get_projectile_vel(type), type, true));
		}

		ppu.palette_table[BOMBS_PALETTE_IDX] = bombs_tileset->palette;
		int idx = BOMBS_TILE_IDX;
		for (auto &tile : bombs_tileset->tiles) {
			ppu.tile_table[idx] = tile;
			idx++;
		}
		type = Projectile::Type::Bomb;
		projectiles.emplace_back(Projectile(HIDE_X, HIDE_Y, get_projectile_vel(type), type, true));
		type = Projectile::Type::BombReflect;
		projectiles.emplace_back(Projectile(HIDE_X, HIDE_Y, get_projectile_vel(type), type, true));

		ppu.palette_table[EXPLOSION_PALETTE_IDX] = explosion_tileset->palette;
		idx = EXPLOSION_TILE_IDX;
		for (auto &tile : explosion_tileset->tiles) {
			ppu.tile_table[idx] = tile;
			idx++;
		}
		type = Projectile::Type::Explosion;
		projectiles.emplace_back(Projectile(HIDE_X, HIDE_Y, get_projectile_vel(type), type, true));
		projectiles.emplace_back(Projectile(HIDE_X, HIDE_Y, get_projectile_vel(type), type, true));
	}

	{ //Setup background map
		ppu.palette_table[ROAD_PALETTE_IDX] = road_tileset->palette;
		int idx = ROAD_TILE_IDX;
		for (auto &tile : road_tileset->tiles) {
			ppu.tile_table[idx] = tile;
			idx++;
		}
		ppu.palette_table[WATER_PALETTE_IDX] = water_tileset->palette;
		idx = WATER_TILE_IDX;
		for (auto &tile : water_tileset->tiles) {
			ppu.tile_table[idx] = tile;
			idx++;
		}

		//Map is as follows (R is road, W is water):
		//22 rows of water, randomly selected
		//16 rows of road, equating to 8 lanes of 2 rows each
		//22 rows of water, randomly selected

		//Water on both sides
		for (uint32_t y = 0; y < 22; ++y) {
			for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
				ppu.background[x+PPU466::BackgroundWidth*y] =
					(std::rand() % 64 + WATER_TILE_IDX) | WATER_PALETTE_MAX;
			}
		}
		for (uint32_t y = 38; y < PPU466::BackgroundHeight; ++y) {
			for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
				ppu.background[x+PPU466::BackgroundWidth*y] =
					(std::rand() % 64 + WATER_TILE_IDX) | WATER_PALETTE_MAX;
			}
		}

		// Upper road edge
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			ppu.background[x+PPU466::BackgroundWidth*22] =
				(std::rand() % 16 + ROAD_TILE_IDX + 32) | ROAD_PALETTE_MAX;
		}

		// Lower road edge
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			ppu.background[x+PPU466::BackgroundWidth*37] =
				(std::rand() % 16 + ROAD_TILE_IDX + 48) | ROAD_PALETTE_MAX;
		}

		// Road internals
		for (uint32_t y = 23; y < 37; ++y) {
			uint16_t offset = y % 2 == 0 ? 16 : 0;
			for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
				ppu.background[x+PPU466::BackgroundWidth*y] =
					(std::rand() % 16 + ROAD_TILE_IDX + offset) | ROAD_PALETTE_MAX;
			}
		}
	}
}

PlayMode::~PlayMode() {
	//Ideally, should not delete here since wasn't allocated here. Works though...
	delete (const TileSet *)player_tileset;
	delete (const TileSet *)enemy_tileset;
	delete (const TileSet *)bombs_tileset;
	delete (const TileSet *)lasers_tileset;
	delete (const TileSet *)misc_tileset;
	delete (const TileSet *)explosion_tileset;
	delete (const TileSet *)road_tileset;
	delete (const TileSet *)water_tileset;
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//Make it look like player constantly advances left
	float drift = player.speed * 3.0f;
	background_x += drift * elapsed;

	int8_t player_offset = PPU466::ScreenHeight / 2 - 4 - int8_t(player.pos.y);

	if (attack_count < 2) {
		attack_count++;

		//Randomly select a bomb or laser to spawn from the enemy
		//For reference: 1 -- Laser, 2 -- Laser, 3 -- Bomb, 4 -- BombReflect
		int idx = std::rand() % 4;
		while (!projectiles[idx + 1].hidden) {
			idx = (idx + 1) % 4;
		}
		Projectile &projectile = projectiles[idx + 1];
		projectile.pos.x = (float)(PPU466::ScreenWidth - 8);
		//Subtracting player_offset is a hack, but it's the easiest solution
		projectile.pos.y = player.pos.y - player_offset + std::rand() % 16 - 8;
		projectile.vel = get_projectile_vel(projectile.type);
		projectile.update_cnt = 0;
		projectile.hidden = false;

		//There is no spawning of HugeLaser/Warn due to time constrainted. It was
		//planned to be a special attack pattern where all other attacks pause, warning
		//signs appear on several lanes, which are flooded with HugeLaser projectiles.
		//The current game should sufficiently exercise the asset pipeline so this
		//feature has been omitted for the submission.
	}

	player.update(left.pressed, right.pressed, down.pressed, up.pressed, elapsed);

	for (auto &projectile : projectiles) {
		bool contact = !projectile.hidden
			&& projectile.can_explode()
			&& player.check_collide(projectile);
		if (projectile.update(elapsed, contact, -drift)) {
			//Place an explosion if needed and possible
			if (projectile.can_explode()) {
				attack_count--;

				Projectile &explosion = projectiles[5];
				if (!explosion.hidden) {
					explosion = projectiles[6];
				}

				//There's an edge case where 2 explosions are on screen, and the player
				//somehow immediately triggers a third before the other two finish. It's left unhandled
				//since it'll just be a visual bug due to PPU466 sprite count limitations. Currently,
				//the code will refuse to try to spawn the third explosion.

				if (explosion.hidden && projectile.pos.x >= 0) {
					explosion.pos.x = projectile.pos.x;
					explosion.pos.y = projectile.pos.y;
					explosion.update_cnt = 0;
					explosion.hidden = false;

					// LOG("explosion at " << explosion.pos.x << " " << explosion.pos.y << " from " << projectile.type);
				}
			}

			//Hide this projectile
			projectile.pos.x = HIDE_X;
			projectile.pos.y = HIDE_Y;
			projectile.hidden = true;
		}
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color is the first color in water palette
	ppu.background_color = glm::u8vec4(33,49,117,255);

	//background scroll:
	ppu.background_position.x = int32_t(-background_x -player.pos.x);
	ppu.background_position.y = int32_t(-player.pos.y - 4);

	uint32_t sprite_idx = 0;

	//player sprite:
	ppu.sprites[sprite_idx].x = int8_t(player.pos.x);
	ppu.sprites[sprite_idx].y = int8_t(player.pos.y);
	ppu.sprites[sprite_idx].index = PLAYER_TILE_IDX;
	ppu.sprites[sprite_idx].attributes = PLAYER_PALETTE_IDX;
	sprite_idx++;

	int8_t player_offset = PPU466::ScreenHeight / 2 - 4 - int8_t(player.pos.y);

	//enemy sprites (56 sprites that aggregate into the massive enemy)
	uint8_t enemy_x_offset = 224;
	uint8_t enemy_y_offset = 56 + player_offset;
	for (uint8_t i = 0; i < 64; i++) {
		assert(sprite_idx < 64);
		if (i > 0 && i % 4 == 0) {
			enemy_x_offset = 224;
			enemy_y_offset += 8;
		}

		if (enemy_ignore_idx.count(i) == 0) {
			ppu.sprites[sprite_idx].x = enemy_x_offset;
			ppu.sprites[sprite_idx].y = enemy_y_offset;
			ppu.sprites[sprite_idx].index = ENEMY_TILE_IDX + i;
			ppu.sprites[sprite_idx].attributes = ENEMY_PALETTE_IDX;
			sprite_idx++;
		}

		enemy_x_offset += 8;
	}

	//all other sprites (bombs, lasers, explosions, misc)
	for (auto &projectile : projectiles) {
		assert(sprite_idx < 64);
		ppu.sprites[sprite_idx].x = int8_t(projectile.pos.x);
		ppu.sprites[sprite_idx].y = int8_t(projectile.pos.y) + player_offset;
		ppu.sprites[sprite_idx].index =
			get_projectile_tile_idx(projectile.type, projectile.update_cnt / UPDATES_PER_FRAME);
		ppu.sprites[sprite_idx].attributes =
			get_projectile_palette_idx(projectile.type) | (projectile.hidden << 7);
		sprite_idx++;
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
