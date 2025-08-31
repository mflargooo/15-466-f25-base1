#pragma once

#include "TileTable.hpp"

#include <fstream>

#include "Load.hpp"
#include "read_write_chunk.hpp"
#include "PPU466.hpp"
#include "data_path.hpp"

void TileTable::import(std::string const &filename, std::array < PPU466::Tile, 256 > *to) {
    std::ifstream file(data_path(filename), std::ios::binary);
    
    if (!file) {
        throw std::runtime_error("Could not find " + filename);
    }

    file.read(reinterpret_cast< char * >(to->data()), sizeof(PPU466::Tile) * 256);
}