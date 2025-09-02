#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

// for static pointer casts
#include <memory>

#include <algorithm>
#include <chrono>
#include <iomanip>


// import fixed tiletable
#include "TileTable.hpp"
#include "Entity.hpp"
#include "Sprites.hpp"

#include "Instantiate.hpp"

	
static GenericSpritePrefabs generic_sprites;
static EntityPrefabs entities; 

PlayMode::PlayMode() {

	TileTable::import("../assets/tiletable.tt",  &ppu.tile_table);

	std::array < glm::ivec2, 4 > typical_offsets = { 
		glm::ivec2 ( 0, -8 ), 
		glm::ivec2 (  -8, -8 ), 
		glm::ivec2 ( 0, 0 ), 
		glm::ivec2 (  -8, 0 ) 
	};

	// generated with coolors.co

	// normal heat
	ppu.palette_table[0] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x91, 0xe3, 0x78, 0xff),
		glm::u8vec4(0x5e, 0xce, 0x41, 0xff),
		glm::u8vec4(0x2b, ~0x00, 0x25, 0xff),
	};

	// more overheated
	ppu.palette_table[1] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0xf3, 0x81, 0x38, 0xff),
		glm::u8vec4(0xfe, 0x3e, 0x21, 0xff),
		glm::u8vec4(~0x00, 0x2b, 0x25, 0xff),
	};

	// overheat recovery
	ppu.palette_table[2] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x9c, 0xb3, 0xf2, 0xff),
		glm::u8vec4(0x60, 0x84, 0xed, 0xff),
		glm::u8vec4(0x2b, 0x25, ~0x00, 0xff),
	};

	// non-player bullets
	ppu.palette_table[4] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(~0x0d, ~0x0a, ~0x0f, 0xff),
		glm::u8vec4(~0x8d, ~0x8a, ~0x8f, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
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

	// create bullet prefab
	std::shared_ptr< Entities::Bullet > bullet_prefab = std::make_shared< Entities::Bullet >();
	entities["bullet"] = std::static_pointer_cast< Entity >(bullet_prefab);

	bullet_prefab->collider.offset = glm::highp_vec2(0.f);
	bullet_prefab->collider.radius = 5.f;
	bullet_prefab->assign_sprite("0", std::make_shared< Sprite >(Sprite(generic_sprites["bullet"], { 3, 3, 3, 3 })));

	bullet_prefab->set_sprite("0");


	// create and populate player entity
	std::shared_ptr< Entities::Player > player_prefab = std::make_shared< Entities::Player >();
	entities["player"] = std::static_pointer_cast< Entity >(player_prefab);

	player_prefab->position = glm::highp_vec2(float(PPU466::ScreenWidth) * .5f - 8.f, float(PPU466::ScreenHeight) * .5f - 8.f);
	player_prefab->collider.offset = glm::highp_vec2(0.f);
	player_prefab->collider.radius = 20.f;

	std::shared_ptr< Sprite > player_up = std::make_shared< Sprite >(Sprite(generic_sprites["tank_up"], { 3, 3, 3, 3}));
	std::shared_ptr< Sprite > player_down = std::make_shared< Sprite >(Sprite(generic_sprites["tank_down"], { 3, 3, 3, 3}));
	std::shared_ptr< Sprite > player_left = std::make_shared< Sprite >(Sprite(generic_sprites["tank_left"], { 3, 3, 3, 3}));
	std::shared_ptr< Sprite > player_right = std::make_shared< Sprite >(Sprite(generic_sprites["tank_right"], { 3, 3, 3, 3}));
	std::shared_ptr< Sprite > player_death_0 = std::make_shared< Sprite >(Sprite(generic_sprites["explosion_0"], { 3, 3, 3, 3}));
	std::shared_ptr< Sprite > player_death_1 = std::make_shared< Sprite >(Sprite(generic_sprites["explosion_1"], { 3, 3, 3, 3}));
	std::shared_ptr< Sprite > player_death_2 = std::make_shared< Sprite >(Sprite(generic_sprites["explosion_2"], { 3, 3, 3, 3}));
	std::shared_ptr< Sprite > player_death_3 = std::make_shared< Sprite >(Sprite(generic_sprites["explosion_3"], { 3, 3, 3, 3}));

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
	srand(unsigned int(std::chrono::system_clock::now().time_since_epoch().count()));
}

PlayMode::~PlayMode() {
	for (auto entity : entities) {
		entity.second.reset();
	}
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
		} else if (evt.key.key == SDLK_ESCAPE) {
			esc.downs += 1;
			esc.pressed = true;
			return true;
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
		} else if (evt.key.key == SDLK_ESCAPE) {
			esc.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	if (esc.pressed) {
		Mode::set_current(nullptr);
		std::cout << "You survived for " << std::setprecision(2) << game_time << " seconds!";
		return;
	}

	if (auto p = player.lock())
		p->buttons_pressed = ((left.pressed & 0b1) << 4) | ((right.pressed & 0b1) << 3) | ((down.pressed & 0b1) << 2) | ((up.pressed & 0b1) << 1) | (space.pressed & 0b1);
	else return;

	// spawn new projectiles on timer
	static float proj_timer = 0.f;
	static size_t bullet_cycle = 0;

	if (proj_timer <= 0.f) {
		std::weak_ptr< Entities::Bullet > weak_bullet;
		if (active_entities.size() - 1 == MAX_PROJS) {
			weak_bullet = std::static_pointer_cast< Entities::Bullet >(active_entities[1 + bullet_cycle]);
			bullet_cycle += 1;

		}
		else {
			weak_bullet = init_bullet(&entities, &active_entities);
		}

		if (auto bullet = weak_bullet.lock()) {
			bullet->position.x = (float) rand() / (float) RAND_MAX < .5f ? 0.f : PPU466::ScreenWidth;
			bullet->position.y = (float) rand() / (float) RAND_MAX < .5f ? 0.f : PPU466::ScreenHeight;

			float angle = (float) rand() / (float) RAND_MAX * (float) M_PI * 2.f;
			bullet->direction.x = std::cos(angle);
			bullet->direction.y = std::sin(angle);

			bullet->travel_speed += 4.f * std::cos(game_time * .25f) + game_time + ((float) rand() / (float) RAND_MAX - .5f) * 20.f;
			bullet->travel_speed = std::max(30.f, std::min(bullet->travel_speed, 150.f));

			proj_timer = spawn_proj_cooldown - std::min(game_time / 30.f, 2.f) + .5f * std::sin(game_time);
		}
	}
	else {
		proj_timer -= elapsed;
	}

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

	// update player palette as linear combination of "normal" to "overheat"
	if (auto p = player.lock()) {
		for (size_t i = 0; i < 4; i++) {
			ppu.palette_table[3][i] = glm::u8vec4(glm::vec4(ppu.palette_table[0][i]) * (1.0f - p->overheat) + glm::vec4(ppu.palette_table[p->can_move ? 1 : 2][i]) * p->overheat);
		}
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
	esc.downs = 0;

	game_time += elapsed;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	ppu.background_color = glm::u8vec4(
		0x0d,
		0x0a,
		0x0f,
		0xff
	);

	for (size_t i = 0; i < PPU466::BackgroundHeight * PPU466::BackgroundWidth; i++) {
		ppu.background[i] = 255;
	}

	size_t sprites_drawn = 0;
	// we will always draw the player
	if (auto p = player.lock()) {
		std::copy(p->active_ppu_sprite.begin(), p->active_ppu_sprite.end(), ppu.sprites.begin());
		sprites_drawn += p->active_ppu_sprite.size();
	}

	static uint16_t entity_cycle = 0;
	for (size_t i = 1; i < active_entities.size(); i++) {
		// No more space in the current PPU sprite lis
		if (sprites_drawn >= 64) {
			// start flashing entities
			entity_cycle += 1;
			entity_cycle %= (active_entities.size() - 1);
			break;
		}

		assert((i + entity_cycle) % (active_entities.size() - 1) + 1 != 0);
		Entity entity = *active_entities[(i + entity_cycle) % (active_entities.size() - 1) + 1];
		std::copy(entity.active_ppu_sprite.begin(), entity.active_ppu_sprite.end(), ppu.sprites.begin() + sprites_drawn);
		sprites_drawn += entity.active_ppu_sprite.size();
	}

	if (sprites_drawn < 64) {
		entity_cycle = 0;

		for (size_t i = sprites_drawn; i < 64; i++) {
			ppu.sprites[i].y = 240;
		}
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
