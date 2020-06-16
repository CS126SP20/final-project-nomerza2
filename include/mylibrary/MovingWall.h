//
// Created by natha on 6/5/2020.
//

#ifndef FINALPROJECT_MOVINGWALL_H
#define FINALPROJECT_MOVINGWALL_H

#include <Box2D/Box2D.h>
#include <cinder/gl/gl.h>
#include <mylibrary/enemy.h>

namespace mylibrary {

const int kMovingWallID = -87123; // ID Values doesn't matter as long as unique, and negative to provide conflicts with entity IDs
const int kSpikeID = -98765;
const float kSpikeSize = 0.2f;
const ci::Color kSpikeColor = ci::Color(0.87f, 0.87f, 0.87f);

class MovingWall {
 public:
  //lower_limit if the furthest point the left/bottom edge of the spikes will touch
  //upper_limit is the furthest point the right/top edge of the spikes will touch
  //half_width and half_height do not include spike length
  //sign of velocity should match the initial direction of movement
  MovingWall(b2World* world, float x_loc, float y_loc, float half_width,
  float half_height, float lower_limit, float upper_limit, float velocity,
  bool moves_vertically, ci::Color color);

  //Overloaded constructor, to allow activator to be set
  MovingWall(b2World* world, float x_loc, float y_loc, float half_width,
             float half_height, float lower_limit, float upper_limit, float velocity,
             bool moves_vertically, ci::Color color, Enemy* activator);

  // Called to check if wall has hit limit, reverses velocity if it has
  void VelocityUpdate();
  void Draw();

 private:
  float half_width_;
  float half_height_;
  float lower_limit_;
  float upper_limit_;
  float initial_velocity_;
  bool moves_vertically_;
  b2Body* body_;
  ci::Color color_;

  Enemy* activator_; //If an activator is set, the wall will not move until this enemy is activated

};

}

#endif  // FINALPROJECT_MOVINGWALL_H
