//
// Created by natha on 4/23/2020.
//

#ifndef FINALPROJECT_BULLET_H
#define FINALPROJECT_BULLET_H

#include <Box2D/Box2D.h>

namespace myLibrary {

class Bullet {
 public:
  Bullet(b2World* world, b2Vec2 position);
  void Draw();

 private:
  b2Body* body;
};

}
#endif  // FINALPROJECT_BULLET_H
