//
// Created by natha on 6/26/2020.
//

#ifndef FINALPROJECT_UFO_H
#define FINALPROJECT_UFO_H

#include <mylibrary/entity.h>
#include <Box2D/Box2D.h>
#include <cinder/gl/Texture.h>

using std::vector;
using std::pair;
using mylibrary::Entity;

const int kUfoHeight = 180/kPixelsPerMeter;
const int kUfoWidth = 230/kPixelsPerMeter; // effective width for hitbox -- actual image is 268

class Ufo {
 public:
  Ufo(b2World* world);
  void VelocityUpdate();

  // Each pair is the Entity Data that will be used for entity manager or asleep enemies, as needed
  vector<pair<unsigned int, Entity*>> Attack();
  void Draw();

 private:
  b2World* world_;
  b2Body* body_;
  ci::gl::Texture2dRef image_;
  int lives_;

};

#endif  // FINALPROJECT_UFO_H
