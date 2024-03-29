//
// Created by natha on 4/25/2020.
//

#ifndef FINALPROJECT_ENEMY_H
#define FINALPROJECT_ENEMY_H

#include <mylibrary/entity.h>
#include <Box2D/Box2D.h>
#include <cinder/gl/Texture.h>
#include <mylibrary/Bullet.h>

// image is 84x105 pixels
// for better interaction, Box2D box will be 52x105, with 16 pixels taken from
// each side evenly so that way the center remains the same
const float kEnemyHeight = (105.0f/kPixelsPerMeter);
const float kEnemyWidth = 52.0f/kPixelsPerMeter;
const float kTotalMeterWidth = 84.0f/kPixelsPerMeter;

namespace mylibrary {

enum EnemyType{standard_enemy, flying_enemy, hunter};

class Enemy : public Entity {
 public:
  Enemy(b2World* world, b2Vec2 position, bool is_facing_right);
  virtual void Draw();

  // When the enemy shoots, this determines the location the bullet is created
  // To account for the location of enemy, direction it's facing, and height of gun
  b2Vec2 Calculate_Bullet_Spawn();

  virtual Bullet* Shoot(b2World* world, b2Vec2 player_pos);

  // Does the actual changing of linear velocity when turning around or touching moving walls.
  void UpdateVelocity();

  // Changes facing_right_, and reverses the direction of movement
  void TurnAround();

  // Checks if the enemy is within the bounds.
  // If it is, the body is "awoken" and changed from a static to dynamic body.
  // Returns true if the body has been awoken
  // bounds are in METERS, not pixels
  bool Activate(float left_bound, float right_bound);

  EnemyType getEnemyType() const;
  bool isFacingRight() const;
  bool isActive() const;
  void setMovingWallContact(b2Body* movingWallContact);

 protected:
  bool is_active_;
  bool facing_right_;
  EnemyType enemyType;
  float relative_velocity_;
  b2Body* moving_wall_contact_;
  ci::gl::Texture2dRef right_image_;
  ci::gl::Texture2dRef left_image_;
};

}

#endif  // FINALPROJECT_ENEMY_H
