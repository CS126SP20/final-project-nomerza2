//
// Created by natha on 4/23/2020.
//

#ifndef FINALPROJECT_BULLET_H
#define FINALPROJECT_BULLET_H

#include <Box2D/Box2D.h>

//const int kBulletID = 55555; //Value unimportant, just needs to be unique

namespace mylibrary {

class Bullet {
 public:
  Bullet(b2World* world, b2Vec2 position);
  void Draw();
  b2Body* getBody() const;
  static unsigned int getBulletId();

 private:
  b2Body* body_;

  //Maintains a count for bullet IDs. Increments whenever a new one is made,
  //so it will always be unique.
  static unsigned int bullet_ID_;
};
}
#endif  // FINALPROJECT_BULLET_H
