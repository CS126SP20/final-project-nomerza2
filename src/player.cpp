// Copyright (c) 2020 [Your Name]. All rights reserved.

#include <Box2D/Box2D.h>
#include <cinder/app/App.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>
#include <mylibrary/player.h>
#include <mylibrary/EffectiveDimensions.h>

using mylibrary::EffectiveDimensions;

namespace mylibrary {

Player::Player(b2World* world, float x_loc, float y_loc){
  //Dynamic now
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.fixedRotation = true;
  bodyDef.position.Set(x_loc, y_loc);
  body_ = world->CreateBody(&bodyDef);

  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(kPlayerWidth/2, kPlayerHeight/2);
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.density = 2.0f;
  fixtureDef.friction = 0.0f;
  body_->CreateFixture(&fixtureDef);

  //Foot Sensor for jumping
  b2PolygonShape sensor_box;
  sensor_box.SetAsBox(
      kPlayerWidth/2 - 0.1f, 0.2f, b2Vec2(0, -1), 0);
  //x is almost same as player, smaller to prevent jumping when leaning against wall.
  // y is much smaller center in y must equal the negative half-height of the player
  // !! 0.2f is the smallest hy that the player could still be able to jump normally.

  b2FixtureDef sensor_fixture_def;
  sensor_fixture_def.shape = &sensor_box;
  sensor_fixture_def.isSensor = true;
  b2Fixture* sensor_fixture = body_->CreateFixture(&sensor_fixture_def);
  sensor_fixture->SetUserData((void*)kFootSensorID);

  facing_right_ = true;

  ci::ImageSourceRef right_image_source =
      ci::loadImage(cinder::app::loadAsset(
          "robot_right.png"));
  ci::ImageSourceRef left_image_source =
      ci::loadImage(cinder::app::loadAsset(
          "robot_left.png"));

  right_image_ = ci::gl::Texture2d::create(right_image_source);
  left_image_ = ci::gl::Texture2d::create(left_image_source);

  moving_wall_contact_ = nullptr;
  relative_velocity_ = 0;
}

void Player::Draw() {
  b2Vec2 position = body_->GetPosition();

  int pixel_x = position.x*kPixelsPerMeter - kPlayerWidth*kPixelsPerMeter/2;
  int pixel_y = EffectiveDimensions::GetEffectiveHeight()
      - (position.y*kPixelsPerMeter + kPlayerHeight*kPixelsPerMeter/2);

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

void Player::UpdateVelocity() {
  //Adds the set velocity from keypress to the velocity of the object it is on for x
  //y velocity should remain the same
  if (moving_wall_contact_ != nullptr) {
    body_->SetLinearVelocity(b2Vec2((relative_velocity_ + moving_wall_contact_->GetLinearVelocity().x), body_->GetLinearVelocity().y));
  } else {
    body_->SetLinearVelocity(b2Vec2(relative_velocity_, body_->GetLinearVelocity().y));
  }
}

void Player::setFacingRight(bool facingRight) { facing_right_ = facingRight; }
bool Player::isFacingRight() const { return facing_right_; }
b2Body* Player::getBody() const { return body_; }
void Player::setMovingWallContact(b2Body* movingWallContact) {
  moving_wall_contact_ = movingWallContact;
}
void Player::setRelativeVelocity(float relativeVelocity) {
  relative_velocity_ = relativeVelocity;
}
}  // namespace mylibrary
