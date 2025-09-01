#include "Sprites.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

struct Entity {
    // within a Entity::Sprite, x and y of PPU466::Sprite are offsets 
    // from the Entity::position
    std::array < PPU466::Sprite, 4 > reserved;

    std::pair < int8_t, int8_t > position = { (int8_t) 0, (int8_t) 0 };
    
    std::unordered_map < std::string, std::shared_ptr < Sprite > > spritesheet;
    std::vector < PPU466::Sprite > active_sprite;

    void assign_sprite(std::string state, std::shared_ptr < Sprite > sprite);
    void set_sprite(std::string state);

    std::function< void(float) > on_update;
    void update(float elapsed) { if (on_update) on_update(elapsed); };

    Entity(std::function < void(float) > on_update) : on_update(on_update) {};
};

typedef std::unordered_map < std::string, std::shared_ptr< Entity > > EntityPrefabs;

namespace Entities {
    struct Player : Entity {
        float move_speed = 10.f;
        float attack_speed = 1.f;
        float overheat = 0.f;

        uint16_t buttons_pressed;
    };

    struct Bullet : Entity {
        float travel_speed = 1.f;
        glm::vec2 direction = { 0.f, 1.f };
    };

    struct Enemy : Entity {
        float move_speed = 10.f;
        float attack_speed = 10.f;
    };
}