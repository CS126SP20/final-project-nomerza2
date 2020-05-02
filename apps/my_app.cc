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

namespace myapp {

using cinder::app::KeyEvent;

const ci::Color kYellow = ci::Color(1,1,0);
const ci::Color kGreen = ci::Color(0,1,0);
const ci::Color kDarkGreen = ci::Color(0,0.4f,0);
const ci::Color kRed = ci::Color(1,0,0);
const ci::Color kLightBlue = ci::Color(0.4, 1, 1);
const ci::Color kOrange = ci::Color(1,0.5f,0);
const ci::Color kDarkPurple = ci::Color(0.4f, 0, 0.4f);
const ci::Color kBlue = ci::Color(0,0,1);
const b2Vec2 kGravity = b2Vec2(0, -18);

//The enemies will activate when they are within this many pixels of the screen
const int kActivateRange = 85;

MyApp::MyApp() {
  world = new b2World(kGravity);

  player_ = new Player(world);

  world->SetContactListener(&contactListener);
  contactListener.myApp = this;

  lives_ = 5;
  jump_timer = 0;
  shooting_timer = 0;
  enemy_shooting_timer_ = 44;
  scope_x_ = 0;
  sensor_contacts = 0;

  end_position_ = 130.0f;
  finish_width_ = 2.5f;

  won_game = false;
  developer_mode = false;
}

void MyApp::setup() {
  cinder::app::WindowRef windowRef = this->getWindow();
  windowRef->setFullScreen();

  // Intro
  GroundInit(0, 45);
  WallInit(3.0f, 0, 3, 2, kYellow);
  WallInit(11, 0, 3, 3.0f, kYellow);
  WallInit(14, 0, 3, 6.0f, kRed);
  WallInit(17, 0, 3, 9.0f, kGreen);
  EnemyInit(21, 0.4f, true);

  // Double With Bridge
  WallInit(28, 0, 2, 3, kDarkGreen);
  EnemyInit(30, 0.4f, true);
  WallInit(34.5f, 4.5f, 2, 2, kDarkGreen);
  EnemyInit(36, 0.4f, false);
  WallInit(42, 0, 1.5f, 2, kDarkGreen);

  // First Fall
  WallInit(47, 3.5f, 2, 1, kLightBlue);
  GroundInit(55.5f, 61);
  EnemyInit(60, 0.4f, true);

  // Separated Stairs
  WallInit(66.1f, 1, 2, 2, kOrange);
  WallInit(69.5f, 4.2f, 1.5f, 1.5f, kOrange);
  WallInit(74.5f, 7.2f, 1.5f, 1.5f, kOrange);
  WallInit(80.5f, 10, 8.5f, 0.5f, kOrange);
  EnemyInit(88.5f, 11, true);

  // Staircase of Enemies
  GroundInit(96, 130);
  EnemyInit(102, 0.4, true);
  WallInit(105, 0, 4, 1.5f, kDarkPurple);
  EnemyInit(107, 1.7f, false);
  EnemyInit(108, 1.7f, true);
  WallInit(109, 0, 4, 3.8f, kDarkPurple);
  EnemyInit(110, 4, true);
  EnemyInit(112, 4, false);
  WallInit(113, 0, 4, 5.6f, kDarkPurple);
  EnemyInit(113.2f, 5.8f, false);
  EnemyInit(114.2f, 5.8f, true);
  EnemyInit(115.2f, 5.8f, false);
  EnemyInit(116.2f, 5.8f, true);
  WallInit(117, 0, 4, 8.4f, kDarkPurple);
  EnemyInit(120.5f, 8.6f, true);
}

// y_loc is the height of the enemy's feet
void MyApp::EnemyInit(float x_loc, float y_loc, bool is_facing_right) {
  Enemy* enemy = new Enemy(world, b2Vec2(x_loc, y_loc + kEnemyHeight), is_facing_right);
  std::pair<unsigned int, Enemy*> enemy_data(Entity::GetEntityID(), enemy);
  entity_manager_.insert(enemy_data);
  asleep_enemies_.insert(enemy_data);
}

// x and y _loc are the bottom left corner
void MyApp::WallInit(float x_loc, float y_loc, float width, float height, ci::Color color) {
  Wall* wall = new Wall(world, x_loc + width/2, y_loc + height/2, width/2, height/2, color);
  walls_.push_back(wall);
}

void MyApp::GroundInit(float start, float end) {
  WallInit(start, 0, (end-start), 0.2f, ci::Color(0, 0, 1));
}

void MyApp::update() {
  if (lives_ <= 0 || won_game) {
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

    if ((enemy->Activate(((scope_x_ - kActivateRange)/ kPixelsPerMeter),
        ((scope_x_ + getWindowWidth() + kActivateRange) / kPixelsPerMeter)))) {
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

  b2Vec2 player_position = player_->getBody()->GetPosition();
  //Death by falling
  if (player_position.y < 0) {
    lives_ = 0;
  }

  if (player_position.x > end_position_ - finish_width_) {
    won_game = true;
    EndGame();
  }

  if (enemy_shooting_timer_ > 0) {
    enemy_shooting_timer_--;
  } else {
    enemy_shooting_timer_ = 80;

    for (std::pair<unsigned int, Enemy*> enemy_data : enemy_shooters_) {
      Enemy* enemy = enemy_data.second;
      b2Vec2 spawn_location = enemy->Calculate_Bullet_Spawn();
      b2Vec2 bullet_velocity;

      // Death by Falling for enemies
      // Handled here since enemies are already being iterated through.
      // Also better to do it where it isn't being iterated on every game step,
      // since that is unnecessary. This is simply clean-up.
      if (enemy->getBody()->GetPosition().y < 0) {
        entities_to_destroy_.insert((unsigned int) enemy->getBody()->GetUserData());
      }

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
  //Bounded at the right and left thirds of the screen
  int right_bound = scope_x_ + (getWindowWidth() * 0.67);
  int left_bound = scope_x_ + (getWindowWidth() * 0.33);
  int pixel_player_position = (int) (player_position.x * kPixelsPerMeter);

  if (pixel_player_position > right_bound
    && (scope_x_ + getWindowWidth()) < (end_position_ * kPixelsPerMeter)) {

    //This ensures the view shifts at the same speed of the player, so the player never has to wait for the view to catch up to it.
    scope_x_ += pixel_player_position - right_bound;

  } else if (pixel_player_position < left_bound && scope_x_ > 0) {
    scope_x_ -= left_bound - pixel_player_position;

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
  const cinder::ivec2 size = {50, 50};
  const cinder::vec2 center(50.0f + scope_x_, 50.0f); //Top-left Corner
  PrintText(std::to_string(lives_), kBlue, size, center);

  //https://github.com/asaeed/Box2DTest/blob/master/src/Particle.cpp //TOdo figure out if this is still relevant

  b2Vec2 position = player_->getBody()->GetPosition();

  int k = kPixelsPerMeter; //TODO get rid o this

  player_->Draw();

  for (Wall* wall : walls_) {
    wall->Draw();
  }

  if (won_game) {
    const cinder::ivec2 size = {500, 500};
    const cinder::vec2 center = getWindowCenter();

    PrintText("U WiN", kGreen, size, ci::vec2(center.x + scope_x_, center.y));
  }

  for (std::pair<unsigned int, Entity*> entity_data : entity_manager_) {
    entity_data.second->Draw();
  }

  if (developer_mode) {
    DrawDeveloperMode();
  }
}

void MyApp::DrawDeveloperMode() {

  for (size_t i = 100; i < 115; i++) {
    PrintText(std::to_string(i), ci::Color(0,1,1), ci::ivec2(50,50), ci::vec2(i * kPixelsPerMeter, getWindowCenter().y));
  }

  for (size_t i = 0; i < 13; i++) {
    PrintText(std::to_string(i), ci::Color(1,0,1), ci::ivec2(50,50), ci::vec2(getWindowCenter().x + scope_x_, getWindowHeight() - i * kPixelsPerMeter));
  }
}

void MyApp::keyDown(KeyEvent event) {
  b2Body* body = player_->getBody();

  if (event.getCode() == KeyEvent::KEY_UP && sensor_contacts >= 1 && jump_timer == 0) {  // only jump if in contact with ground //TODO take event.getCode() out of conditional?
      b2Vec2 impulse_vector(0.0f, 27.0f); // Allows for ~3m jump
      body->ApplyLinearImpulse(impulse_vector, body->GetPosition());
      jump_timer = 10; // NOTE this was arbitrarily chosen, change if necessary.

  } else if (event.getCode() == KeyEvent::KEY_RIGHT) {
      b2Vec2 velocity(7.0f, body->GetLinearVelocity().y); // Need to use previous y velocity, or trying to move side to side mid-air will cause player to suddenly fall
      // TODO take velocity changes into helper function?
      body->SetLinearVelocity(velocity);
      player_->setFacingRight(true);
  } else if (event.getCode() == KeyEvent::KEY_LEFT) {
      b2Vec2 velocity(-7.0f, body->GetLinearVelocity().y);
      body->SetLinearVelocity(velocity);
      player_->setFacingRight(false);

  } else if (event.getCode() == KeyEvent::KEY_SPACE && shooting_timer == 0) {
    shooting_timer = 25;

    b2Vec2 player_position = player_->getBody()->GetPosition();
    b2Vec2 spawn_location;
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

  } else if (event.getCode() == KeyEvent::KEY_r) {
    Restart();
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

  if (entity->GetEntityType()  == mylibrary::type_enemy) {
    enemy_shooters_.erase(entity_ID);
  }

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

      myApp->sensor_contacts++;
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
        myApp->sensor_contacts--;
    }
}

void MyApp::EndGame() {
  //TODO Delete if unused
}

void MyApp::Restart() {
  delete world;  // Also deletes every b2body in the world
  world = new b2World(kGravity);

  delete player_;
  player_ = new Player(world);

  sensor_contacts = 0;
  world->SetContactListener(&contactListener);
  contactListener.myApp = this;

  for (std::pair<unsigned int, Entity*> entity_data : entity_manager_) {
    delete entity_data.second;
  }
  entity_manager_.clear();
  for (Wall* wall : walls_) {
    delete wall;
  }
  walls_.clear();
  // Members deleted in entity_manager_
  enemy_shooters_.clear();
  asleep_enemies_.clear();
  entities_to_destroy_.clear();

  Entity::ResetID();

  lives_ = 5;
  jump_timer = 0;
  shooting_timer = 0;
  enemy_shooting_timer_ = 44;
  scope_x_ = 0;

  won_game = false;

  setup();
}

}  // namespace myapp
