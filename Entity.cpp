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

    std::shared_ptr< Sprite > s = it->second;
    for (size_t i = 0; i < s->sprite.size; i++) {
        reserved[i].index = s->sprite.tile_idxs[i];
        reserved[i].x = int8_t(s->sprite.offsets[i].x);
        reserved[i].y = int8_t(s->sprite.offsets[i].y);
        reserved[i].attributes = (reserved[i].attributes & ~0x7) | (s->palette_idxs[i] & 0x7);
    }

    active_sprite = std::vector < PPU466::Sprite >(reserved.begin(), reserved.begin() + s->sprite.size);
}