#pragma once

#include "TileTable.hpp"

#include <fstream>

#include "PPU466.hpp"
#include "data_path.hpp"

void TileTable::import(std::string const &filename, std::array < PPU466::Tile, 256 > *to) {
    std::ifstream file(data_path(filename), std::ios::binary);
    
    if (!file) {
        throw std::runtime_error("Could not find " + filename);
    }

    // Using a fixed tile table, per class Discord
    file.read(reinterpret_cast< char * >(to->data()), sizeof(PPU466::Tile) * 256);
}