#include "PlayMode.hpp"

#include "data_path.hpp"
#include "Load.hpp"
#include "TileSet.hpp"


//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

#define PLAYER_TILE_IDX 32
#define PLAYER_PALETTE_IDX 7

#define ROAD_TILE_IDX 64
#define ROAD_PALETTE_IDX 0
#define ROAD_PALETTE_MAX (ROAD_PALETTE_IDX << 8)

#define WATER_TILE_IDX 128
#define WATER_PALETTE_IDX 1
#define WATER_PALETTE_MAX (WATER_PALETTE_IDX << 8)


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



PlayMode::PlayMode() {
	{ //Setup player tile and palette
		ppu.palette_table[PLAYER_PALETTE_IDX] = player_tileset->palette;
		ppu.tile_table[PLAYER_TILE_IDX] = player_tileset->tiles[0];
	}

	{ //Setup enemy

	}

	{ //Setup bombs, lasers, misc, and explosion

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
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	//Also, *don't* use these tiles in your game:

	{ //use tiles 0-16 as some weird dot pattern thing:
		std::array< uint8_t, 8*8 > distance;
		for (uint32_t y = 0; y < 8; ++y) {
			for (uint32_t x = 0; x < 8; ++x) {
				float d = glm::length(glm::vec2((x + 0.5f) - 4.0f, (y + 0.5f) - 4.0f));
				d /= glm::length(glm::vec2(4.0f, 4.0f));
				distance[x+8*y] = uint8_t(std::max(0,std::min(255,int32_t( 255.0f * d ))));
			}
		}
		for (uint32_t index = 0; index < 16; ++index) {
			PPU466::Tile tile;
			uint8_t t = uint8_t((255 * index) / 16);
			for (uint32_t y = 0; y < 8; ++y) {
				uint8_t bit0 = 0;
				uint8_t bit1 = 0;
				for (uint32_t x = 0; x < 8; ++x) {
					uint8_t d = distance[x+8*y];
					if (d > t) {
						bit0 |= (1 << x);
					} else {
						bit1 |= (1 << x);
					}
				}
				tile.bit0[y] = bit0;
				tile.bit1[y] = bit1;
			}
			ppu.tile_table[index] = tile;
		}
	}

	//makes the outside of tiles 0-16 solid:
	// ppu.palette_table[0] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// };

	//makes the center of tiles 0-16 solid:
	// ppu.palette_table[1] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// };

	//used for the misc other sprites:
	ppu.palette_table[6] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x88, 0x88, 0xff, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	};

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

	constexpr float PlayerSpeed = 45.0f;

	//Make it look like player constantly advances left
	background_x += (PlayerSpeed * 3.0f) * elapsed;

	if (left.pressed) player.pos.x -= PlayerSpeed * elapsed;
	if (right.pressed) player.pos.x += PlayerSpeed * elapsed;
	if (down.pressed) player.pos.y -= (PlayerSpeed / 2.0f) * elapsed;
	if (up.pressed) player.pos.y += (PlayerSpeed / 2.0f) * elapsed;

	//Binder the player to within the road
	player.pos.x = std::min(std::max(player.pos.x, 0.0f), 216.0f);
	player.pos.y = std::min(std::max(player.pos.y, 87.0f), 145.0f);

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

	//tilemap gets recomputed every frame as some weird plasma thing:
	//NOTE: don't do this in your game! actually make a map or something :-)
	// for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
	// 	for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
	// 		//TODO: make weird plasma thing
	// 		ppu.background[x+PPU466::BackgroundWidth*y] = ((x+y)%16);
	// 	}
	// }

	//background scroll:
	ppu.background_position.x = int32_t(-background_x -player.pos.x);
	ppu.background_position.y = int32_t(-player.pos.y - 4);

	//player sprite:
	ppu.sprites[0].x = int8_t(player.pos.x);
	ppu.sprites[0].y = int8_t(player.pos.y);
	ppu.sprites[0].index = PLAYER_TILE_IDX;
	ppu.sprites[0].attributes = PLAYER_PALETTE_IDX;

	//some other misc sprites:
	for (uint32_t i = 1; i < 63; ++i) {
		float amt = (i + 2.0f * background_fade) / 62.0f;
		ppu.sprites[i].x = int8_t(0.5f * PPU466::ScreenWidth + std::cos( 2.0f * M_PI * amt * 5.0f + 0.01f * player.pos.x) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].y = int8_t(0.5f * PPU466::ScreenHeight + std::sin( 2.0f * M_PI * amt * 3.0f + 0.01f * player.pos.y) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].index = 32;
		ppu.sprites[i].attributes = 6;
		if (i % 2) ppu.sprites[i].attributes |= 0x80; //'behind' bit
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
