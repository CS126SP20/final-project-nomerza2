//
// Created by natha on 5/8/2020.
//

#include <cmath>
#include "mylibrary/SuperBullet.h"

namespace mylibrary {

SuperBullet::SuperBullet(b2World* world, b2Vec2 position, b2Vec2 player_pos)  : Bullet(world, position, false, 0.1f){
  hits_left_ = 5;

  //Vector calculation, should travel towards player at with speed of set magnitude

  float x_comp = player_pos.x - position.x;
  float y_comp = player_pos.y - position.y;
  // reg enemy is 6, player is 6.7
  float magnitude = 6.5f;
  float angle = atan(y_comp / x_comp);
  if (x_comp < 0) {
    angle += 180;
  }
  b2Vec2 velocity(magnitude * cos(angle), magnitude * sin(angle));
  body_->SetLinearVelocity(velocity);

  super_ = true;
}

int SuperBullet::Rebound() { return --hits_left_; }

}