// Copyright (c) 2020 [Your Name]. All rights reserved.

#include "my_app.h"

#include <cinder/app/App.h>
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include <cinder/Text.h>
#include <cinder/Font.h>
#include <cinder/app/Window.h>
#include <thread>
#include <chrono>

using mylibrary::Player;
using mylibrary::Bullet;
using mylibrary::Entity;
using mylibrary::Enemy;
using mylibrary::EntityType;
using mylibrary::Wall;
using mylibrary::FlyingEnemy;

namespace myapp {

using cinder::app::KeyEvent;

const ci::Color kRed = ci::Color(1,0,0);
const ci::Color kGreen = ci::Color(0,1,0);
const ci::Color kBlue = ci::Color(0,0,1);
const ci::Color kYellow = ci::Color(1,1,0);
const ci::Color kPurple = ci::Color(1,0,1);
const ci::Color kCyan = ci::Color(0,1,1);
const ci::Color kDarkGreen = ci::Color(0,0.4f,0);
const ci::Color kLightBlue = ci::Color(0.4, 1, 1);
const ci::Color kOrange = ci::Color(1,0.5f,0);
const ci::Color kDarkPurple = ci::Color(0.4f, 0, 0.4f);
const ci::Color kDeepRed = ci::Color(0.6f,0,0);
const ci::Color kGray = ci::Color(0.57f, 0.57f, 0.57f);
const ci::Color kGold = ci::Color(0.83f, 0.69f, 0.215f);
const ci::Color kDarkPink = ci::Color(1, 0, 0.5f);
const ci::Color kLightGreen = ci::Color(0.2f, 1, 0.2f);
const ci::Color kDarkBlue = ci::Color(0, 0, 0.4f);
const ci::Color kShallowRed = ci::Color(1, 0.17f, 0.17f);

const b2Vec2 kGravity = b2Vec2(0, -18);

//The enemies will activate when they are within this many pixels of the screen
const int kActivateRange = 160;

const int kStartingLives = 5;
const int kEnemyReloadTime = 80;
const int kPlayerReloadTime = 25;
const float kFinishWidth = 2.5f;
const int kStartLevel = 0;
const int kFinalLevel = 1;
const int kWaitTime = 4;

MyApp::MyApp() {
  lives_ = kStartingLives;
  level_ = kStartLevel;
  jump_timer_ = 0;
  shooting_timer_ = 0;
  enemy_shooting_timer_ = kEnemyReloadTime;
  sensor_contacts_ = 0;

  developer_mode_ = false;
  title_screen_ = true;

  ci::ImageSourceRef finish_image = ci::loadImage(ci::app::loadAsset("redflagbot.png"));
  ci::ImageSourceRef win_image= ci::loadImage(ci::app::loadAsset("greenflagbot.png"));
  finish_line_bot_ = ci::gl::Texture2d::create(finish_image);
  finish_line_bot_win_ = ci::gl::Texture2d::create(win_image);

  AudioSetup();
}

/**
 * Developer Note - this function is very long. All of the code in here is used
 * the game layout. Since the entire level is loaded at once, and (while given
 * separate labels) the different obstacles are dependent on each other (e.g.
 * ground blocks that stretch across multiple obstacles, enemies that move
 * between areas, etc.) Arbitrarily separating the level setup into multiple
 * functions would be unnecessary and confusing, hence why this function is 100+
 * lines long.
 * */
void MyApp::setup() {
  cinder::app::WindowRef windowRef = this->getWindow();
  windowRef->setFullScreen();

  if (level_ == 0) {
    LevelZero();
  } else if (level_ == 1) {
    LevelOne();
  }

}

void MyApp::AudioSetup() {
  ci::audio::SourceFileRef laser_file =
      ci::audio::load(ci::app::loadAsset("laser.mp3"));
  laser_sound_ = ci::audio::Voice::create(laser_file);

  ci::audio::SourceFileRef boom_file =
      ci::audio::load(ci::app::loadAsset("boom.mp3"));
  boom_sound_ = ci::audio::Voice::create(boom_file);

  ci::audio::SourceFileRef damage_file =
      ci::audio::load(ci::app::loadAsset("damage.mp3"));
  damage_sound_ = ci::audio::Voice::create(damage_file);

  ci::audio::SourceFileRef music_file =
      ci::audio::load(ci::app::loadAsset("Spectral.mp3"));
  music_player_ = ci::audio::VoiceSamplePlayerNode::create(music_file);
  music_player_->getSamplePlayerNode()->setLoopEnabled();
}

// y_loc is the height of the enemy's feet. x_loc is the center of the enemy.
void MyApp::EnemyInit(float x_loc, float y_loc, bool is_facing_right) {
  Enemy* enemy = new Enemy(world_, b2Vec2(x_loc, y_loc + kEnemyHeight), is_facing_right);
  std::pair<unsigned int, Enemy*> enemy_data(Entity::GetEntityID(), enemy);
  entity_manager_.insert(enemy_data);
  asleep_enemies_.insert(enemy_data);
}

void MyApp::FlyingEnemyInit(float x_loc, float y_loc, bool is_facing_right){
  FlyingEnemy* enemy = new FlyingEnemy(world_, b2Vec2(x_loc, y_loc + kEnemyHeight), is_facing_right);
  std::pair<unsigned int, Enemy*> enemy_data(Entity::GetEntityID(), enemy);
  entity_manager_.insert(enemy_data);
  asleep_enemies_.insert(enemy_data);
}

// x and y _loc are the bottom left corner
void MyApp::WallInit(float x_loc, float y_loc, float width, float height, ci::Color color) {
  Wall* wall = new Wall(world_, x_loc + width/2, y_loc + height/2, width/2, height/2, color);
  walls_.push_back(wall);
}

void MyApp::GroundInit(float start, float end) {
  WallInit(start, 0, (end-start), 0.2f, kBlue);
}

// Needs to call restart if level is changing
void MyApp::PlayerWorldInit(float x_loc, float y_loc) {
  world_ = new b2World(kGravity);
  player_ = new Player(world_, x_loc, y_loc);
  world_->SetContactListener(&contact_listener_);
  contact_listener_.myApp_ = this;
}

void MyApp::update() {
  if (won_level_) {

    if (level_ == kFinalLevel) {
      return;
    }

    level_++;
    std::this_thread::sleep_for(std::chrono::seconds(kWaitTime));
    Restart(); //Calls setup on its own
  }

  if (lives_ <= 0 || title_screen_) {
    return;
  }

  for (unsigned int bullet_ID : entities_to_destroy_) {
    DestroyEntity(bullet_ID);
  }
  entities_to_destroy_.clear();

  ActivateEnemies();

  //Simulate
  float timeStep = 1.0f / 60.0f;
  // Iterations affect how many calculations the program does. The values were recommended on Box2D.org
  int32 velocityIterations = 8;
  int32 positionIterations = 3;
  world_->Step(timeStep, velocityIterations, positionIterations);

  //using the timer the opposite way with the timer increasing at every step would remove the need for the conditional,
  //but the value could get very large and overflow if the program was left idling.
  if (jump_timer_ > 0) {
    jump_timer_--;
  }

  if (shooting_timer_ > 0) {
    shooting_timer_--;
  }

  b2Vec2 player_position = player_->getBody()->GetPosition();
  //Death by falling
  if (player_position.y < 0) {
    lives_ = 0;
  }

  if (player_position.x > end_position_ - kFinishWidth) {
    won_level_ = true;
  }

  for (std::pair< unsigned int, FlyingEnemy*> data_pair : flying_enemies_) {
    data_pair.second->Fly();
  }

  UpdateActiveEnemies();
  ScrollWindow();
}

void MyApp::ActivateEnemies() {
  //If an enemy is within the window, it will be Activated
  // Next 20 lines copied (with modifications) from:
  // https://www.techiedelight.com/remove-entries-map-iterating-cpp/
  // This method is necessary for iterating through map while erasing
  // elements within map, since erasing invalidates the iterator
  auto it = asleep_enemies_.cbegin();
  while (it != asleep_enemies_.cend())
  {
    unsigned int id = it->first;
    Enemy* enemy = it->second;

    if ((enemy->Activate(((window_shift_ - kActivateRange)/ kPixelsPerMeter),
                         ((window_shift_ + getWindowWidth() + kActivateRange) / kPixelsPerMeter)))) {
      enemy_shooters_.insert(std::pair< unsigned int, Enemy*> (id, enemy));

      if (enemy->isFlying()) {
        flying_enemies_.insert(std::pair< unsigned int, FlyingEnemy*> (id, (FlyingEnemy*) enemy));
      }

      // supported in C++11
      it = asleep_enemies_.erase(it);
    }
    else {
      ++it;
    }
  }
}

void MyApp::UpdateActiveEnemies() {
  if (enemy_shooting_timer_ > 0) {
    enemy_shooting_timer_--;
  } else {
    enemy_shooting_timer_ = kEnemyReloadTime;

    for (std::pair<unsigned int, Enemy*> enemy_data : enemy_shooters_) {
      Enemy* enemy = enemy_data.second;

      // Death by Falling for enemies
      // Handled here since enemies are already being iterated through.
      // Also better to do it where it isn't being iterated on every game step,
      // since that is unnecessary. This is simply clean-up.
      if (enemy->getBody()->GetPosition().y < 0) {
        entities_to_destroy_.insert((unsigned int) enemy->getBody()->GetUserData());
      }

      Entity* bullet = enemy->Shoot(world_);
      entity_manager_.insert(std::pair<unsigned int, Entity*> (Entity::GetEntityID(), bullet));
    }
  }
}

void MyApp::ScrollWindow() {
  //Bounded at the right and left thirds of the screen
  int right_bound = window_shift_ + (getWindowWidth() * 0.57);
  int left_bound = window_shift_ + (getWindowWidth() * 0.23);
  int pixel_player_position = (int) (player_->getBody()->GetPosition().x * kPixelsPerMeter);

  if (pixel_player_position > right_bound
      && (window_shift_ + getWindowWidth()) < (end_position_ * kPixelsPerMeter)) {

    //This ensures the view shifts at the same speed of the player,
    // so the player never has to wait for the view to catch up to it.
    window_shift_ += pixel_player_position - right_bound;

  } else if (pixel_player_position < left_bound && window_shift_ > (left_window_bound_ * kPixelsPerMeter)) {
    window_shift_ -= left_bound - pixel_player_position;
  }

  if (window_shift_ < left_window_bound_) {
    window_shift_ = left_window_bound_;
  }
}

// The following function was copied from the Snake assignment, with minor mods
template <typename C>
void PrintText(const std::string& text, const C& color, const cinder::ivec2& size,
               const cinder::vec2& loc, int font_size) {
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

void MyApp::draw() {
  ci::gl::clear();

  if (title_screen_) {
    DrawTitleScreen();
    return;
  }

  ci::gl::setMatricesWindow(getWindowSize());
  ci::gl::translate(-window_shift_, 0);

  player_->Draw();

  for (Wall* wall : walls_) {
    wall->Draw();
  }

  for (std::pair<unsigned int, Entity*> entity_data : entity_manager_) {
    entity_data.second->Draw();
  }

  if (lives_ <= 0) {
    const cinder::ivec2 size = {500, 500};
    const cinder::vec2 center = getWindowCenter();

    PrintText("U DED", kRed, size, ci::vec2(center.x + window_shift_, center.y), 100);
  }

  // Life Counter
  const cinder::ivec2 size = {50, 50};
  const cinder::vec2 center(50.0f + window_shift_, 50.0f); //Top-left Corner
  PrintText(std::to_string(lives_), kBlue, size, center,50);

  ci::gl::Texture2dRef finish_image;

  if (won_level_) {
    const cinder::ivec2 size = {500, 500};
    const cinder::vec2 center = getWindowCenter();

    std::string to_print;
    if (level_ == kFinalLevel) {
      to_print = "YOU WIN!";
    } else {
      to_print = "Level Complete";
    }

    PrintText(to_print, kGreen, size, ci::vec2(center.x + window_shift_, center.y), 100);

    finish_image = finish_line_bot_win_;

  } else {
    finish_image = finish_line_bot_;
  }

  // Necessary or the image will be tinted the color of the last drawn object.
  ci::Color reset(1,1,1);
  ci::gl::color(reset);
  // 346 = height of ground(18) + height of robot image(328)
  ci::gl::draw(finish_image, ci::vec2((end_position_ - kFinishWidth) * kPixelsPerMeter, getWindowHeight() - 346));

  if (developer_mode_) {
    DrawDeveloperMode();
  }
}

void MyApp::DrawDeveloperMode() {

  for (size_t i = 0; i < 20; i++) {
    PrintText(std::to_string(i), kCyan, ci::ivec2(50,50),
        ci::vec2(i * kPixelsPerMeter, getWindowCenter().y), 30);
  }

  for (size_t i = 0; i < 13; i++) {
    PrintText(std::to_string(i), kPurple, ci::ivec2(50,50),
        ci::vec2(getWindowCenter().x + window_shift_,
            getWindowHeight() - i * kPixelsPerMeter), 30);
  }
}

void MyApp::DrawTitleScreen() {
  ci::ivec2 size(1500,1500);
  PrintText("ROBOT REVOLT", kRed, size, getWindowCenter(), 300);
  PrintText("Press Space to Begin", kGreen, size,
      ci::ivec2(getWindowCenter().x, getWindowCenter().y + 250), 120);

  player_->Draw();

  PrintText("Created by Nathan Omerza", kBlue, ci::ivec2(250,250),
      ci::ivec2(getWindowWidth() - 250, getWindowHeight() - 230), 30);

  PrintText("Music: \"Spectral\" by Kyle Adomaitis", kBlue, ci::ivec2(250,250),
      ci::ivec2(getWindowWidth() - 250, getWindowHeight() - 150), 30);
}

void MyApp::keyDown(KeyEvent event) {
  b2Body* body = player_->getBody();
  int key = event.getCode();

  if (key == KeyEvent::KEY_UP && sensor_contacts_ >= 1 &&
      jump_timer_ == 0) {  // only jump if in contact with ground

      b2Vec2 impulse_vector(0.0f, 27.0f); // Allows for ~3m jump
      body->ApplyLinearImpulse(impulse_vector, body->GetPosition());
      jump_timer_ = 10; // NOTE this was arbitrarily chosen, change if necessary.

  } else if (key == KeyEvent::KEY_RIGHT) {
      b2Vec2 velocity(7.0f, body->GetLinearVelocity().y);
      // Need to use previous y velocity, or trying to move side to side mid-air will cause player to suddenly fall
      body->SetLinearVelocity(velocity);
      player_->setFacingRight(true);

  } else if (key == KeyEvent::KEY_LEFT) {
      b2Vec2 velocity(-7.0f, body->GetLinearVelocity().y);
      body->SetLinearVelocity(velocity);
      player_->setFacingRight(false);

  } else if (key == KeyEvent::KEY_SPACE && shooting_timer_ == 0) {
    if (title_screen_) {
      title_screen_ = false;
      music_player_->start();
      return;
    }

    laser_sound_->start();

    shooting_timer_ = kPlayerReloadTime;

    b2Vec2 player_position = player_->getBody()->GetPosition();
    b2Vec2 spawn_location;
    b2Vec2 bullet_velocity;

    if (player_->isFacingRight()) {
      spawn_location = b2Vec2(player_position.x + (3*kPlayerWidth/4),
          player_position.y - (kPlayerHeight/5));
      // The kPlayerHeight/5 is to make it appear to spawn closer to the gun.
      bullet_velocity = b2Vec2(7.1f, 0.0f);
    } else {
      spawn_location = b2Vec2(player_position.x - (3*kPlayerWidth/4),
          player_position.y - (kPlayerHeight/5));
      bullet_velocity = b2Vec2(-7.1f, 0.0f);
    }

    Entity* bullet = new Bullet(world_, spawn_location, true);
    bullet->getBody()->SetLinearVelocity(bullet_velocity);
    entity_manager_.insert(std::pair<unsigned int, Entity*>
        (Entity::GetEntityID(), bullet));

  } else if (event.getCode() == KeyEvent::KEY_r) {
    Restart();
  }
}

void MyApp::keyUp(cinder::app::KeyEvent event) {
  b2Body* body = player_->getBody();

  if (event.getCode() == KeyEvent::KEY_LEFT
    || event.getCode() == KeyEvent::KEY_RIGHT) {

      b2Vec2 velocity(0.0f, body->GetLinearVelocity().y);
      body->SetLinearVelocity(velocity);
  }
}

void MyApp::DestroyEntity(unsigned int entity_ID){
  Entity* entity = entity_manager_.at(entity_ID);

  if (entity->GetEntityType()  == mylibrary::type_enemy) {
    Enemy* enemy = (Enemy*) entity;
    if (enemy->isActive()) {
      enemy_shooters_.erase(entity_ID);

      if (enemy->isFlying()) {
        flying_enemies_.erase(entity_ID);
      }

    } else { // not Active
      asleep_enemies_.erase(entity_ID);
    }
  }

  world_->DestroyBody(entity->getBody());
  entity_manager_.erase(entity_ID);
  delete(entity);
}

void MyApp::BulletCollision(b2Fixture* bullet, b2Fixture* other) {
  unsigned int bullet_ID = (unsigned int) bullet->GetUserData();
  entities_to_destroy_.insert(bullet_ID);

  if (other->GetBody() == player_->getBody()) {
    lives_--;
    damage_sound_->start();
  }

  void* other_data = other->GetUserData();
  if (other_data != NULL) { //Therefore, other must be an entity
    unsigned int other_ID = (unsigned int) other_data;
    Entity* entity = entity_manager_.at(other_ID);

    // If it is an enemy, it must be removed from the enemy_shooters_ map
    if (entity->GetEntityType() == EntityType::type_enemy) {

      // This prevents friendly-fire between enemies
      if (!(((Bullet*) entity_manager_.at(bullet_ID))->isPlayerMade())) {
        return;
      }

      // At this point, it is a valid shot against an enemy.
      boom_sound_->start();
    }

    entities_to_destroy_.insert((unsigned int) other_data);
  }
}

// Code for ContactListeners to determine if the user is able to jump (citation):
// https://www.iforce2d.net/b2dtut/jumpability

void MyApp::ContactListener::BeginContact(b2Contact* contact) {
  b2Fixture* fixture_A = contact->GetFixtureA();
  b2Fixture* fixture_B = contact->GetFixtureB();
  //check if either fixture is the foot sensor

  if (fixture_A->IsSensor() || fixture_B->IsSensor()) {

    myApp_->sensor_contacts_++;
      return;
  }

  if (fixture_A->GetBody()->IsBullet()) {
    this->myApp_->BulletCollision(fixture_A, fixture_B);
    return;
  }

  if (fixture_B->GetBody()->IsBullet()) {
    this->myApp_->BulletCollision(fixture_B, fixture_A);
    return;
  }

  // The Next two are for when an enemy collides with a non-bullet
  // (which it can't be if it reached this point), non-player object
  if (fixture_A->GetUserData() != NULL) {
    Entity* entity =
        myApp_->entity_manager_.at((unsigned int) fixture_A->GetUserData());

    if (entity->GetEntityType() == EntityType::type_enemy
      && fixture_B->GetBody() != myApp_->player_->getBody()) {
      ((Enemy*) entity)->TurnAround();
    }
  }

  if (fixture_B->GetUserData() != NULL) {
    Entity* entity =
        myApp_->entity_manager_.at((unsigned int) fixture_B->GetUserData());

    if (entity->GetEntityType() == EntityType::type_enemy
      && fixture_A->GetBody() != myApp_->player_->getBody()) {
      ((Enemy*) entity)->TurnAround();
    }
  }
}

void MyApp::ContactListener::EndContact(b2Contact* contact) {
    //check if either fixture is the foot sensor
    if ((int)contact->GetFixtureA()->GetUserData() == kFootSensorID
      || (int)contact->GetFixtureB()->GetUserData() == kFootSensorID) {
      myApp_->sensor_contacts_--;
    }
}

void MyApp::Restart() {
  delete world_;  // Also deletes every b2body in the world
  delete player_;

  sensor_contacts_ = 0;
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
  flying_enemies_.clear();

  Entity::ResetID();

  lives_ = kStartingLives;
  jump_timer_ = 0;
  shooting_timer_ = 0;
  enemy_shooting_timer_ = kEnemyReloadTime;
  window_shift_ = 0;

  won_level_ = false;

  setup();
}

void MyApp::LevelZero() {
  PlayerWorldInit(1, 4);
  window_shift_ = 0;
  left_window_bound_ = window_shift_;
  won_level_ = false;

  // Intro
  GroundInit(0, 45);
  WallInit(-0.1f, 0, 0.1f, getWindowHeight()/kPixelsPerMeter, kYellow);
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

  // Split Path
  WallInit(130, 3.2f, 3, 0.2f, kDeepRed);
  // Bottom Section
  WallInit(134, 0.5f, 0.6f, 0.2f,kDeepRed);
  WallInit(142, 0.5f, 0.6f, 0.2f,kDeepRed);
  WallInit(150, 0.5f, 0.6f, 0.2f,kDeepRed);
  // Top Section
  WallInit(134, 5.5f, 17, 0.2f, kDeepRed);
  EnemyInit(137, 5.6f, false);
  EnemyInit(139, 5.6f, true);
  EnemyInit(141, 5.6f, false);
  EnemyInit(143, 5.6f, true);
  EnemyInit(145, 5.6f, false);
  EnemyInit(147, 5.6f, true);
  WallInit(151, 5.5, 0.1f,0.5f, kDeepRed);
  //Recombined
  WallInit(153, 2.5f, 3, 0.2f, kDeepRed);
  GroundInit(157, 188);

  // The Pit
  WallInit(164, 0, 2, 3, kGray);
  WallInit(166, 0, 2, 6, kGray);
  WallInit(168, 0, 2, 9, kGray);
  WallInit(170, 0, 8, 5, kGray);
  EnemyInit(171, 5.1f, true);
  EnemyInit(173, 5.1f, false);
  EnemyInit(175, 5.1f, true);
  EnemyInit(176.5f, 5.1f, false);
  WallInit(177.9f, 5, 0.1f, 0.4f, kGray);
  WallInit(179.4f, 2, 1.6f, 3, kGray);
  WallInit(179.4f, 5, 0.1f, 0.4f, kGray);
  EnemyInit(180, 5.1f, true);
  WallInit(181, 2, 2, 9, kGray);

  end_position_ = 188;
}

void MyApp::LevelOne() {
  PlayerWorldInit(91, 4);
  window_shift_ = 90*kPixelsPerMeter;
  left_window_bound_ = 90;
  won_level_ = false;

  WallInit(89.9f, 0, 0.1f, getWindowHeight()/kPixelsPerMeter, kYellow);

  // Intro to jetpacks
  GroundInit(90, 119);
  WallInit(95, 0, 1.5f, 3, kShallowRed);
  FlyingEnemyInit(102, 0.5f, true);
  FlyingEnemyInit(106.5f, 0.5f, true);
  WallInit(110, 0, 3, 3, kShallowRed);
  WallInit(113, 0, 3, 6, kShallowRed);
  WallInit(116, 0, 3, 9, kShallowRed);

  // Open Air jumps and jetpacks
  WallInit(124, 4, 2, 0.2f, kDarkBlue);
  WallInit(129, 6.5f, 2, 0.2f, kDarkBlue);
  WallInit(136, 4, 2, 0.2f, kDarkBlue);
  FlyingEnemyInit(137.6f, 4.2f, true);
  WallInit(141, 7, 2, 0.2f, kDarkBlue);
  WallInit(146, 10, 2, 0.2f, kDarkBlue);
  FlyingEnemyInit(147.6f, 10.5f, true);
  WallInit(153, 5, 2, 0.2f, kDarkBlue);

  //Opening With Scaffolding
  GroundInit(160, 257);
  EnemyInit(165, 0.3f, true);
  WallInit(166, 3, 3, 0.2f, kLightGreen);
  FlyingEnemyInit(171, 0.2f, true);
  WallInit(172, 1.6f, 3, 0.2f, kLightGreen);
  EnemyInit(173.5f, 2.2f, true);
  WallInit(172, 5.2f, 3, 0.2f, kLightGreen);
  FlyingEnemyInit(173.5f, 4.3f, true);
  WallInit(178, 0, 3, 3, kLightGreen);

  // Flying Enemy Cage
  WallInit(188, 1.5, 2, 10.3f, kGold);
  WallInit(206, 0, 2, 9.2f, kGold);
  WallInit(204, 0, 2, 1.5f, kGold);
  WallInit(198, 3.2, 2, 1.2f, kGold);
  WallInit(192, 4.7, 2, 1, kGold);
  WallInit(198, 7.3f, 2, 1, kGold);
  WallInit(203, 9.2f, 5, 0.5f, kGold);
  WallInit(188, 11.8f, 20, 0.3f, kGold);
  FlyingEnemyInit(195, 1, true);
  FlyingEnemyInit(202, 0.5f, false);
  FlyingEnemyInit(192.5f, 6, false);

  // Pyramid
  WallInit(217, 1.5f, 1, 12, kDarkPink);
  WallInit(221, 0, 21, 2, kDarkPink);
  WallInit(224, 0, 15, 4, kDarkPink);
  WallInit(227, 0, 9, 6, kDarkPink);
  WallInit(230, 0, 3, 8, kDarkPink);
  WallInit(246, 1.5f, 1, 12, kDarkPink);
  WallInit(218, 11.8f, 28, 0.3f, kDarkPink);
  FlyingEnemyInit(220, 1, false);
  FlyingEnemyInit(226, 6, true);
  EnemyInit(228, 6.3f, true);
  FlyingEnemyInit(232, 8.5f, false);
  EnemyInit(235, 6.3f, false);
  FlyingEnemyInit(238, 4.5f, false);
  FlyingEnemyInit(244, 0.5f, true);

  end_position_ = 257.0f;
}

}  // namespace myapp
