//
// Created by natha on 4/25/2020.
//

#include "mylibrary/enemy.h"

#include <cinder/app/App.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>
#include <mylibrary/EffectiveDimensions.h>

using mylibrary::EffectiveDimensions;

namespace mylibrary {

Enemy::Enemy(b2World* world, b2Vec2 position, bool is_facing_right) {
  entity_type_ = EntityType::type_enemy;
  facing_right_ = is_facing_right;
  is_active_ = false;
  enemyType = standard_enemy;

  b2BodyDef bodyDef;
  bodyDef.type = b2_staticBody;
  bodyDef.fixedRotation = true;
  bodyDef.position.Set(position.x, position.y);
  body_ = world->CreateBody(&bodyDef);

  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(kEnemyWidth/2, kEnemyHeight/2);
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.density = 2.0f;
  fixtureDef.friction = 0.0f;
  b2Fixture* fixture = body_->CreateFixture(&fixtureDef);

  fixture->SetUserData((void*)++Entity::entity_ID_); //Used for collision callback
  //Increments first, so then entity_ID_ = the ID of the new bullet afterwards

  body_->SetUserData((void*)Entity::entity_ID_); // Same ID
  // Used for death by falling where the fixture is unreachable

  right_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("badRobotRight.png")));
  left_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("badRobotLeft.png")));
  moving_wall_contact_ = nullptr;
  relative_velocity_ = 0;
}

void Enemy::Draw() {
  b2Vec2 position = body_->GetPosition();

  int pixel_x = position.x*kPixelsPerMeter - kEnemyWidth*kPixelsPerMeter/2;
  int pixel_y = EffectiveDimensions::GetEffectiveHeight() - (position.y*kPixelsPerMeter + kEnemyHeight*kPixelsPerMeter/2);

  ci::gl::Texture2dRef image_ref;
  if(facing_right_) {
    image_ref = right_image_;
  } else {
    image_ref = left_image_;
  }

  // Necessary or the image will be tinted the color of the last drawn object.
  ci::Color reset(1,1,1);
  ci::gl::color(reset);
  ci::gl::draw(image_ref, ci::vec2(pixel_x, pixel_y));
}

b2Vec2 Enemy::Calculate_Bullet_Spawn() {
  b2Vec2 position = body_->GetPosition();

  // Determined by measuring pixels on the image
  float percentage_gun_height = 76.0f/105.0f;
  // position.y is center, -0.5 height = bottom, + gun-height percent * enemyheight = gun position
  float y = (percentage_gun_height - 0.5f) * kEnemyHeight + position.y;
  float x;

  if (facing_right_) {
    x = position.x + (kTotalMeterWidth * 0.5f); //Uses the total width, so it references the image rather than the internal Box2D box
  } else {
    x = position.x - (kTotalMeterWidth * 0.5f);
  }

  return b2Vec2(x, y);
}

Bullet* Enemy::Shoot(b2World* world, b2Vec2 player_pos) {
  b2Vec2 bullet_velocity;
  if (facing_right_) {
    bullet_velocity = b2Vec2(6.0f, 0.0f);
  } else {
    bullet_velocity = b2Vec2(-6.0f, 0.0f);
  }

  Bullet* bullet = new Bullet(world, Calculate_Bullet_Spawn(), false);
  bullet->getBody()->SetLinearVelocity(bullet_velocity);
  return bullet;
}

void Enemy::UpdateVelocity() {
  //Adds the set velocity from keypress to the velocity of the object it is on for x
  //y velocity should remain the same
  if (moving_wall_contact_ != nullptr) {
    body_->SetLinearVelocity(b2Vec2((relative_velocity_ + moving_wall_contact_->GetLinearVelocity().x), body_->GetLinearVelocity().y));
  } else {
    body_->SetLinearVelocity(b2Vec2(relative_velocity_, body_->GetLinearVelocity().y));
  }
}

void Enemy::TurnAround() {
  facing_right_ = !facing_right_;

  if (facing_right_) {
    relative_velocity_ = 2;
  } else {
    relative_velocity_ = -2;
  }

  UpdateVelocity();
}

bool Enemy::Activate(float left_bound, float right_bound) {
  float x_pos = body_->GetPosition().x;

  if (left_bound <= x_pos && x_pos <= right_bound) {
    body_->SetType(b2_dynamicBody);
    body_->SetAwake(true);
    is_active_ = true;

    if (enemyType != hunter) {
      if (facing_right_) {
        relative_velocity_ = 2;
      } else {
        relative_velocity_ = -2;
      }
    }

    UpdateVelocity();
    return true;
  }

  return false;
}

bool Enemy::isFacingRight() const { return facing_right_; }
bool Enemy::isActive() const { return is_active_; }
EnemyType Enemy::getEnemyType() const { return enemyType; }
void Enemy::setMovingWallContact(b2Body* movingWallContact) {
  moving_wall_contact_ = movingWallContact;
}
}