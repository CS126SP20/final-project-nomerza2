// Copyright (c) 2020 [Your Name]. All rights reserved.

#include "my_app.h"

#include <cinder/app/App.h>
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

int sensor_contacts = 0; //TODO extern definition here should it be fixed?

const int kPixelsPerMeter = 90;
const float kPlayerWidth = 1.0f;
const float kPlayerHeight = (105.0f/90.0f);

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

  //Dynamic now
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.fixedRotation = true;
  bodyDef.position.Set(0.0f, 4.0f);
  body = world->CreateBody(&bodyDef);

  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(kPlayerWidth/2, kPlayerHeight/2); //Using sizes of the image, and 90 pixels = 1m. need half-width and half-height here. width set to 1m.
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.density = 2.0f;
  fixtureDef.friction = 0.3f;
  body->CreateFixture(&fixtureDef);

  //Foot Sensor for jumping
  b2PolygonShape sensor_box;
  sensor_box.SetAsBox(kPlayerWidth/2 - 0.1f, 0.2f, b2Vec2(0, -1), 0); //x is almost same as player, smaller to prevent jumping when leaning against wall. y is much smaller center in y must equal the negative half-height of the player
  b2FixtureDef sensor_fixture_def;
  sensor_fixture_def.shape = &sensor_box;
  sensor_fixture_def.isSensor = true;
  b2Fixture* sensor_fixture = body->CreateFixture(&sensor_fixture_def);
  sensor_fixture->SetUserData((void*)3); //Not sure what this syntax does, but gives it tag of "3". Maybe use constant instead?

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

  b2Vec2 position = body->GetPosition();

  ci::gl::clear();
  int k = 90; //kPixelsPerMeter

  ci::gl::Texture2dRef texture2D = ci::gl::Texture2d::create(ci::loadImage(loadAsset("robot_right.png")));
  ci::gl::draw(texture2D, ci::vec2(position.x*k - kPlayerWidth*kPixelsPerMeter/2, getWindowHeight() - (position.y*k + kPlayerHeight*kPixelsPerMeter/2)));

  b2Vec2 cposition = centerBody->GetPosition();
  ci::Rectf rect2(cposition.x*k - 1.5*k, getWindowHeight() - cposition.y*k - 2.3*k, cposition.x*k + 1.5*k, getWindowHeight() - cposition.y*k + 2.3*k);
  ci::Color wall_color(1,1,0);
  ci::gl::color(wall_color);
  ci::gl::drawSolidRect(rect2);

}

void MyApp::keyDown(KeyEvent event) {
    if (event.getCode() == KeyEvent::KEY_UP && sensor_contacts >= 1 && jump_timer == 0) {  // only jump if in contact with ground //TODO take event.getCode() out of conditional?
        b2Vec2 impulse_vector(0.0f, 25.0f);//Arbitrarily chosen value, looks good in testing.
        body->ApplyLinearImpulse(impulse_vector, body->GetPosition());
        jump_timer = 10; // NOTE this was arbitrarily chosen, change if necessary.

    } else if (event.getCode() == KeyEvent::KEY_RIGHT) {
        b2Vec2 velocity(5.0f, body->GetLinearVelocity().y); // Need to use previous y velocity, or trying to move side to side mid-air will cause player to suddenly fall
        // TODO take velocity changes into helper function?
        body->SetLinearVelocity(velocity);
    } else if (event.getCode() == KeyEvent::KEY_LEFT) {
        b2Vec2 velocity(-5.0f, body->GetLinearVelocity().y);
        body->SetLinearVelocity(velocity);
    }
}

void MyApp::keyUp(cinder::app::KeyEvent event) {
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
    if ((int)contact->GetFixtureA()->GetUserData() == 3 || (int)contact->GetFixtureB()->GetUserData() == 3) {
        sensor_contacts++;
    }
}

void MyApp::ContactListener::EndContact(b2Contact* contact) {
    //check if either fixture is the foot sensor
    if ((int)contact->GetFixtureA()->GetUserData() == 3 || (int)contact->GetFixtureB()->GetUserData() == 3) {
        sensor_contacts--;
    }
}
}  // namespace myapp
