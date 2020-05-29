//
// Created by natha on 5/3/2020.
//

#include "mylibrary/flying_enemy.h"
#include <cinder/app/App.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>
#include <mylibrary/EffectiveDimensions.h>

using mylibrary::EffectiveDimensions;

namespace mylibrary {

FlyingEnemy::FlyingEnemy(b2World* world, b2Vec2 position, bool is_facing_right) : Enemy(world, position, is_facing_right) {

  enemyType = flying_enemy;

  right_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("jetpackoffright.png")));
  left_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("jetpackoffleft.png")));
  right_flying_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("jetpackonright.png")));
  left_flying_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("jetpackonleft.png")));
}

void FlyingEnemy::Draw() {
  b2Vec2 position = body_->GetPosition();
  float vertical_velocity = body_->GetLinearVelocity().y;

  int pixel_x = position.x*kPixelsPerMeter - kEnemyWidth*kPixelsPerMeter/2;
  int pixel_y = EffectiveDimensions::GetEffectiveHeight() - (position.y*kPixelsPerMeter + kEnemyHeight*kPixelsPerMeter/2);

  ci::gl::Texture2dRef image_ref;
  if(facing_right_ && vertical_velocity > 0) {
    image_ref = right_flying_image_;
  } else if (!facing_right_ && vertical_velocity > 0) {
    image_ref = left_flying_image_;
  } else if (facing_right_) {
    image_ref = right_image_;
  } else {
    image_ref = left_image_;
  }

  // Necessary or the image will be tinted the color of the last drawn object.
  ci::Color reset(1,1,1);
  ci::gl::color(reset);
  ci::gl::draw(image_ref, ci::vec2(pixel_x, pixel_y));
}

void FlyingEnemy::Fly() {
  // Don't allow them to fly off screen
  if (body_->GetPosition().y * kPixelsPerMeter > ci::app::getWindowHeight() - (kEnemyHeight * kPixelsPerMeter)
    && body_->GetLinearVelocity().y > 0) {

    b2Vec2 lin_velocity(body_->GetLinearVelocity().x, 0);
    body_->SetLinearVelocity(lin_velocity);
    return;
  }

  // Gives 2% chance
  if (rand() % 100 > 1 || body_->GetLinearVelocity().y > 0) {
    return;
  }

  // impulse is from 10 to 30
  body_->ApplyLinearImpulse(b2Vec2(0, rand() % 20 + 8), body_->GetPosition());
}

}