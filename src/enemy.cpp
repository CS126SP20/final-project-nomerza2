//
// Created by natha on 4/25/2020.
//

#include "mylibrary/enemy.h"
#include <cinder/app/App.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>

const int kPixelsPerMeter = 90;

namespace mylibrary {

Enemy::Enemy(b2World* world, b2Vec2 position, bool is_facing_right) {
  entity_type_ = EntityType::type_enemy;
  facing_right_ = is_facing_right;

  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.fixedRotation = true;
  bodyDef.position.Set(position.x, position.y);
  body_ = world->CreateBody(&bodyDef);

  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(kEnemyWidth/2, kEnemyHeight/2);
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.density = 2.0f;
  fixtureDef.friction = 0.3f;
  b2Fixture* fixture = body_->CreateFixture(&fixtureDef);

  fixture->SetUserData((void*)++Entity::entity_ID_); //Used for collision callback
  //Increments first, so then entity_ID_ = the ID of the new bullet afterwards

  right_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("badRobotRight.png")));
  left_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("badRobotLeft.png")));
}

void Enemy::Draw() {
  //TODO maybe get a util since I legit just copy-pasted from the player.cc file?
  b2Vec2 position = body_->GetPosition();

  int pixel_x = position.x*kPixelsPerMeter - kEnemyWidth*kPixelsPerMeter/2;
  int pixel_y = ci::app::getWindowHeight() - (position.y*kPixelsPerMeter + kEnemyHeight*kPixelsPerMeter/2);

  ci::gl::Texture2dRef image_ref;
  if(facing_right_) {
    image_ref = right_image_;
  } else {
    image_ref = left_image_;
  }

  ci::Color reset(1,1,1); //This is necessary or the image will be tinted the color of the last drawn object.
  ci::gl::color(reset);
  ci::gl::draw(image_ref, ci::vec2(pixel_x, pixel_y));
}

b2Vec2 Enemy::Calculate_Bullet_Spawn() {
  b2Vec2 position = body_->GetPosition();

  // Determined by counting pixels on the image
  float percentage_gun_height = 76/105;
  float y = percentage_gun_height * kEnemyHeight + position.y;
  float x;

  if (facing_right_) {
    x = position.x + (kEnemyWidth * 0.62f); //slightly over half, to ensure it doesn't collide with enemy itself
  } else {
    x = position.x - (kEnemyWidth * 0.62f);
  }

  return b2Vec2(x, y);
}
bool Enemy::isFacingRight() const { return facing_right_; }
}