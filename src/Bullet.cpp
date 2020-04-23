//
// Created by natha on 4/23/2020.
//

#include "mylibrary/Bullet.h"
#include <cinder/app/App.h>  //TODO check if ok to use Cinder in source file?
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>

const int kPixelsPerMeter = 90; //TODO This is both here and in player.h. Figure out how to not repeat constant definitions
const float kBulletRadius = 0.05f;

namespace mylibrary {
Bullet::Bullet(b2World* world, b2Vec2 position) {
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.fixedRotation = true; //probable won't matter, but it shouldn't be rotating anyway
  bodyDef.position.Set(position.x, position.y);
  bodyDef.bullet = true;
  body_ = world->CreateBody(&bodyDef);

  b2CircleShape circleShape;
  circleShape.m_radius = kBulletRadius;
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &circleShape;
  fixtureDef.density = 0.1f; //todo: decide how we want physics to affect bullet.
  fixtureDef.friction = 1.0f;
  body_->CreateFixture(&fixtureDef);
}

void Bullet::Draw() {
  ci::Color color(1,0,0); //Red
  ci::gl::color(color);
  b2Vec2 box2d_position = body_->GetPosition();
  ci::vec2 cinder_position(box2d_position.x * kPixelsPerMeter, ci::app::getWindowHeight() - (box2d_position.y * kPixelsPerMeter));
  ci::gl::drawSolidCircle(cinder_position, kBulletRadius*kPixelsPerMeter);
}

b2Body* Bullet::getBody() const { return body_; }
}


