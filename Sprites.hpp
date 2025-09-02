#pragma once

#include "PPU466.hpp"
#include <array>
#include <unordered_map>
#include <string>
#include <glm/glm.hpp>

// A GenericSprite is a collection of Tile indices and offsets without attributes
struct GenericSprite {
    std::array < uint8_t, 4 > tile_idxs;
    std::array< glm::ivec2, 4 > offsets;
    uint8_t size;
};

struct Sprite {
    const GenericSprite &sprite;
    const std::array< uint8_t, 4 > palette_idxs;

    Sprite(const GenericSprite &sprite, const std::array< uint8_t, 4 > & palette_idxs) : sprite(sprite), palette_idxs(palette_idxs) {};
};

typedef std::unordered_map< std::string, GenericSprite > GenericSpritePrefabs;
