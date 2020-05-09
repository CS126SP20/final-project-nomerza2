//
// Created by natha on 5/8/2020.
//

#ifndef FINALPROJECT_SUPERBULLET_H
#define FINALPROJECT_SUPERBULLET_H

#include <mylibrary/Bullet.h>

namespace mylibrary {

class SuperBullet : public Bullet {
 public:
  // Reg bullet doesn't set own start velocity, this does
  SuperBullet(b2World* world, b2Vec2 position, b2Vec2 player_pos);
  int Rebound();

 private:
  int hits_left_;
};

}

#endif  // FINALPROJECT_SUPERBULLET_H
