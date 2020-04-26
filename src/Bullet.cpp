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

Bullet::Bullet(b2World* world, b2Vec2 position, bool player_made) {
  entity_type_ = EntityType::type_bullet;
  player_made_ = player_made;

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
  //fixtureDef.density = 0;
  fixtureDef.density = 1.0f; //todo: decide how we want physics to affect bullet.
  fixtureDef.friction = 1.0f;
  b2Fixture* fixture = body_->CreateFixture(&fixtureDef);

  fixture->SetUserData((void*)++Entity::entity_ID_); //Used for collision callback
  //Increments first, so then entity_ID_ = the ID of the new bullet afterwards

  //This makes gravity not affect bullets, so they shoot straight
  body_->SetGravityScale(0);
}

void Bullet::Draw() {
  ci::Color color;

  if (player_made_) {
    color = ci::Color(1,0,0); // Red
  } else {
    color = ci::Color(0,1,0); // Green
  }

  ci::gl::color(color);
  b2Vec2 box2d_position = body_->GetPosition();
  ci::vec2 cinder_position(box2d_position.x * kPixelsPerMeter, ci::app::getWindowHeight() - (box2d_position.y * kPixelsPerMeter));
  ci::gl::drawSolidCircle(cinder_position, kBulletRadius*kPixelsPerMeter);
}

bool Bullet::isPlayerMade() const { return player_made_; }
}


