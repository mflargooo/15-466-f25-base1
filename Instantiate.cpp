#include "Entity.hpp"
#include "PPU466.hpp"

#include <iostream>

std::weak_ptr< Entities::Bullet > init_bullet(EntityPrefabs *table, std::vector< std::shared_ptr< Entity > > *into) {
	into->emplace_back(std::make_shared< Entities::Bullet >(*std::static_pointer_cast< Entities::Bullet >((*table)["bullet"])));
    std::weak_ptr< Entities::Bullet > weak_bullet = std::static_pointer_cast< Entities::Bullet >(into->back());
    if (auto b = weak_bullet.lock()) {
		b->on_update = [weak_bullet](float elapsed) {
			std::shared_ptr< Entities::Bullet > bullet;
			if (!(bullet = weak_bullet.lock())) return;

			bullet->position.x += bullet->direction.x * bullet->travel_speed * elapsed;
			bullet->position.y += bullet->direction.y * bullet->travel_speed * elapsed;

			if (bullet->position.x < 0.f) {
				bullet->position.x = (float)PPU466::ScreenWidth;
			}
			else if (bullet->position.x > (float)PPU466::ScreenWidth) {
				bullet->position.x = 0;
			}
			if (bullet->position.y < 0.f) {
				bullet->position.y = (float)PPU466::ScreenHeight;
			}
			else if (bullet->position.y > (float)PPU466::ScreenHeight) {
				bullet->position.y = 0;
			}
		};
	}

    return weak_bullet;
}

std::weak_ptr< Entities::Player > init_player(EntityPrefabs *table, std::vector< std::shared_ptr< Entity > > *into) {
    into->emplace_back(std::make_shared< Entities::Player >(*std::static_pointer_cast< Entities::Player >((*table)["player"])));
	std::weak_ptr< Entities::Player > weak_player = std::static_pointer_cast< Entities::Player >(into->back());
	if (auto p = weak_player.lock()) {
		p->on_update = [weak_player, table, into](float elapsed) {
			static float attack_speed_cooldown = 0.f;
			static std::string look = "up";

			std::shared_ptr< Entities::Player > player;
			if (!(player = weak_player.lock())) return;

			if (player->can_move) {

				bool left_pressed = (player->buttons_pressed & 0b10000) >> 4;
				bool right_pressed = (player->buttons_pressed & 0b1000) >> 3;
				bool down_pressed = (player->buttons_pressed & 0b100) >> 2;
				bool up_pressed = (player->buttons_pressed & 0b10) >> 1;
				bool space_pressed = player->buttons_pressed & 0b1;

				int8_t dir_x = right_pressed - left_pressed;
				int8_t dir_y = up_pressed - down_pressed;

				if ((left_pressed || right_pressed) && !(down_pressed ^ up_pressed)) {
					look = dir_x == 1 ? "right" : "left";
					player->set_sprite(look);
				}
				if ((down_pressed || up_pressed) && !(left_pressed ^ right_pressed)) {
					look = dir_y == 1 ? "up" : "down";
					player->set_sprite(look);
				}

				if (space_pressed && attack_speed_cooldown <= 0.f) {
					std::weak_ptr< Entities::Bullet > weak_bullet = init_bullet(table, into);

					if (auto bullet = weak_bullet.lock()) {
						bullet->direction.x = (look.compare("left") == 0 ? -1.f : (look.compare("right") == 0 ? 1.f : 0.f));
						bullet->direction.y = (look.compare("down") == 0 ? -1.f : (look.compare("up") == 0 ? 1.f : 0.f));

						bullet->position.x = player->position.x + bullet->direction.x * 8.f;
						bullet->position.y = player->position.y + bullet->direction.y * 8.f;

						bullet->travel_speed = 80.f;

						attack_speed_cooldown = player->attack_speed;
					}
				}
				else if (attack_speed_cooldown > 0.f) {
					attack_speed_cooldown -= elapsed;
				}
				

				player->position.x += dir_x * (dir_x * dir_y > 0 ? 1.1f : 1.0f) * (dir_x ^ dir_y ? 1.f : .707f) * player->move_speed * elapsed;
				player->position.y += dir_y * (dir_x * dir_y > 0 ? 1.1f : 1.0f) * (dir_x ^ dir_y ? 1.f : .707f) * player->move_speed * elapsed;
				
				player->overheat += elapsed * (player->buttons_pressed ? .5f : -.25f);

				if (player->position.x < 0.f) {
					player->position.x = (float)PPU466::ScreenWidth;
				}
				else if (player->position.x > (float)PPU466::ScreenWidth) {
					player->position.x = 0;
				}
				if (player->position.y < 0.f) {
					player->position.y = (float)PPU466::ScreenHeight;
				}
				else if (player->position.y > (float)PPU466::ScreenHeight) {
					player->position.y = 0;
				}

				if (player->overheat >= 1.0f) {
					player->can_move = false;
				}
				player->overheat = std::max(0.f, std::min(player->overheat, 1.0f));
			}
			else if (player->overheat > 0.f) {
				player->overheat -= elapsed * .2f;
				attack_speed_cooldown = 0.f;
			}
			else {
				player->can_move = true;
			}
		};

		p->on_death = [weak_player](float time) {
			std::shared_ptr< Entities::Player > player;
			if (!(player = weak_player.lock())) return true;

			if (time < .1f) {
				player->set_sprite("death_0");
			}
			else if (time < .18f) {
				player->set_sprite("death_1");
				return false;
			}
			else if (time < .23f) {
				player->set_sprite("death_2");
			}
			else if (time < .25f) {
				player->set_sprite("death_3");
			}
			
			return time > .26f;
		};
	}

    return weak_player;
}