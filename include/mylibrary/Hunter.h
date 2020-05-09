//
// Created by natha on 5/9/2020.
//

#ifndef FINALPROJECT_HUNTER_H
#define FINALPROJECT_HUNTER_H

#include <mylibrary/enemy.h>

namespace mylibrary {

class Hunter : Enemy {
  Hunter(b2World* world, b2Vec2 position);
  Bullet* Shoot(b2World* world, b2Vec2 player_pos);
};

}

#endif  // FINALPROJECT_HUNTER_H
