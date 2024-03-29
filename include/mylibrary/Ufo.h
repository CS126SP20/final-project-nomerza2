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
const int kUfoID = -128745; //Value unimportant, must be unique from other IDs (and therefore negative)

class Ufo {
 public:
  Ufo(b2World* world, b2Body* player_body);
  void VelocityUpdate();

  // Each pair is the Entity Data that will be used for entity manager or asleep enemies, as needed
  vector<pair<unsigned int, Entity*>> Attack();
  void Draw();
  int Shot();

 private:
  // if horizontal is false, the bullet will go down. the second parameter can be anything.
  // if isEnemy is true and isHunter is false, will generate a flying enemy
  pair<unsigned int, Entity*> EntityMaker(bool horizontal, bool right, bool super, float offset = 0, bool isEnemy = false, bool isHunter = false);

  b2World* world_;
  b2Body* body_;
  b2Body* player_;
  ci::gl::Texture2dRef image_;
  int lives_;
  int invincible_count;

};

#endif  // FINALPROJECT_UFO_H
