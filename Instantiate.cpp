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
				bullet->position.x = (float)PPU466::BackgroundWidth * 4.f;
			}
			else if (bullet->position.x > (float)PPU466::BackgroundWidth * 4.f) {
				bullet->position.x = 0;
			}
			if (bullet->position.y < 0.f) {
				bullet->position.y = (float)PPU466::BackgroundHeight * 4.f;
			}
			else if (bullet->position.y > (float)PPU466::BackgroundHeight * 4.f) {
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

					attack_speed_cooldown = player->attack_speed;
				}
			}
			else if (attack_speed_cooldown > 0.f) {
				attack_speed_cooldown -= elapsed;
			}

			player->position.x += dir_x * (dir_x * dir_y > 0 ? 1.1f : 1.0f) * (dir_x ^ dir_y ? 1.f : .707f) * player->move_speed * elapsed;
			player->position.y += dir_y * (dir_x * dir_y > 0 ? 1.1f : 1.0f) * (dir_x ^ dir_y ? 1.f : .707f) * player->move_speed * elapsed;

			// clamp player position to screen
			player->position.x = std::max(0.f, std::min(player->position.x, (float) PPU466::BackgroundWidth * 4.f - 16.f));
			player->position.y = std::max(0.f, std::min(player->position.y, (float) PPU466::BackgroundHeight * 4.f - 16.f));
		};
	}

    return weak_player;
}