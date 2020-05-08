// Copyright (c) 2020 [Your Name]. All rights reserved.
#ifndef FINALPROJECT_MYLIBRARY_EXAMPLE_H_
#define FINALPROJECT_MYLIBRARY_EXAMPLE_H_

#include <Box2D/Box2D.h>
#include <cinder/gl/Texture.h>
#include "entity.h"

// These sizes are in meters, using the pixel sizes in image divided by PPM
const float kPlayerWidth = 90.0f/kPixelsPerMeter;
const float kPlayerHeight = (105.0f/kPixelsPerMeter);

// Meaningless value, just needs to be unique.
// Negative to insure there is never conflict with EntityID system
const int kFootSensorID = -12345;

namespace mylibrary {

class Player {
 public:
  Player(b2World* world, float x_loc, float y_loc);
  void Draw();
  void setFacingRight(bool facingRight);
  b2Body* getBody() const;
  bool isFacingRight() const;

 private:
  b2Body* body_;
  bool facing_right_;
  ci::gl::Texture2dRef right_image_;
  ci::gl::Texture2dRef left_image_;

};

}  // namespace mylibrary


#endif // FINALPROJECT_MYLIBRARY_EXAMPLE_H_
