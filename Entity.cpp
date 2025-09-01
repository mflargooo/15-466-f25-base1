#include "Entity.hpp"

#include <stdexcept>
#include <memory>

void Entity::assign_sprite(std::string state, std::shared_ptr< Sprite > sprite) {
    spritesheet[state] = sprite;
}

void Entity::set_sprite(std::string state) {
    auto it = spritesheet.find(state);
    if (it == spritesheet.end()) {
        throw std::runtime_error("State " + state + " is not defined");
    }

    active_sprite = it->second;
    for (size_t i = 0; i < active_sprite->sprite.size; i++) {
        reserved[i].index = active_sprite->sprite.tile_idxs[i];
        reserved[i].x = int8_t(active_sprite->sprite.offsets[i].x);
        reserved[i].y = int8_t(active_sprite->sprite.offsets[i].y);
        reserved[i].attributes = (reserved[i].attributes & ~0x7) | (active_sprite->palette_idxs[i] & 0x7);
    }

    active_ppu_sprite = std::vector < PPU466::Sprite >(reserved.begin(), reserved.begin() + active_sprite->sprite.size);
}

void Entity::update_sprite() {
        for (size_t i = 0; i < active_sprite->sprite.size; i++) {
            active_ppu_sprite[i].x = int8_t(position.x) - int8_t(active_sprite->sprite.offsets[i].x);
            active_ppu_sprite[i].y = int8_t(position.y) - int8_t(active_sprite->sprite.offsets[i].y);
        }
    };