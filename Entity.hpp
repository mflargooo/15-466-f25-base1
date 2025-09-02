# pragma once

#include "Sprites.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

struct Collider {
    glm::highp_vec2 offset;
    float radius;
};

struct Entity {
    // within a Entity::Sprite, x and y of PPU466::Sprite are offsets 
    // from the Entity::position
    std::array < PPU466::Sprite, 4 > reserved;

    glm::highp_vec2 position = { 0.f, 0.f };

    Collider collider;

    float time_since_death = 0.f;
    bool dead;
    
    std::unordered_map < std::string, std::shared_ptr< Sprite > > spritesheet;
    std::shared_ptr< Sprite > active_sprite;
    std::vector < PPU466::Sprite > active_ppu_sprite;

    void assign_sprite(std::string state, std::shared_ptr < Sprite > sprite);
    void set_sprite(std::string state);
    virtual void update_sprite();

    std::function< void(float) > on_update = nullptr;
    std::function< bool(float) > on_death = nullptr;
    void update(float elapsed) { 
        if (on_update) {
            on_update(elapsed); 
        }
        update_sprite();
    }
    bool collides_with(std::shared_ptr< Entity > other);
    bool death(float elapsed) { 
        time_since_death += elapsed;
        if (on_death) {
            bool res = on_death(time_since_death);
            update_sprite();
            return res;
        }
        else return true;
    };
};

typedef std::unordered_map< std::string, std::shared_ptr< Entity > > EntityPrefabs;

namespace Entities {
    struct Player : Entity {
        float move_speed = 30.f;
        float attack_speed = 1.f;
        float overheat = 0.f;

        uint16_t buttons_pressed;
    };

    struct Bullet : Entity {
        float travel_speed = 50.f;
        glm::vec2 direction = glm::vec2(0.f);
    };

    struct Enemy : Entity {
        float move_speed = 20.f;
        float attack_speed = 3.f;
    };
}