// Copyright (c) 2020 [Your Name]. All rights reserved.

#include "my_app.h"

#include <cinder/app/App.h>
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

using mylibrary::Player;
using mylibrary::Bullet;
using mylibrary::Entity;

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
  contactListener.myApp = this;
  jump_timer = 0;
  shooting_timer = 0;
}

void MyApp::setup() {
}

void MyApp::update() {
  for (unsigned int bullet_ID : bullets_to_destroy_) {
    DestroyBullet(bullet_ID);
  }
  bullets_to_destroy_.clear();

  // Destroys bodies that aren't allowed to be destroyed in collision callbacks.
  /*for (b2Body* body : to_destroy_) {
    world->DestroyBody(body);
  }*/

  //Simulate
  float timeStep = 1.0f / 60.0f;
  // Iterations affect how many calculations the program does. The values were recommended on Box2D.org
  int32 velocityIterations = 8;
  int32 positionIterations = 3;
  world->Step(timeStep, velocityIterations, positionIterations);

  //using the timer the opposite way with the timer increasing at every tep would remove the need for the conditional,
  //but the value could get very large and overflow if the program was left idling.
  if (jump_timer > 0) {
      jump_timer--;
  }

  if (shooting_timer > 0) {
    shooting_timer--;
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
  ci::Color wall_color(1,1,0); //Yellow
  ci::gl::color(wall_color);
  ci::gl::drawSolidRect(rect2);

  for (std::pair<unsigned int, Bullet> bullet_data : bullet_manager_) {
    bullet_data.second.Draw();
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

  } else if (event.getCode() == KeyEvent::KEY_SPACE && shooting_timer == 0) {
    shooting_timer = 25;

    b2Vec2 player_position = player_->getBody()->GetPosition();
    b2Vec2 spawn_location;
    //b2Vec2 bullet_impulse;
    b2Vec2 bullet_velocity;

    if (player_->isFacingRight()) {
      spawn_location = b2Vec2(player_position.x + (3*kPlayerWidth/4), player_position.y - (kPlayerHeight/5));// The kPlayerHeight/5 is to make it appear to spawn closer to the gun. this isn't the most clear solution prob.
      //bullet_impulse = b2Vec2(0.0133f, 0.0f);
      bullet_velocity = b2Vec2(6.0f, 0.0f);
    } else {
      spawn_location = b2Vec2(player_position.x - (3*kPlayerWidth/4), player_position.y - (kPlayerHeight/5));
      //bullet_impulse = b2Vec2(-0.0133f, 0.0f);
      bullet_velocity = b2Vec2(-6.0f, 0.0f);
    }

    Bullet bullet(world, spawn_location);
    //bullet.getBody()->ApplyLinearImpulse(bullet_impulse, spawn_location);
    bullet.getBody()->SetLinearVelocity(bullet_velocity);
    bullet_manager_.insert(std::pair<unsigned int, Bullet> (Entity::GetEntityID(), bullet));
  }
}

void MyApp::keyUp(cinder::app::KeyEvent event) {
  b2Body* body = player_->getBody();

  if (event.getCode() == KeyEvent::KEY_LEFT || event.getCode() == KeyEvent::KEY_RIGHT) {
      b2Vec2 velocity(0.0f, body->GetLinearVelocity().y);
      body->SetLinearVelocity(velocity);
  }
}

void MyApp::DestroyBullet(unsigned int bullet_ID) {
  Bullet bullet = bullet_manager_.at(bullet_ID);
  world->DestroyBody(bullet.getBody());
  bullet_manager_.erase(bullet_ID);
}

void MyApp::BulletCollision(b2Fixture* bullet, b2Fixture* other) {
  //Currently just destroys the bullet
  unsigned int bullet_ID = (unsigned int) bullet->GetUserData();
  bullets_to_destroy_.insert(bullet_ID);
  //bullets_to_destroy_.push_back(bullet_ID);
  //to_destroy_.push_back(bullet->GetBody());
  //bullet_manager_.erase(bullet_ID);
}

// Code for ContactListeners to determine if the user is able to jump (citation):
// https://www.iforce2d.net/b2dtut/jumpability
// Code has been modified to avoid repetition

void MyApp::ContactListener::BeginContact(b2Contact* contact) {
  b2Fixture* fixture_A = contact->GetFixtureA();
  b2Fixture* fixture_B = contact->GetFixtureB();
  //check if either fixture is the foot sensor
  /*(if ((int)fixture_A->GetUserData() == kFootSensorID
    || (int)fixture_B->GetUserData() == kFootSensorID) {*/
  if (fixture_A->IsSensor() || fixture_B->IsSensor()) { //TODO change or handle if new sensors are made.

      sensor_contacts++;
      return;
  }

  if (fixture_A->GetBody()->IsBullet()) {
    this->myApp->BulletCollision(fixture_A, fixture_B);
  }

  if (fixture_B->GetBody()->IsBullet()) {
    this->myApp->BulletCollision(fixture_B, fixture_A);
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
