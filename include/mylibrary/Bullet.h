//
// Created by natha on 4/23/2020.
//

#ifndef FINALPROJECT_BULLET_H
#define FINALPROJECT_BULLET_H

#include <Box2D/Box2D.h>

namespace mylibrary {

class Bullet {
 public:
  Bullet(b2World* world, b2Vec2 position);
  void Draw();
  b2Body* getBody() const;

 private:
  b2Body* body_;
};

}
#endif  // FINALPROJECT_BULLET_H
