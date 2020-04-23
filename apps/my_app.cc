// Copyright (c) 2020 [Your Name]. All rights reserved.

#include "my_app.h"

#include <cinder/app/App.h>
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

using mylibrary::Player;
using mylibrary::Bullet;

int sensor_contacts = 0; //TODO extern definition here should it be fixed?

namespace myapp {

using cinder::app::KeyEvent;

MyApp::MyApp() {
  //All of this is copied straight from Box2D.org
  b2Vec2 gravity(0.0f, -10.0f);
  world = new b2World(gravity);

  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0.0f, -10.0f);
  b2Body* groundBody = world->CreateBody(&groundBodyDef);
  b2PolygonShape groundBox;
  groundBox.SetAsBox(50.0f, 10.0f);
  groundBody->CreateFixture(&groundBox, 0.0f);

    b2BodyDef centerBodyDef;
    centerBodyDef.position.Set(4.0f, 2.0f);
    centerBody = world->CreateBody(&centerBodyDef);
    b2PolygonShape centerBox;
    centerBox.SetAsBox(1.5f, 2.3f);
    centerBody->CreateFixture(&centerBox, 0.0f);

  player_ = new Player(world);

  //Set Up the Foot Sensor Listener
  world->SetContactListener(&contactListener);
  jump_timer = 0;
}

void MyApp::setup() {

}

void MyApp::update() {
  //Simulate
  float timeStep = 1.0f / 60.0f;
  int32 velocityIterations = 6;
  int32 positionIterations = 2;
  world->Step(timeStep, velocityIterations, positionIterations);

  //using the timer the opposite way with the timer increasing at every tep would remove the need for the conditional,
  //but the value could get very large and overflow if the program was left idling.
  if (jump_timer > 0) {
      jump_timer--;
  }
}

void MyApp::draw() {
  //https://github.com/asaeed/Box2DTest/blob/master/src/Particle.cpp

  b2Vec2 position = player_->getBody()->GetPosition();

  ci::gl::clear();
  int k = kPixelsPerMeter; //TODO get rid o this

  player_->Draw();

  b2Vec2 cposition = centerBody->GetPosition();
  ci::Rectf rect2(cposition.x*k - 1.5*k, getWindowHeight() - cposition.y*k - 2.3*k, cposition.x*k + 1.5*k, getWindowHeight() - cposition.y*k + 2.3*k);
  ci::Color wall_color(1,1,0);
  ci::gl::color(wall_color);
  ci::gl::drawSolidRect(rect2);

  for (Bullet bullet : bullet_manager_) {
    bullet.Draw();
  }
}

void MyApp::keyDown(KeyEvent event) {
  b2Body* body = player_->getBody();

  if (event.getCode() == KeyEvent::KEY_UP && sensor_contacts >= 1 && jump_timer == 0) {  // only jump if in contact with ground //TODO take event.getCode() out of conditional?
      b2Vec2 impulse_vector(0.0f, 25.0f);//Arbitrarily chosen value, looks good in testing.
      body->ApplyLinearImpulse(impulse_vector, body->GetPosition());
      jump_timer = 10; // NOTE this was arbitrarily chosen, change if necessary.

  } else if (event.getCode() == KeyEvent::KEY_RIGHT) {
      b2Vec2 velocity(5.0f, body->GetLinearVelocity().y); // Need to use previous y velocity, or trying to move side to side mid-air will cause player to suddenly fall
      // TODO take velocity changes into helper function?
      body->SetLinearVelocity(velocity);
      player_->setFacingRight(true);
  } else if (event.getCode() == KeyEvent::KEY_LEFT) {
      b2Vec2 velocity(-5.0f, body->GetLinearVelocity().y);
      body->SetLinearVelocity(velocity);
      player_->setFacingRight(false);

  } else if (event.getCode() == KeyEvent::KEY_SPACE) {
    b2Vec2 player_position = player_->getBody()->GetPosition();
    b2Vec2 spawn_location;
    b2Vec2 bullet_impulse;

    if (player_->isFacingRight()) {
      spawn_location = b2Vec2(player_position.x + (kPlayerWidth/2), player_position.y - (kPlayerHeight/5));// The kPlayerHeight/5 is to make it appear to spawn closer to the gun. this isn't the most clear solution prob.
      bullet_impulse = b2Vec2(0.01f, 0.0f);
    } else {
      spawn_location = b2Vec2(player_position.x - (kPlayerWidth/2), player_position.y - (kPlayerHeight/5));
      bullet_impulse = b2Vec2(-0.01f, 0.0f);
    }

    Bullet bullet(world, spawn_location);
    bullet.getBody()->ApplyLinearImpulse(bullet_impulse, spawn_location);
    bullet_manager_.push_back(bullet);
  }
}

void MyApp::keyUp(cinder::app::KeyEvent event) {
  b2Body* body = player_->getBody();

  if (event.getCode() == KeyEvent::KEY_LEFT || event.getCode() == KeyEvent::KEY_RIGHT) {
      b2Vec2 velocity(0.0f, body->GetLinearVelocity().y);
      body->SetLinearVelocity(velocity);
  }
}


// Code for ContactListeners to determine if the user is able to jump (citation):
// https://www.iforce2d.net/b2dtut/jumpability
// Code has been modified to avoid repetition

void MyApp::ContactListener::BeginContact(b2Contact* contact) {
    //check if either fixture is the foot sensor
    if ((int)contact->GetFixtureA()->GetUserData() == kFootSensorID
      || (int)contact->GetFixtureB()->GetUserData() == kFootSensorID) {
        sensor_contacts++;
    }
}

void MyApp::ContactListener::EndContact(b2Contact* contact) {
    //check if either fixture is the foot sensor
    if ((int)contact->GetFixtureA()->GetUserData() == kFootSensorID
      || (int)contact->GetFixtureB()->GetUserData() == kFootSensorID) {
        sensor_contacts--;
    }
}
}  // namespace myapp
