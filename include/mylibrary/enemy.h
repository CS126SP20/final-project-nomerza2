//
// Created by natha on 4/25/2020.
//

#ifndef FINALPROJECT_ENEMY_H
#define FINALPROJECT_ENEMY_H

#include <mylibrary/entity.h>
#include <Box2D/Box2D.h>
#include <cinder/gl/Texture.h>

//TODO make different from player sizes?
const float kEnemyWidth = 52.0f/90.0f;
const float kEnemyHeight = (105.0f/90.0f); // TODO the 90 references kPixelsPerMeter Clarify all height/width defs that it is # of pixels / pixels per meter
// image is 84x105 pixels
// for better interaction, Box2D box will be 52x105, with 16 pixels taken from
// each side evenly so that way the center remains the same

const float kTotalMeterWidth = 84.0f/90.0f;

namespace mylibrary {

class Enemy : public Entity {
 public:
  Enemy(b2World* world, b2Vec2 position, bool is_facing_right);
  void Draw();
  b2Vec2 Calculate_Bullet_Spawn();

  // Changes facing_right_, and reverses the direction of movement
  void TurnAround();

  // Checks if the enemy is within the bounds.
  // If it is, the body is "awoken" and changed from a static to dynamic body.
  // Returns true if the body has been awoken
  // bounds are in METERS, not pixels
  bool Activate(float left_bound, float right_bound);

  bool isFacingRight() const;

 private:
  bool facing_right_;
  ci::gl::Texture2dRef right_image_;
  ci::gl::Texture2dRef left_image_;
};

}

#endif  // FINALPROJECT_ENEMY_H
