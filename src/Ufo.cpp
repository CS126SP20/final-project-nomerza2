//
// Created by natha on 6/26/2020.
//

#include "mylibrary/Ufo.h"
#include <mylibrary/EffectiveDimensions.h>
#include <cinder/app/App.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>
#include <mylibrary/Bullet.h>
#include <mylibrary/SuperBullet.h>
#include <mylibrary/flying_enemy.h>
#include <mylibrary/Hunter.h>

//TODO redefinition from my_app.h
const int kStandardWidth = 1920;
const int kStandardHeight = 1080;
const int kBulletSpeed = 6;

using mylibrary::EffectiveDimensions;
using mylibrary::Bullet;
using mylibrary::SuperBullet;
using mylibrary::FlyingEnemy;
using mylibrary::Hunter;

Ufo::Ufo(b2World* world, b2Body* player_body) {
  world_ = world;

  b2BodyDef bodyDef;
  bodyDef.type = b2_kinematicBody;
  bodyDef.fixedRotation = true;
  bodyDef.position.Set(10, 10);
  body_ = world->CreateBody(&bodyDef);

  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(kUfoWidth/2, kUfoHeight/2);
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.density = 2.0f;
  fixtureDef.friction = 0.0f;
  b2Fixture* fixture = body_->CreateFixture(&fixtureDef);

  fixture->SetUserData((void*)kUfoID);

  image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("UFO.png")));
  body_->SetLinearVelocity(b2Vec2(3, 0));

  lives_ = 7;
  player_ = player_body;
  invincible_count = 0;
}

//TODO also increments invincible count down. Should prob make that a separate function
void Ufo::VelocityUpdate() {
  //Acts like a moving wall, with lower limit being always 0 and upper limit being kStandardWidth/kPPM
  if ((body_->GetPosition().x + (kUfoWidth / 2) >= kStandardWidth / kPixelsPerMeter)
    || (body_->GetPosition().x - (kUfoWidth / 2) <= 0)) {

      body_->SetLinearVelocity(-1 * body_->GetLinearVelocity());
    }

  if (invincible_count > 0) {
    invincible_count--;
  }
}

vector<pair<unsigned int, Entity*>> Ufo::Attack() {
  int choice = rand() % 100 + 1; //range 0-99
  vector<pair<unsigned int, Entity*>> result;

  float x = body_->GetPosition().x;
  float y = body_->GetPosition().y;

  if (choice < 35) { //3 down, one each side
    result.push_back(EntityMaker(true, false, false));
    result.push_back(EntityMaker(false, false, false, -1));
    result.push_back(EntityMaker(false, false, false));
    result.push_back(EntityMaker(false, false, false, 1));
    result.push_back(EntityMaker(true, true, false));
  } else if (choice < 50) { //5 down, one on each side
    result.push_back(EntityMaker(true, false, false));
    result.push_back(EntityMaker(false, false, false, -1.5));
    result.push_back(EntityMaker(false, false, false, -.75));
    result.push_back(EntityMaker(false, false, false));
    result.push_back(EntityMaker(false, false, false, .75));
    result.push_back(EntityMaker(false, false, false, 1.5));
    result.push_back(EntityMaker(true, true, false));
  } else if (choice < 60) { //2 on sides, super going down
    result.push_back(EntityMaker(true, false, false, .5));
    result.push_back(EntityMaker(true, false, false, -.5));
    result.push_back(EntityMaker(true, true, false, .5));
    result.push_back(EntityMaker(true, true, false, -.5));
    result.push_back(EntityMaker(false, false, true));
  } else if (choice < 70) { //supers on the sides, normal down
    result.push_back(EntityMaker(true, false, true));
    result.push_back(EntityMaker(true, true, true));
    result.push_back(EntityMaker(false, false, false, -1));
    result.push_back(EntityMaker(false, false, false));
    result.push_back(EntityMaker(false, false, false, 1));
  } else if (choice < 80) { //3 supers
    result.push_back(EntityMaker(true, false, true));
    result.push_back(EntityMaker(true, true, true));
    result.push_back(EntityMaker(false, false, true));
  } else if (choice < 85) { //4 side bullets, jetpack drop
    result.push_back(EntityMaker(true, false, false, .5));
    result.push_back(EntityMaker(true, false, false, -.5));
    result.push_back(EntityMaker(true, true, false, .5));
    result.push_back(EntityMaker(true, true, false, -.5));
    result.push_back(EntityMaker(false, bool(rand() % 2), false, 0, true)); //rand()%2 produces 0 or 1, => false or true
  } else if (choice < 90) { //supers on sides, jetpack drop
    result.push_back(EntityMaker(true, false, true));
    result.push_back(EntityMaker(true, true, true));
    result.push_back(EntityMaker(false, bool(rand() % 2), false, 0, true));
  } else if (choice < 95) { //4 side bullets, double jetpack
    result.push_back(EntityMaker(true, false, false, .5));
    result.push_back(EntityMaker(true, false, false, -.5));
    result.push_back(EntityMaker(true, true, false, .5));
    result.push_back(EntityMaker(true, true, false, -.5));
    result.push_back(EntityMaker(false, bool(rand() % 2), false, -0.7, true)); //rand()%2 produces 0 or 1, => false or true
    result.push_back(EntityMaker(false, bool(rand() % 2), false, 0.7, true)); //rand()%2 produces 0 or 1, => false or true
  } else { //hunter spawn
    result.push_back(EntityMaker(false, false, false, 0, true, true));
  }

  return result;
}

pair<unsigned int, Entity*> Ufo::EntityMaker(bool horizontal, bool right, bool super, float offset, bool isEnemy, bool isHunter) {
  b2Vec2 loc;
  b2Vec2 velocity;
  float x = body_->GetPosition().x;
  float y = body_->GetPosition().y;
  float gap;

  if (isEnemy) {
    gap = kEnemyHeight;
  } else {
    gap = .09;
  }

  if (horizontal) {
    if (right) {
      loc = b2Vec2(x + kUfoWidth/2 + gap, y + offset);
      velocity = b2Vec2(kBulletSpeed, 0);
    } else {
      loc = b2Vec2(x - kUfoWidth/2 - gap, y + offset);
      velocity = b2Vec2(-1 * kBulletSpeed, 0);
    }
  } else {
    loc = b2Vec2(x + offset, y - kUfoHeight/2 - gap);
    velocity = b2Vec2(0, -1 * kBulletSpeed);
  }

  Entity* entity;
  if (isEnemy && isHunter) {
    entity = new Hunter(world_, loc);
  } else if (isEnemy) {
    entity = new FlyingEnemy(world_, loc, right);
  } else if (super) {
    entity = new SuperBullet(world_, loc, player_->GetPosition());
  } else {
    entity = new Bullet(world_, loc, false);
    entity->getBody()->SetLinearVelocity(velocity);
  }

  unsigned int id = Entity::GetEntityID();
  pair<unsigned int, Entity*> data(id, entity);
  return data;
}

void Ufo::Draw() {
  b2Vec2 position = body_->GetPosition();

  int pixel_x = position.x*kPixelsPerMeter - kUfoWidth*kPixelsPerMeter/2;
  int pixel_y = EffectiveDimensions::GetEffectiveHeight() - (position.y*kPixelsPerMeter + kUfoHeight*kPixelsPerMeter/2);

  // If the UFO is invincible, tint it red
  if (invincible_count == 0) {
    ci::Color reset(1,1,1);
    ci::gl::color(reset);
  } else {
    ci::gl::color(ci::Color(1, 0, 0));
  }

  ci::gl::draw(image_, ci::vec2(pixel_x, pixel_y));


  //TODO this was copy/pasted from the PrintText function in main because I didn't feel like setting up a helper class that would let it be used here
  //Enemy Life counter
  std::string text = std::to_string(lives_);
  const cinder::ivec2 size = {100, 50};
  const cinder::vec2 loc(EffectiveDimensions::GetEffectiveWidth() - 50.0f,
      EffectiveDimensions::GetEffectiveHeight() - (kStandardHeight - 50.0f)); //Top-right Corner
  int font_size = 50;

  ci::Color color = ci::Color(1,0,0);
  cinder::gl::color(color);
  auto box = cinder::TextBox()
      .alignment(cinder::TextBox::CENTER)
      .font(cinder::Font("Arial", font_size))
      .size(size)
      .color(color)
      .backgroundColor(cinder::ColorA(0, 0, 0, 0))
      .text(text);

  const auto box_size = box.getSize();
  const cinder::vec2 locp = {loc.x - box_size.x / 2, loc.y - box_size.y / 2};
  const auto surface = box.render();
  const auto texture = cinder::gl::Texture::create(surface);
  cinder::gl::draw(texture, locp);
}

int Ufo::Shot() {
  if (invincible_count > 0) {
    return lives_;
  }

  invincible_count = 80; //Matches enemy reload time, effectively ensuring the ufo gets a new shot in before being hit

  lives_--;
  return lives_;
}