//
// Created by natha on 5/8/2020.
//

#include <cmath>
#include "mylibrary/SuperBullet.h"

//https://stackoverflow.com/questions/1727881/how-to-use-the-pi-constant-in-c
// two lines used to get pi constant
#define _USE_MATH_DEFINES
#include <math.h>

namespace mylibrary {

SuperBullet::SuperBullet(b2World* world, b2Vec2 position, b2Vec2 player_pos)  : Bullet(world, position, false, 0.1f){
  hits_left_ = 5;
  super_ = true;

  // This makes the ball bouncy
  body_->GetFixtureList()->SetRestitution(1);

  //Vector calculation, should travel towards player at with speed of set magnitude

  float x_comp = player_pos.x - position.x;
  float y_comp = player_pos.y - position.y;
  // reg enemy is 6, player is 7.1
  float magnitude = 6.5f;
  float angle = atan(y_comp / x_comp);
  if (x_comp < 0) {
    angle += M_PI;
  }
  b2Vec2 velocity(magnitude * cos(angle), magnitude * sin(angle));
  body_->SetLinearVelocity(velocity);

}

int SuperBullet::Rebound() { return --hits_left_; }

}