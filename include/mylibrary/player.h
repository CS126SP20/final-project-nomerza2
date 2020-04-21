// Copyright (c) 2020 [Your Name]. All rights reserved.
#ifndef FINALPROJECT_MYLIBRARY_EXAMPLE_H_
#define FINALPROJECT_MYLIBRARY_EXAMPLE_H_

#include <Box2D/Box2D.h>
#include <cinder/gl/Texture.h>

const int kPixelsPerMeter = 90;
const float kPlayerWidth = 1.0f;
const float kPlayerHeight = (105.0f/90.0f);
const int kFootSensorID = 12345; // Meaningless value, just needs to be unique.

namespace mylibrary {

class Player {
 public:
  Player(b2World* world);
  void Draw();
  void setFacingRight(bool facingRight);
  b2Body* getBody() const;

 private:
  //b2World* world_;
  b2Body* body_;
  bool facing_right_;
  ci::gl::Texture2dRef right_image_;
  ci::gl::Texture2dRef left_image_;

};

}  // namespace mylibrary


#endif // FINALPROJECT_MYLIBRARY_EXAMPLE_H_
