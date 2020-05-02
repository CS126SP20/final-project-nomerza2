// Copyright (c) 2020 [Your Name]. All rights reserved.

#include <Box2D/Box2D.h>
#include <cinder/app/App.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>
#include <mylibrary/player.h>

namespace mylibrary {

Player::Player(b2World* world){
  //Dynamic now
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.fixedRotation = true;
  bodyDef.position.Set(59.0f, 4.0f);
  body_ = world->CreateBody(&bodyDef);

  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(kPlayerWidth/2, kPlayerHeight/2); //Using sizes of the image, and 90 pixels = 1m. need half-width and half-height here. width set to 1m.
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.density = 2.0f;
  fixtureDef.friction = 0.3f;
  body_->CreateFixture(&fixtureDef);

  //Foot Sensor for jumping
  b2PolygonShape sensor_box;
  sensor_box.SetAsBox(kPlayerWidth/2 - 0.1f, 0.2f, b2Vec2(0, -1), 0); //x is almost same as player, smaller to prevent jumping when leaning against wall. y is much smaller center in y must equal the negative half-height of the player
  b2FixtureDef sensor_fixture_def;
  sensor_fixture_def.shape = &sensor_box;
  sensor_fixture_def.isSensor = true;
  b2Fixture* sensor_fixture = body_->CreateFixture(&sensor_fixture_def);
  sensor_fixture->SetUserData((void*)kFootSensorID);

  facing_right_ = true;

  right_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("robot_right.png")));
  left_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("robot_left.png")));
}

void Player::Draw() {
  b2Vec2 position = body_->GetPosition();

  int pixel_x = position.x*kPixelsPerMeter - kPlayerWidth*kPixelsPerMeter/2;
  int pixel_y = ci::app::getWindowHeight() - (position.y*kPixelsPerMeter + kPlayerHeight*kPixelsPerMeter/2);

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

void Player::setFacingRight(bool facingRight) { facing_right_ = facingRight; }
bool Player::isFacingRight() const { return facing_right_; }
b2Body* Player::getBody() const { return body_; }
}  // namespace mylibrary
