#include "Entity.hpp"

#include <iostream>

std::shared_ptr< Entities::Bullet > init_bullet(EntityPrefabs *table, std::vector< std::shared_ptr< Entity > > *into);
std::weak_ptr< Entities::Player > init_player(EntityPrefabs *table, std::vector< std::shared_ptr< Entity > > *into);