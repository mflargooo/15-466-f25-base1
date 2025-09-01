#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

// for static pointer casts
#include <memory>

// import fixed tiletable
#include "TileTable.hpp"
#include "Entity.hpp"
#include "Sprites.hpp"

	
static GenericSpritePrefabs generic_sprites;
static EntityPrefabs entities; 

PlayMode::PlayMode() {
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
		for (uint32_t index = 17; index < 17 + 16; ++index) {
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

	TileTable::import("../assets/tiletable.tt",  &ppu.tile_table);

	std::array < glm::ivec2, 4 > typical_offsets = { 
		glm::ivec2 ( 0, -8 ), 
		glm::ivec2 (  -8, -8 ), 
		glm::ivec2 ( 0, 0 ), 
		glm::ivec2 (  -8, 0 ) 
	};

	// setup generic tank and bullet sprites to be used across player and enemies
	GenericSprite tank_up, tank_down, tank_left, tank_right, bullet;
	tank_up.tile_idxs = { (uint8_t) 0, (uint8_t) 1, (uint8_t) 8, (uint8_t) 9 };
	tank_down.tile_idxs = { (uint8_t) 2, (uint8_t) 3, (uint8_t) 10, (uint8_t) 11 };
	tank_left.tile_idxs = { (uint8_t) 4, (uint8_t) 5, (uint8_t) 12, (uint8_t) 13 };
	tank_right.tile_idxs = { (uint8_t) 6, (uint8_t) 7, (uint8_t) 14, (uint8_t) 15 };

	bullet.tile_idxs = { (uint8_t) 16 };

	tank_up.offsets = tank_down.offsets = tank_left.offsets = tank_right.offsets = typical_offsets;
	tank_up.size = tank_down.size = tank_left.size = tank_right.size = 4;

	generic_sprites["tank_up"] = tank_up;
	generic_sprites["tank_down"] = tank_down;
	generic_sprites["tank_left"] = tank_left;
	generic_sprites["tank_right"] = tank_right;

	// create and populate player entity, and save it as a prefab
	std::shared_ptr< Entities::Player > player = std::make_shared< Entities::Player >();
	entities["player"] = player;

	std::array< uint8_t, 4 > player_palette = {
		(uint8_t) 7, (uint8_t) 7, (uint8_t) 7, (uint8_t) 7
	};

	std::shared_ptr< Sprite > player_up = std::make_shared< Sprite >(Sprite(generic_sprites["tank_up"], player_palette));
	std::shared_ptr< Sprite > player_down = std::make_shared< Sprite >(Sprite(generic_sprites["tank_down"], player_palette));
	std::shared_ptr< Sprite > player_left = std::make_shared< Sprite >(Sprite(generic_sprites["tank_left"], player_palette));
	std::shared_ptr< Sprite > player_right = std::make_shared< Sprite >(Sprite(generic_sprites["tank_right"], player_palette));

	player->assign_sprite("up", player_up);
	player->assign_sprite("down", player_down);
	player->assign_sprite("left", player_left);
	player->assign_sprite("right", player_right);

	player->set_sprite("up");

	player->on_update = [player](float elapsed) {
		bool left_pressed = (player->buttons_pressed & 0b10000) >> 4;
		bool right_pressed = (player->buttons_pressed & 0b1000) >> 3;
		bool down_pressed = (player->buttons_pressed & 0b100) >> 2;
		bool up_pressed = (player->buttons_pressed & 0b10) >> 1;
		bool space_pressed = player->buttons_pressed & 0b1;
		space_pressed;

		int8_t dir_x = right_pressed - left_pressed;
		int8_t dir_y = up_pressed - down_pressed;

		if ((left_pressed || right_pressed) && !(down_pressed ^ up_pressed)) {
			player->set_sprite(dir_x == 1 ? "right" : "left");
		}
		if ((down_pressed || up_pressed) && !(left_pressed ^ right_pressed)) {
			player->set_sprite(dir_y == 1 ? "up" : "down");
		}

		player->position.x += dir_x * (dir_x ^ dir_y ? 1.f : .707f) * player->move_speed * elapsed;
		player->position.y += dir_y * (dir_x ^ dir_y ? 1.f : .707f) * player->move_speed * elapsed;

		// clamp player position to screen
		player->position.x = std::max(0.f, std::min(player->position.x, (float) PPU466::BackgroundWidth * 4.f - 16.f));
		player->position.y = std::max(0.f, std::min(player->position.y, (float) PPU466::BackgroundHeight * 4.f - 16.f));
	};
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_DOWN) {
			down.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	auto it = entities.find("player");
	std::shared_ptr< Entities::Player > player = std::static_pointer_cast < Entities::Player >((it != entities.end()) ? it->second : nullptr);
	if (player) {
		player->buttons_pressed = ((left.pressed & 0b1) << 4) | ((right.pressed & 0b1) << 3) | ((down.pressed & 0b1) << 2) | ((up.pressed & 0b1) << 1) | (space.pressed & 0b1);
		player->update(elapsed);

	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	ppu.background_color = glm::u8vec4(
		0,
		0,
		0,
		0xff
	);

	for (size_t i = 0; i < PPU466::BackgroundHeight * PPU466::BackgroundWidth; i++) {
		ppu.background[i] = 255;
	}

	auto it = entities.find("player");
	std::shared_ptr< Entity > player = std::static_pointer_cast < Entity >((it != entities.end()) ? it->second : nullptr);

	//player sprite:
	for (size_t i = 0; i < player->active_sprite.size(); i++) {
		ppu.sprites[i].x = int8_t(player->position.x) - int8_t(player->active_sprite[i].x);
		ppu.sprites[i].y = int8_t(player->position.y) - int8_t(player->active_sprite[i].y);
		ppu.sprites[i].index = 	player->active_sprite[i].index;
		ppu.sprites[i].attributes = player->active_sprite[i].attributes;
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
