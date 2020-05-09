//
// Created by natha on 4/23/2020.
//

#include "mylibrary/Bullet.h"
#include <cinder/app/App.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>

namespace mylibrary {

Bullet::Bullet(b2World* world, b2Vec2 position, bool player_made, float bullet_radius) {
  entity_type_ = EntityType::type_bullet;
  player_made_ = player_made;
  bullet_radius_ = bullet_radius;
  super_ = false;

  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.fixedRotation = true; //probable won't matter, but it shouldn't be rotating anyway
  bodyDef.position.Set(position.x, position.y);
  bodyDef.bullet = true;
  body_ = world->CreateBody(&bodyDef);

  b2CircleShape circleShape;
  circleShape.m_radius = bullet_radius_;
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &circleShape;
  fixtureDef.density = 0.1f;
  fixtureDef.friction = 0.0f;
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
  } else if (super_) {
    color = ci::Color(0,0,1); // Blue
  } else {
    color = ci::Color(0,1,0); // Green
  }

  ci::gl::color(color);
  b2Vec2 box2d_position = body_->GetPosition();
  ci::vec2 cinder_position(box2d_position.x * kPixelsPerMeter,
      ci::app::getWindowHeight() - (box2d_position.y * kPixelsPerMeter));
  ci::gl::drawSolidCircle(cinder_position, bullet_radius_*kPixelsPerMeter);
}

bool Bullet::isPlayerMade() const { return player_made_; }
bool Bullet::isSuper() const { return super_; }
}


