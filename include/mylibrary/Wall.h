//
// Created by natha on 4/30/2020.
//

#ifndef FINALPROJECT_WALL_H
#define FINALPROJECT_WALL_H

#include <Box2D/Box2D.h>
#include <cinder/gl/gl.h>
#include "entity.h"

namespace mylibrary {

class Wall {
 public:
  Wall(b2World* world, float x_loc, float y_loc, float half_width, float half_height, ci::Color color);
  void Draw();

 private:
  float half_width_;
  float half_height_;
  b2Body* body_;
  ci::Color color_;
};

}

#endif  // FINALPROJECT_WALL_H
