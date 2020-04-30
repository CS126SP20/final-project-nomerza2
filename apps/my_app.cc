// Copyright (c) 2020 [Your Name]. All rights reserved.

#include "my_app.h"

#include <cinder/app/App.h>
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include <cinder/Text.h>
#include <cinder/Font.h>
#include <cinder/app/Window.h>

using mylibrary::Player;
using mylibrary::Bullet;
using mylibrary::Entity;
using mylibrary::Enemy;
using mylibrary::EntityType;
using mylibrary::Wall;

int sensor_contacts = 0; //TODO extern definition here should it be fixed?

namespace myapp {

using cinder::app::KeyEvent;

MyApp::MyApp() {
  //All of this is copied straight from Box2D.org
  b2Vec2 gravity(0.0f, -10.0f);
  world = new b2World(gravity);

  player_ = new Player(world);

  //Set Up the Foot Sensor Listener
  world->SetContactListener(&contactListener);
  contactListener.myApp = this;

  lives_ = 3;
  jump_timer = 0;
  shooting_timer = 0;
  enemy_shooting_timer_ = 44;
  scope_x_ = 0;
  scope_y_ = 0;
}

void MyApp::setup() {
  cinder::app::WindowRef windowRef = this->getWindow();
  windowRef->setFullScreen();

  Wall* wall = new Wall(world, 0, -9.8f, 27.0f, 10.0f, ci::Color(0,0,1));
  walls_.push_back(wall);
  wall = new Wall(world, 0, 0, 1.0f, 1.5f, ci::Color(1,1,0));
  walls_.push_back(wall);
  wall = new Wall(world, 8.0f, 2.0f, 1.5f, 2.3f, ci::Color(1,1,0));
  walls_.push_back(wall);

  Enemy* enemy = new Enemy(world, b2Vec2(5.0f, 5.0f), true);
  std::pair<unsigned int, Enemy*> enemy_data(Entity::GetEntityID(), enemy);
  entity_manager_.insert(enemy_data);
  asleep_enemies_.insert(enemy_data);

  Enemy* enemy_2 = new Enemy(world, b2Vec2(3.0f, 5.0f), true);
  std::pair<unsigned int, Enemy*> enemy_data_2(Entity::GetEntityID(), enemy_2);
  entity_manager_.insert(enemy_data_2);
  asleep_enemies_.insert(enemy_data_2);

  Enemy* far_enemy = new Enemy(world, b2Vec2((getWindowWidth() / kPixelsPerMeter) + 2.0f, 5.0f), true);
  std::pair<unsigned int, Enemy*> far_enemy_data(Entity::GetEntityID(), far_enemy);
  entity_manager_.insert(far_enemy_data);
  asleep_enemies_.insert(far_enemy_data);
}

void MyApp::update() {
  if (lives_ <= 0) {
    return;
  }

  for (unsigned int bullet_ID : entities_to_destroy_) {
    DestroyEntity(bullet_ID);
  }
  entities_to_destroy_.clear();

  //If an enemy is within the window, it will be Activated
  // Next 15 lines copied (with modifications) from:
  // https://www.techiedelight.com/remove-entries-map-iterating-cpp/
  // This method is necessary for iterating through map while erasing
  // elements within map, since erasing invalidates the iterator
  auto it = asleep_enemies_.cbegin();
  while (it != asleep_enemies_.cend())
  {
    unsigned int id = it->first;
    Enemy* enemy = it->second;

    if ((enemy->Activate((scope_x_ / kPixelsPerMeter), ((scope_x_ + getWindowWidth()) / kPixelsPerMeter))))
    {
      enemy_shooters_.insert(std::pair< unsigned int, Enemy*> (id, enemy));
      // supported in C++11
      it = asleep_enemies_.erase(it);
    }
    else {
      ++it;
    }
  }

  //Simulate
  float timeStep = 1.0f / 60.0f;
  // Iterations affect how many calculations the program does. The values were recommended on Box2D.org
  int32 velocityIterations = 8;
  int32 positionIterations = 3;
  world->Step(timeStep, velocityIterations, positionIterations);

  //using the timer the opposite way with the timer increasing at every step would remove the need for the conditional,
  //but the value could get very large and overflow if the program was left idling.
  if (jump_timer > 0) {
      jump_timer--;
  }

  if (shooting_timer > 0) {
    shooting_timer--;
  }

  //Death by falling
  if (player_->getBody()->GetPosition().y < 0) {
    lives_ = 0;
  }

  if (enemy_shooting_timer_ > 0) {
    enemy_shooting_timer_--;
  } else {
    enemy_shooting_timer_ = 80;

    for (std::pair<unsigned int, Enemy*> enemy_data : enemy_shooters_) {
      Enemy* enemy = enemy_data.second;
      b2Vec2 spawn_location = enemy->Calculate_Bullet_Spawn();
      b2Vec2 bullet_velocity;

      if (enemy->isFacingRight()) {
        bullet_velocity = b2Vec2(6.0f, 0.0f);
      } else {
        bullet_velocity = b2Vec2(-6.0f, 0.0f);
      }

      Entity* bullet = new Bullet(world, spawn_location, false);
      bullet->getBody()->SetLinearVelocity(bullet_velocity);
      entity_manager_.insert(std::pair<unsigned int, Entity*> (Entity::GetEntityID(), bullet));
    }
  }

  //Window Scrolling
  //Bounded at the right and left fifths of the screen
  int right_bound = scope_x_ + (getWindowWidth() * 0.8);
  int left_bound = scope_x_ + (getWindowWidth() * 0.2);
  int player_position = (int) (player_->getBody()->GetPosition().x * kPixelsPerMeter);

  if (player_position > right_bound) {
    //This ensures the view shifts at the same speed of the player, so the player never has to wait for the view to catch up to it.
    scope_x_ += player_position - right_bound;

  } else if (player_position < left_bound && scope_x_ > 0) {
    scope_x_ -= left_bound - player_position;

    if (scope_x_ < 0) {
      scope_x_ = 0;
    }
  }
}

// The following function was 100% copied from the Snake assignment
template <typename C>
void PrintText(const std::string& text, const C& color, const cinder::ivec2& size,
               const cinder::vec2& loc) {
  cinder::gl::color(color);

  auto box = cinder::TextBox()
      .alignment(cinder::TextBox::CENTER)
      .font(cinder::Font("Arial", 30))
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

void MyApp::draw() {
  ci::gl::clear();
  ci::gl::setMatricesWindow(getWindowSize());
  ci::gl::translate(-scope_x_, 0);

  if (lives_ <= 0) {
    ci::Color color(1,0,0); // Red
    const cinder::ivec2 size = {500, 500};
    const cinder::vec2 center = getWindowCenter();

    PrintText("U DED", color, size, ci::vec2(center.x + scope_x_, center.y));
  }

  // Life Counter
  ci::Color color(0,0,1); // Bleu
  const cinder::ivec2 size = {50, 50};
  const cinder::vec2 center(50.0f + scope_x_, 50.0f); //Top-left Corner
  PrintText(std::to_string(lives_), color, size, center);

  //https://github.com/asaeed/Box2DTest/blob/master/src/Particle.cpp //TOdo figure out if this is still relevant

  b2Vec2 position = player_->getBody()->GetPosition();

  int k = kPixelsPerMeter; //TODO get rid o this

  player_->Draw();

  for (Wall* wall : walls_) {
    wall->Draw();
  }

  for (std::pair<unsigned int, Entity*> entity_data : entity_manager_) {
    entity_data.second->Draw();
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

    Entity* bullet = new Bullet(world, spawn_location, true);
    //bullet.getBody()->ApplyLinearImpulse(bullet_impulse, spawn_location);
    bullet->getBody()->SetLinearVelocity(bullet_velocity);
    entity_manager_.insert(std::pair<unsigned int, Entity*> (Entity::GetEntityID(), bullet));
  }
}

void MyApp::keyUp(cinder::app::KeyEvent event) {
  b2Body* body = player_->getBody();

  if (event.getCode() == KeyEvent::KEY_LEFT || event.getCode() == KeyEvent::KEY_RIGHT) {
      b2Vec2 velocity(0.0f, body->GetLinearVelocity().y);
      body->SetLinearVelocity(velocity);
  }
}

void MyApp::DestroyEntity(unsigned int entity_ID){
  Entity* entity = entity_manager_.at(entity_ID);
  world->DestroyBody(entity->getBody());
  entity_manager_.erase(entity_ID);
  delete(entity);
}

void MyApp::BulletCollision(b2Fixture* bullet, b2Fixture* other) {
  //Currently just destroys the bullet
  unsigned int bullet_ID = (unsigned int) bullet->GetUserData();
  entities_to_destroy_.insert(bullet_ID);

  if (other->GetBody() == player_->getBody()) { //TODO if Player becomes entity, this will be more straightforward inside the entities section
    lives_--;
  }

  void* other_data = other->GetUserData();
  if (other_data != NULL) { //Therefore, other must be an entity
    unsigned int other_ID = (unsigned int) other_data;
    Entity* entity = entity_manager_.at(other_ID);

    // If it is an enemy, it must be removed from the enemy_shooters_ map //todo useless comment?
    if (entity->GetEntityType() == EntityType::type_enemy) {

      // This prevents friendly-fire between enemies
      if (!(((Bullet*) entity_manager_.at(bullet_ID))->isPlayerMade())) {
        return;
      }

      enemy_shooters_.erase(other_ID);
    }

    entities_to_destroy_.insert((unsigned int) other_data);
  }
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
    return;
  }

  if (fixture_B->GetBody()->IsBullet()) {
    this->myApp->BulletCollision(fixture_B, fixture_A);
    return;
  }

  // The Next two are for when an enemy collides with a non-bullet (which it can't be if it reached this point), non-player object

  if (fixture_A->GetUserData() != NULL) {
    Entity* entity = myApp->entity_manager_.at((unsigned int) fixture_A->GetUserData());
    if (entity->GetEntityType() == EntityType::type_enemy && fixture_B->GetBody() != myApp->player_->getBody()) {
      ((Enemy*) entity)->TurnAround();
    }
  }

  if (fixture_B->GetUserData() != NULL) {
    Entity* entity = myApp->entity_manager_.at((unsigned int) fixture_B->GetUserData());
    if (entity->GetEntityType() == EntityType::type_enemy && fixture_A->GetBody() != myApp->player_->getBody()) {
      ((Enemy*) entity)->TurnAround();
    }
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
