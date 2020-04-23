//
// Created by natha on 4/23/2020.
//

#include "mylibrary/Bullet.h"
#include <cinder/app/App.h>  //TODO check if ok to use Cinder in source file?
#include <cinder/gl/draw.h>

const int kPixelsPerMeter = 90; //TODO This is both here and in player.h. Figure out how to not repeat constant definitions
const float kBulletRadius = 0.2f;

myLibrary::Bullet::Bullet(b2World* world, b2Vec2 position) {
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.fixedRotation = true; //probable won't matter, but it shouldn't be rotating anyway
  bodyDef.position.Set(position);
  bodyDef.bullet = true;
  body = world->CreateBody(&bodyDef);

  b2CircleShape circleShape;
  circleShape.m_radius = kBulletRadius;
  b2FixtureDef fixtureDef;
  fixtureDef.shape = circleShape;
  fixtureDef.density = 0.1f; //todo: decide how we want physics to affect bullet.
  fixtureDef.friction = 1.0f;
  body->CreateFixture(fixtureDef);
}

void myLibrary::Bullet::Draw() {
  ci::Color color(1,0,0); //Red
  b2Vec2 position = body->GetPosition();
  ci::gl::drawSolidCircle((position.x * kPixelsPerMeter, ci::app::getWindowHeight() - (position.y * kPixelsPerMeter)), kBulletRadius*kPixelsPerMeter);
}