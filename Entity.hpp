#include "PPU466.hpp"
#include <vector>
#include <map>
#include <string>

struct Entity {
    private:
        uint8_t max_sprite_tiles = 0;

    public:
        std::string name;

        struct Sprite {
            std::vector< uint8_t > tile_idxs;
            std::vector< std::pair< uint8_t, uint8_t > > tile_offsets;
        };

        std::map < std::string, Sprite > sprites;
        std::vector < PPU466::Sprite& > ppu_sprite_ref;
        
        Entity(const std::string& name);
        
        void add_sprite(std::string state, Sprite sprite);
        void change_sprite(std::string state);
        void reserve_sprite_ref(std::array< PPU466::Sprite, 64 >::iterator& ref);
};