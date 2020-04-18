// Copyright (c) 2020 [Your Name]. All rights reserved.

#include "my_app.h"

#include <cinder/app/App.h>
#include "cinder/gl/gl.h"


namespace myapp {

using cinder::app::KeyEvent;

MyApp::MyApp() {
  //All of this is copied straight from Box2D.org
  b2Vec2 gravity(0.0f, -10.0f);
  world = new b2World(gravity);
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0.0f, -10.0f);
  b2Body* groundBody = world->CreateBody(&groundBodyDef);
  //b2Body* groundBody = world.CreateBody(&groundBodyDef);
  b2PolygonShape groundBox;
  groundBox.SetAsBox(50.0f, 10.0f);
  groundBody->CreateFixture(&groundBox, 0.0f);

  //Dynamic now
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.position.Set(0.0f, 4.0f);
  body = world->CreateBody(&bodyDef);

  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(1.0f, 1.0f);
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.density = 1.0f;
  fixtureDef.friction = 0.3f;
  body->CreateFixture(&fixtureDef);

  //world_point = &world;
}

void MyApp::setup() {
}

void MyApp::update() {
  //Simulate
  float timeStep = 1.0f / 60.0f;
  int32 velocityIterations = 6;
  int32 positionIterations = 2;
  world->Step(timeStep, velocityIterations, positionIterations);
}

void MyApp::draw() {
    //https://github.com/asaeed/Box2DTest/blob/master/src/Particle.cpp
    ci::Color color(1, 0, 0);
    b2Vec2 position = body->GetPosition();
    float angle = body->GetAngle();

    ci::gl::clear();
    ci::gl::color(color);
    ci::Rectf rect(position.x*50, position.y*50, position.x*50 + 50, position.y*50 + 50);
    ci::gl::drawSolidRect(rect);
}

void MyApp::keyDown(KeyEvent event) { }

}  // namespace myapp
