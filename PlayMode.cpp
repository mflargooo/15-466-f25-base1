#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

// for static pointer casts
#include <memory>

#include <algorithm>


// import fixed tiletable
#include "TileTable.hpp"
#include "Entity.hpp"
#include "Sprites.hpp"

#include "Instantiate.hpp"

	
static GenericSpritePrefabs generic_sprites;
static EntityPrefabs entities; 

PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	TileTable::import("../assets/tiletable.tt",  &ppu.tile_table);

	std::array < glm::ivec2, 4 > typical_offsets = { 
		glm::ivec2 ( 0, -8 ), 
		glm::ivec2 (  -8, -8 ), 
		glm::ivec2 ( 0, 0 ), 
		glm::ivec2 (  -8, 0 ) 
	};

	// setup generic tank and bullet sprites to be used across player and enemies
	GenericSprite tank_up, tank_down, tank_left, tank_right, explosion_0, explosion_1, explosion_2, explosion_3, bullet;
	tank_up.tile_idxs = { (uint8_t) 0, (uint8_t) 1, (uint8_t) 8, (uint8_t) 9 };
	tank_down.tile_idxs = { (uint8_t) 2, (uint8_t) 3, (uint8_t) 10, (uint8_t) 11 };
	tank_left.tile_idxs = { (uint8_t) 4, (uint8_t) 5, (uint8_t) 12, (uint8_t) 13 };
	tank_right.tile_idxs = { (uint8_t) 6, (uint8_t) 7, (uint8_t) 14, (uint8_t) 15 };

	bullet.tile_idxs = { (uint8_t) 16 };

	explosion_0.tile_idxs = { (uint8_t) 17, (uint8_t) 18, (uint8_t) 25, (uint8_t) 26 };
	explosion_1.tile_idxs = { (uint8_t) 19, (uint8_t) 20, (uint8_t) 27, (uint8_t) 28 };
	explosion_2.tile_idxs = { (uint8_t) 21, (uint8_t) 22, (uint8_t) 29, (uint8_t) 30 };
	explosion_3.tile_idxs = { (uint8_t) 23, (uint8_t) 24, (uint8_t) 31, (uint8_t) 32 };

	tank_up.offsets = tank_down.offsets = tank_left.offsets = tank_right.offsets = typical_offsets;
	explosion_0.offsets = explosion_1.offsets = explosion_2.offsets = explosion_3.offsets = typical_offsets;

	explosion_0.size = explosion_1.size = explosion_2.size = explosion_3.size = 4;
	tank_up.size = tank_down.size = tank_left.size = tank_right.size = 4;

	bullet.offsets = {
		glm::ivec2 (-4, -5)
	};
	bullet.size = 1;

	generic_sprites["tank_up"] = tank_up;
	generic_sprites["tank_down"] = tank_down;
	generic_sprites["tank_left"] = tank_left;
	generic_sprites["tank_right"] = tank_right;
	generic_sprites["bullet"] = bullet;

	generic_sprites["explosion_0"] = explosion_0;
	generic_sprites["explosion_1"] = explosion_1;
	generic_sprites["explosion_2"] = explosion_2;
	generic_sprites["explosion_3"] = explosion_3;

	std::array< uint8_t, 4 > player_palette = {
		(uint8_t) 7, (uint8_t) 7, (uint8_t) 7, (uint8_t) 7
	};


	// create bullet prefab
	std::shared_ptr< Entities::Bullet > bullet_prefab = std::make_shared< Entities::Bullet >();
	entities["bullet"] = std::static_pointer_cast< Entity >(bullet_prefab);

	bullet_prefab->collider.offset = glm::highp_vec2(0.f);
	bullet_prefab->collider.radius = 5.f;
	bullet_prefab->assign_sprite("0", std::make_shared< Sprite >(Sprite(generic_sprites["bullet"], player_palette)));

	bullet_prefab->set_sprite("0");


	// create and populate player entity
	std::shared_ptr< Entities::Player > player_prefab = std::make_shared< Entities::Player >();
	entities["player"] = std::static_pointer_cast< Entity >(player_prefab);

	player_prefab->collider.offset = glm::highp_vec2(0.f);
	player_prefab->collider.radius = 20.f;

	std::shared_ptr< Sprite > player_up = std::make_shared< Sprite >(Sprite(generic_sprites["tank_up"], player_palette));
	std::shared_ptr< Sprite > player_down = std::make_shared< Sprite >(Sprite(generic_sprites["tank_down"], player_palette));
	std::shared_ptr< Sprite > player_left = std::make_shared< Sprite >(Sprite(generic_sprites["tank_left"], player_palette));
	std::shared_ptr< Sprite > player_right = std::make_shared< Sprite >(Sprite(generic_sprites["tank_right"], player_palette));
	std::shared_ptr< Sprite > player_death_0 = std::make_shared< Sprite >(Sprite(generic_sprites["explosion_0"], player_palette));
	std::shared_ptr< Sprite > player_death_1 = std::make_shared< Sprite >(Sprite(generic_sprites["explosion_1"], player_palette));
	std::shared_ptr< Sprite > player_death_2 = std::make_shared< Sprite >(Sprite(generic_sprites["explosion_2"], player_palette));
	std::shared_ptr< Sprite > player_death_3 = std::make_shared< Sprite >(Sprite(generic_sprites["explosion_3"], player_palette));

	player_prefab->assign_sprite("up", player_up);
	player_prefab->assign_sprite("down", player_down);
	player_prefab->assign_sprite("left", player_left);
	player_prefab->assign_sprite("right", player_right);
	player_prefab->assign_sprite("death_0", player_death_0);
	player_prefab->assign_sprite("death_1", player_death_1);
	player_prefab->assign_sprite("death_2", player_death_2);
	player_prefab->assign_sprite("death_3", player_death_3);

	player_prefab->set_sprite("up");

	// add player as active entity
	player = init_player(&entities, &active_entities);
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
	if (auto weak_player = player.lock()) {
		weak_player->buttons_pressed = ((left.pressed & 0b1) << 4) | ((right.pressed & 0b1) << 3) | ((down.pressed & 0b1) << 2) | ((up.pressed & 0b1) << 1) | (space.pressed & 0b1);
	}
	else return;

	// update object positions
	for (size_t i = 0; i < active_entities.size(); i++) {
		// std::string tag = elem.first;
		if (!active_entities[i]->dead)
			active_entities[i]->update(elapsed);
	}

	// check collisions
	for (size_t i = 0; i < active_entities.size() - 1; i++) {
		if (!active_entities[i]->dead)
			for (size_t j = i + 1; j < active_entities.size(); j++) {
				if (!active_entities[j]->dead)
					active_entities[i]->collides_with(active_entities[j]);
			}
	}

	// remove all elements that have collided.
	// Erase-remove_if idiom from wikipedia - https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
	active_entities.erase(
		std::remove_if(
			active_entities.begin(), active_entities.end(), [elapsed](std::shared_ptr< Entity > entity) {
			return entity->dead && entity->death(elapsed);
		}),
		active_entities.end()
	);

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

	size_t sprites_drawn = 0;
	static uint16_t entity_cycle = 0;
	for (size_t i = 0; i < active_entities.size(); i++) {
		// No more space in the current PPU sprite lis
		if (sprites_drawn >= 64) {
			// start flashing entities
			entity_cycle += 1;
			entity_cycle %= active_entities.size();
			break;
		}

		Entity entity = *active_entities[(i + entity_cycle) % active_entities.size()];
		std::copy(entity.active_ppu_sprite.begin(), entity.active_ppu_sprite.end(), ppu.sprites.begin() + sprites_drawn);
		sprites_drawn += entity.active_ppu_sprite.size();
	}

	if (sprites_drawn < 64) {
		entity_cycle = 0;

		for (size_t i = sprites_drawn; i < 64; i++) {
			ppu.sprites[i].y = 250;
		}
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
