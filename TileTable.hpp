#pragma once

#include <fstream>

#include "Load.hpp"
#include "read_write_chunk.hpp"
#include "PPU466.hpp"
#include "data_path.hpp"

namespace TileTable {
    void import(std::string const &filename, std::array < PPU466::Tile, 256 > *to);
}