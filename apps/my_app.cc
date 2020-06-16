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
#include <mylibrary/EffectiveDimensions.h>

using mylibrary::Player;
using mylibrary::Bullet;
using mylibrary::SuperBullet;
using mylibrary::Entity;
using mylibrary::Enemy;
using mylibrary::EntityType;
using mylibrary::EnemyType;
using mylibrary::Wall;
using mylibrary::FlyingEnemy;
using mylibrary::Hunter;
using mylibrary::RepairKit;
using mylibrary::EffectiveDimensions;
using mylibrary::MovingWall;

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
const ci::Color kHunterGreen = ci::Color(0.1f, 0.285f, 0.16f);
const ci::Color kBlueGray = ci::Color(0.168f, .539f, .5f);
const ci::Color kGoldGreen = ci::Color(0.5333f, 0.5922f, 0.235f);
const ci::Color kBlack = ci::Color(0, 0, 0);
const ci::Color kMiddleBlue = ci::Color(0.2f, 0.6f, 1);

const b2Vec2 kGravity = b2Vec2(0, -18);

//The enemies will activate when they are within this many pixels of the screen
const int kActivateRange = 160;

const int kStartingLives = 3;
const int kEnemyReloadTime = 80;
const int kPlayerReloadTime = 25;
const float kFinishWidth = 2.5f;
const int kStartLevel = 0;
const int kFinalLevel = 5;
const int kWaitTime = 4;

MyApp::MyApp() {
  lives_ = kStartingLives;
  level_ = kStartLevel;
  jump_timer_ = 0;
  shooting_timer_ = 0;
  scale_ = 1.0f;
  enemy_shooting_timer_ = kEnemyReloadTime;
  sensor_contacts_ = 0;
  can_press_up = true;

  developer_mode_ = false;
  title_screen_ = true;
  spike_death_ = false;

  ci::ImageSourceRef finish_image = ci::loadImage(ci::app::loadAsset("redflagbot.png"));
  ci::ImageSourceRef win_image= ci::loadImage(ci::app::loadAsset("greenflagbot.png"));
  finish_line_bot_ = ci::gl::Texture2d::create(finish_image);
  finish_line_bot_win_ = ci::gl::Texture2d::create(win_image);

  repair_value_ = -1; // Used to signify no difficult has been chosen. Won't be able to start game if this is -1

  setFullScreen(true);
  AudioSetup();
}

void MyApp::setup() {
  cinder::app::WindowRef windowRef = this->getWindow();
  //windowRef->setFullScreen();

  if (level_ == 0) {
    LevelZero();
  } else if (level_ == 1) {
    LevelOne();
  } else if (level_ == 2) {
    LevelTwo();
  } else if (level_ == 3) {
    LevelThree();
  } else if (level_ == 4) {
    LevelFour();
  } else if (level_ == 5) {
    LevelFive();
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

void MyApp::RescaleWindow() {
  float width_scale = (float) getWindowWidth() / kStandardWidth;
  float height_scale = (float) getWindowHeight() / kStandardHeight;

  if (width_scale < height_scale) {
    scale_ = width_scale;
    is_x_limiting_ = true;

  } else {
    scale_ = height_scale;
    is_x_limiting_ = false;
  }

  EffectiveDimensions::SetEffectiveWidth(kStandardWidth * scale_);
  EffectiveDimensions::SetEffectiveHeight(kStandardHeight * scale_);
}

// y_loc is the height of the enemy's feet. x_loc is the center of the enemy.
void MyApp::EnemyInit(float x_loc, float y_loc, bool is_facing_right) {
  Enemy* enemy = new Enemy(world_, b2Vec2(x_loc, y_loc + (kEnemyHeight / 2)), is_facing_right);
  std::pair<unsigned int, Enemy*> enemy_data(Entity::GetEntityID(), enemy);
  entity_manager_.insert(enemy_data);
  asleep_enemies_.insert(enemy_data);
}

void MyApp::FlyingEnemyInit(float x_loc, float y_loc, bool is_facing_right){
  FlyingEnemy* enemy = new FlyingEnemy(world_, b2Vec2(x_loc, y_loc + (kEnemyHeight / 2)), is_facing_right);
  std::pair<unsigned int, Enemy*> enemy_data(Entity::GetEntityID(), enemy);
  entity_manager_.insert(enemy_data);
  asleep_enemies_.insert(enemy_data);
}

Hunter* MyApp::HunterInit(float x_loc, float y_loc) {
  Hunter* hunter  = new Hunter(world_, b2Vec2(x_loc, y_loc + (kEnemyHeight / 2)));
  std::pair<unsigned int, Enemy*> enemy_data(Entity::GetEntityID(), hunter);
  entity_manager_.insert(enemy_data);
  asleep_enemies_.insert(enemy_data);
  return hunter;
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

//Uses bottom left corner
void MyApp::RepairInit(float x_loc, float y_loc) {
  Entity* repair_kit = new RepairKit(world_, b2Vec2(x_loc + kEnemyWidth/2, y_loc + kEnemyHeight/2));
  std::pair<unsigned int, Entity*> entity_data(Entity::GetEntityID(), repair_kit);
  entity_manager_.insert(entity_data);
}

//Bottom left corner for start
void MyApp::MovingWallInit(float start_x, float start_y, float width,
                           float height, ci::Color color, bool moves_vertically,
                           float lower_limit, float upper_limit,
                           float velocity) {
  MovingWall* wall = new MovingWall(world_, start_x + width/2, start_y + height/2,
      width/2, height/2, lower_limit, upper_limit, velocity, moves_vertically, color);
  moving_walls_.push_back(wall);
}

//Bottom left corner for start
//This version needs a pointer for an enemy to rely on activation for.
void MyApp::InactiveMovingWallInit(float start_x, float start_y, float width,
                           float height, ci::Color color, bool moves_vertically,
                           float lower_limit, float upper_limit,
                           float velocity, Enemy* activator) {
  MovingWall* wall = new MovingWall(world_, start_x + width/2, start_y + height/2,
                                    width/2, height/2, lower_limit,
                                    upper_limit, velocity, moves_vertically, color, activator);
  moving_walls_.push_back(wall);
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
  //Death by falling Or Spikes
  if (player_position.y < 0 || spike_death_) {
    spike_death_ = false;
    lives_--;
    if (lives_ > 0) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      RespawnPlayer();
    }
  }

  if (player_position.x > end_position_ - kFinishWidth) {
    won_level_ = true;
  }

  for (std::pair< unsigned int, FlyingEnemy*> data_pair : flying_enemies_) {
    data_pair.second->Fly();
  }

  for (MovingWall* movingWall : moving_walls_) {
    movingWall->VelocityUpdate();
  }

  player_->UpdateVelocity();
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
    // TODO figure out how this works with window scaling
    float left_bound = (window_shift_ - kActivateRange)/ kPixelsPerMeter;
    float right_bound = (window_shift_ + kStandardWidth + kActivateRange) / kPixelsPerMeter;
    //kStandard called, since this should use standard dimensions, since box2D doesn't account for scale, and kPPM refers to standard dimensions

    if ((enemy->Activate(left_bound, right_bound))) {
      enemy_shooters_.insert(std::pair< unsigned int, Enemy*> (id, enemy));

      if (enemy->getEnemyType() == EnemyType::flying_enemy) {
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
  for (std::pair<unsigned int, Enemy*> enemy_data : enemy_shooters_) {
    Enemy* enemy = enemy_data.second;
    enemy->UpdateVelocity(); // Update Velocity must be called every time

    if (enemy_shooting_timer_ > 0) {
      continue;
    }

    // Death by Falling for enemies
    // Handled here since enemies are already being iterated through.
    // Also better to do it where it isn't being iterated on every game step,
    // since that is unnecessary. This is simply clean-up.
    if (enemy->getBody()->GetPosition().y < 0) {
      entities_to_destroy_.insert((unsigned int) enemy->getBody()->GetUserData());
    }

    Entity* bullet = enemy->Shoot(world_, player_->getBody()->GetPosition());
    entity_manager_.insert(std::pair<unsigned int, Entity*> (Entity::GetEntityID(), bullet));
  }

  if (enemy_shooting_timer_ == 0) {
    enemy_shooting_timer_ = kEnemyReloadTime;
  }
  enemy_shooting_timer_--;
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

// Assumes checkpoints_ has been properly initialized with at least one location
void MyApp::RespawnPlayer() {
  b2Vec2 position = player_->getBody()->GetPosition();
  world_->DestroyBody(player_->getBody());
  delete player_;
  b2Vec2 checkpoint = checkpoints_[0];

  // checkpoint is set to the furthest to the right point the player has passed
  for (b2Vec2 point : checkpoints_) {
    if (point.x > position.x) {
      break;
    }
    checkpoint = point;
  }

  player_ = new Player(world_, checkpoint.x, checkpoint.y);
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
  ci::gl::setMatricesWindow(getWindowSize());
  RescaleWindow();
  ci::gl::scale(scale_, scale_);

  if (title_screen_) {
    DrawTitleScreen();
    return;
  }

  //the y shift prevents stuff from being cut off at the top when scaling.
  ci::gl::translate(-window_shift_, (((float) (1.0f - scale_)) * EffectiveDimensions::GetEffectiveHeight()) / scale_);

  player_->Draw();

  for (Wall* wall : walls_) {
    wall->Draw();
  }

  for (MovingWall* movingWall : moving_walls_) {
    movingWall->Draw();
  }

  for (std::pair<unsigned int, Entity*> entity_data : entity_manager_) {
    entity_data.second->Draw();
  }

  if (lives_ <= 0) {
    const cinder::ivec2 size = {500, 500};
    const cinder::vec2 center = ci::ivec2(kStandardWidth/2, kStandardHeight/2);

    PrintText("U DED", kRed, size, ci::vec2(center.x + window_shift_, center.y), 100);
  }

  // Life Counter
  const cinder::ivec2 size = {100, 50};
  const cinder::vec2 center(50.0f + window_shift_, EffectiveDimensions::GetEffectiveHeight() - (kStandardHeight - 50.0f)); //Top-left Corner
  PrintText(std::to_string(lives_), kBlue, size, center,50);

  ci::gl::Texture2dRef finish_image;

  if (won_level_) {
    const cinder::ivec2 size = {500, 500};
    const cinder::vec2 center = ci::ivec2(kStandardWidth/2, kStandardHeight/2);

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
  ci::gl::draw(finish_image, ci::vec2((end_position_ - kFinishWidth) * kPixelsPerMeter, EffectiveDimensions::GetEffectiveHeight() - 346)); //TODO see if this needs refactoring?

  //Draws "curtain," the area outside of the effective width
  ci::gl::setMatricesWindow(getWindowSize());
  ci::Rectf curtain (EffectiveDimensions::GetEffectiveWidth(), 0, getWindowWidth(), getWindowHeight());
  ci::gl::color(kBlack);
  ci::gl::drawSolidRect(curtain);

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
  //May seem a bit redundant to define these, but this is written to be hardcoded
  // to the standard dimensions, and then lets the scale and transform functions do the rest
  float center_x = kStandardWidth / 2;
  float center_y = kStandardHeight / 2;


  PrintText("ROBOT REVOLT", kRed, size, ci::ivec2(center_x, center_y - 250), 300);

  PrintText("Use 1, 2, or 3 to select difficulty", kBlue, size, ci::ivec2(center_x, center_y), 120);

  if (repair_value_ == 3) {
    PrintText("EASY", kGreen, size, ci::ivec2(center_x, center_y + 150), 120);
  }

  if (repair_value_ == 2) {
    PrintText("NORMAL", kYellow, size, ci::ivec2(center_x, center_y + 150), 120);
  }

  if (repair_value_ == 1) {
    PrintText("HARD", kRed, size, ci::ivec2(center_x, center_y + 150), 120);
  }

  if (repair_value_ != -1) {
    PrintText("Press Space to Begin", kGreen, size,
              ci::ivec2(center_x, center_y + 350), 120);
  }

  player_->Draw();

  PrintText("Created by Nathan Omerza", kBlue, ci::ivec2(250,250),
      ci::ivec2(kStandardWidth - 250, kStandardHeight - 230), 30);

  PrintText("Music: \"Spectral\" by Kyle Adomaitis", kBlue, ci::ivec2(250,250),
      ci::ivec2(kStandardWidth - 250, kStandardHeight - 150), 30);
}

void MyApp::keyDown(KeyEvent event) {
  b2Body* body = player_->getBody();
  int key = event.getCode();

  if (key == KeyEvent::KEY_UP && sensor_contacts_ >= 1 && can_press_up &&
      (jump_timer_ == 0 || (jump_timer_ > 5 && jump_timer_ <= 28 && body->GetLinearVelocity().y == 0))) {

      b2Vec2 impulse_vector(0.0f, 27.0f); // Allows for ~3m jump
      body->ApplyLinearImpulse(impulse_vector, body->GetPosition());
      jump_timer_ = 38;
      can_press_up = false;
      /* Sensor contacts -> only jump if in contact with ground
       * can_press_up -> simply holding up won't allow for a super jump, so pulling one off would need near perfect timing
       * Timers explanation:
       * Using current player size/density values, jump impulse, and gravity values,
      it would take 9/14 seconds to reach the peak of the jump. at 60 updates
      per second, 38.57 updates to reach the peach, in which the player should
      not be able to jump. The exception would be if the player hits their head on a ceiling
      and falls to the ground faster than expected. This is handled by the or statement,
      which checks if the y velocity is zero, so as to see if the player is on the ground.
      The problem with just that is it would allow the player to jump again exactly at the peak
      of their jump, so the two jump_timer_ ranges next to it will make sure it can only use
      the 0-y velocity rule can only be used if the player has not made a full jump,
      so the ceiling event must have occurred. The only edge case for this is if the
      head-on-ceiling scenario occurs while the player is on a vertically-moving platform,
      but since all moving walls have spikes on them at the moment, this won't occur.
       */

  } else if (key == KeyEvent::KEY_RIGHT) {
      player_->setRelativeVelocity(7);
      player_->setFacingRight(true);
  } else if (key == KeyEvent::KEY_LEFT) {
      player_->setRelativeVelocity(-7);
      player_->setFacingRight(false);
  }
  //These 3 are for choosing the game difficulty
  else if (key == KeyEvent::KEY_1 && title_screen_) {
    repair_value_ = 1;
  } else if (key == KeyEvent::KEY_2 && title_screen_) {
    repair_value_ = 2;
  } else if (key == KeyEvent::KEY_3 && title_screen_) {
    repair_value_ = 3;
  } else if (key == KeyEvent::KEY_SPACE && shooting_timer_ == 0) {
    if (title_screen_ && repair_value_ != -1) {
      title_screen_ = false;
      music_player_->start();
      return;
    } else if (title_screen_) {
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
    lives_ = kStartingLives;
    Restart();

  } else if (event.getCode() == KeyEvent::KEY_f) {
    // Toggles fullscreen
    setFullScreen(!isFullScreen());
  }
}

void MyApp::keyUp(cinder::app::KeyEvent event) {
  b2Body* body = player_->getBody();

  if (event.getCode() == KeyEvent::KEY_LEFT
    || event.getCode() == KeyEvent::KEY_RIGHT) {

      player_->setRelativeVelocity(0);
  }

  if (event.getCode() == KeyEvent::KEY_UP) {
    can_press_up = true;
  }
}

void MyApp::DestroyEntity(unsigned int entity_ID){
  Entity* entity = entity_manager_.at(entity_ID);

  if (entity->GetEntityType()  == mylibrary::type_enemy) {
    Enemy* enemy = (Enemy*) entity;
    if (enemy->isActive()) {
      enemy_shooters_.erase(entity_ID);

      if (enemy->getEnemyType() == EnemyType::flying_enemy) {
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

void MyApp::BulletCollision(b2Fixture* bullet_fix, b2Fixture* other) {
  unsigned int bullet_ID = (unsigned int) bullet_fix->GetUserData();
  Bullet* bullet_obj = (Bullet*) entity_manager_.at(bullet_ID);

  if (bullet_obj->isSuper()) {
    int bullet_hits = ((SuperBullet*) bullet_obj)->Rebound();

    //Superbullet shouldn't bounce off of player
    if (bullet_hits == 0 || other->GetBody() == player_->getBody()) {
      entities_to_destroy_.insert(bullet_ID);
    }
  } else {
    entities_to_destroy_.insert(bullet_ID);
  }

  if (other->GetBody() == player_->getBody()) {
    lives_--;
    damage_sound_->start();
  }

  if (isEntityFixture(other)) { //Therefore, other must be an entity
    unsigned int other_ID = (unsigned int) other->GetUserData();
    Entity* entity = entity_manager_.at(other_ID);

    //Can't shoot Repair kits
    if (entity->GetEntityType() == EntityType::type_repair) {
      return;
    }

    // If it is an enemy, it must be removed from the enemy_shooters_ map
    if (entity->GetEntityType() == EntityType::type_enemy) {

      // This prevents friendly-fire between enemies
      if (!(((Bullet*) entity_manager_.at(bullet_ID))->isPlayerMade())) {
        return;
      }

      // At this point, it is a valid shot against an enemy.
      boom_sound_->start();
    }

    entities_to_destroy_.insert(other_ID);
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

    // if the other (non-sensor) object is a moving wall, the players velocity must be updated so the player moves relative to the moving wall
    if (fixture_A->GetUserData() != NULL && (int) fixture_A->GetUserData() == mylibrary::kMovingWallID) {
      myApp_->player_->setMovingWallContact(fixture_A->GetBody());
    }
    if (fixture_B->GetUserData() != NULL && (int) fixture_B->GetUserData() == mylibrary::kMovingWallID) {
      myApp_->player_->setMovingWallContact(fixture_B->GetBody());
    }

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

  b2Body* player_body = myApp_->player_->getBody();

  if ((fixture_A->GetUserData() != NULL && (unsigned int) fixture_A->GetUserData() == mylibrary::kSpikeID && fixture_B->GetBody() == player_body)
        || (fixture_B->GetUserData() != NULL && (unsigned int) fixture_B->GetUserData() == mylibrary::kSpikeID && fixture_A->GetBody() == player_body)) {
    myApp_->spike_death_ = true;
  }

  // The Next two determine if it is an entity. Each contains two parts:
  // - for when an enemy collides with a non-bullet
  // (which it can't be if it reached this point), non-player object
  // - when a player collides with a repair kit
  if (isEntityFixture(fixture_A)) {
    Entity* entity =
        myApp_->entity_manager_.at((unsigned int) fixture_A->GetUserData());

    if (entity->GetEntityType() == EntityType::type_enemy) {

      if (((Enemy*) entity)->getEnemyType() != EnemyType::hunter && fixture_B->GetBody() != player_body) {
        ((Enemy*) entity)->TurnAround();
      }

      //Enemy on moving wall:
      if (fixture_B->GetUserData() != NULL && (int) fixture_B->GetUserData() == mylibrary::kMovingWallID) {
        ((Enemy*) entity)->setMovingWallContact(fixture_B->GetBody());
      }
    }

    if (entity->GetEntityType() == EntityType::type_repair && fixture_B->GetBody() == player_body) {
      myApp_->entities_to_destroy_.insert((unsigned int) fixture_A->GetUserData());
      myApp_->lives_ += myApp_->repair_value_;
    }
  }

  if (isEntityFixture(fixture_B)) {
    Entity* entity =
        myApp_->entity_manager_.at((unsigned int) fixture_B->GetUserData());

    if (entity->GetEntityType() == EntityType::type_enemy) {

      if (((Enemy*) entity)->getEnemyType() != EnemyType::hunter && fixture_A->GetBody() != player_body) {
        ((Enemy*) entity)->TurnAround();
      }

      //Enemy on moving wall:
      if (fixture_A->GetUserData() != NULL && (int) fixture_A->GetUserData() == mylibrary::kMovingWallID) {
        ((Enemy*) entity)->setMovingWallContact(fixture_A->GetBody());
      }
    }

    if (entity->GetEntityType() == EntityType::type_repair && fixture_A->GetBody() == player_body) {
      myApp_->entities_to_destroy_.insert((unsigned int) fixture_B->GetUserData());
      myApp_->lives_ += myApp_->repair_value_;
    }
  }
}

void MyApp::ContactListener::EndContact(b2Contact* contact) {
  b2Fixture* fixture_A = contact->GetFixtureA();
  b2Fixture* fixture_B = contact->GetFixtureB();
    //check if either fixture is the foot sensor
    if ((int)fixture_A->GetUserData() == kFootSensorID
      || (int)fixture_B->GetUserData() == kFootSensorID) {

      // if the other (non-sensor) object is a moving wall, the players velocity must be updated so the player moves relative to the moving wall
      if ((fixture_A->GetUserData() != NULL && (int) fixture_A->GetUserData() == mylibrary::kMovingWallID)
      || (fixture_B->GetUserData() != NULL && (int) fixture_B->GetUserData() == mylibrary::kMovingWallID)) {
        myApp_->player_->setMovingWallContact(nullptr);
      }

      myApp_->sensor_contacts_--;
    }

    //Check for enemies leaving moving walls:

  if (isEntityFixture(fixture_A)) {
    Entity* entity =
        myApp_->entity_manager_.at((unsigned int) fixture_A->GetUserData());

    if (entity->GetEntityType() == EntityType::type_enemy
        && fixture_B->GetUserData() != NULL
        && (int) fixture_B->GetUserData() == mylibrary::kMovingWallID) {

      ((Enemy*) entity)->setMovingWallContact(nullptr);
    }
  }

  if (isEntityFixture(fixture_B)) {
    Entity* entity =
        myApp_->entity_manager_.at((unsigned int) fixture_B->GetUserData());

    if (entity->GetEntityType() == EntityType::type_enemy
        && fixture_A->GetUserData() != NULL
        && (int) fixture_A->GetUserData() == mylibrary::kMovingWallID) {

      ((Enemy*) entity)->setMovingWallContact(nullptr);
    }
  }
}

void MyApp::Restart() {
  delete world_;  // Also deletes every b2body in the world
  delete player_;

  sensor_contacts_ = 0;
  can_press_up = true;

  for (std::pair<unsigned int, Entity*> entity_data : entity_manager_) {
    delete entity_data.second;
  }
  entity_manager_.clear();

  for (Wall* wall : walls_) {
    delete wall;
  }
  walls_.clear();

  for (MovingWall* movingWall : moving_walls_) {
    delete movingWall;
  }
  moving_walls_.clear();

  // Members deleted in entity_manager_
  enemy_shooters_.clear();
  asleep_enemies_.clear();
  entities_to_destroy_.clear();
  flying_enemies_.clear();

  Entity::ResetID();

  jump_timer_ = 0;
  shooting_timer_ = 0;
  enemy_shooting_timer_ = kEnemyReloadTime;
  window_shift_ = 0;

  won_level_ = false;
  spike_death_ = false;

  checkpoints_.clear();

  setup();
}

void MyApp::LevelZero() {
  PlayerWorldInit(1, 4);
  window_shift_ = 0;
  left_window_bound_ = window_shift_;
  won_level_ = false;

  checkpoints_.push_back(b2Vec2(1,4));

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
  checkpoints_.push_back(b2Vec2(43,4));
  WallInit(47, 3.5f, 2, 1, kLightBlue);
  GroundInit(55.5f, 61);
  EnemyInit(60, 0.4f, true);

  // Separated Stairs
  WallInit(66.1f, 1, 2, 2, kOrange);
  RepairInit(67, 3.2f);
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

  checkpoints_.push_back(b2Vec2(125,4));
  RepairInit(125, 1);
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
  checkpoints_.push_back(b2Vec2(91,4));
  RepairInit(93, 2);

  // Intro to jetpacks
  GroundInit(90, 119);
  WallInit(95, 0, 1.5f, 3, kShallowRed);
  FlyingEnemyInit(102, 0.5f, true);
  FlyingEnemyInit(106.5f, 0.5f, true);
  WallInit(110, 0, 3, 3, kShallowRed);
  WallInit(113, 0, 3, 6, kShallowRed);
  WallInit(116, 0, 3, 9, kShallowRed);
  checkpoints_.push_back(b2Vec2(117.5f,9.5f));

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

  RepairInit(184, 1);

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

void MyApp::LevelTwo(){
  PlayerWorldInit(1, 4);
  window_shift_ = 0;
  left_window_bound_ = 0;
  won_level_ = false;

  WallInit(-0.1f, 0, 0.1f, getWindowHeight()/kPixelsPerMeter, kYellow);
  checkpoints_.push_back(b2Vec2(1,4));
  RepairInit(3, 2);

  // First Hunter
  GroundInit(0, 53);
  WallInit(4, 1.8f, 1.5f, 9.5f, kHunterGreen);
  WallInit(7, 11, 10, 0.3f, kHunterGreen);
  WallInit(14, 2, 1, .5f, kHunterGreen);
  WallInit(11.2f, 3.7f, 1, 0.5f, kHunterGreen);
  WallInit(9, 5, 1, 1, kHunterGreen);
  HunterInit(9.5f, 6.2f);
  WallInit(6, 8.7f, 1, .5f, kHunterGreen);
  WallInit(17, 0, 2, 11.3f, kHunterGreen);

  // First gap
  int kGapOne = 15;
  WallInit(19, 0, kGapOne, 5, kYellow);
  EnemyInit(23, 5.2f, true);
  WallInit(24, 1.8f, 2, 1, kYellow);
  EnemyInit(27.5f, 5.2f, true);
  WallInit(29, 5, 1, 2, kYellow);
  FlyingEnemyInit(31, 5.2f, true);
  WallInit(32, 5, 2, 3, kYellow);
  RepairInit(33, 9.8f);

  // Triple combo stairs
  WallInit(25+kGapOne, 2, 2, 1, kBlueGray);
  FlyingEnemyInit(26+kGapOne, 3.3f, true);
  EnemyInit(29+kGapOne, 2, true);
  WallInit(31+kGapOne, 2, 3, 1, kBlueGray);
  FlyingEnemyInit(31.5f + kGapOne, 3.1f, true);
  WallInit(33+kGapOne, 4.5f, 3, 1, kBlueGray);
  EnemyInit(34+kGapOne, 5, true);
  WallInit(35+kGapOne, 7, 3, 1, kBlueGray);
  HunterInit(35+kGapOne, 8.1f);
  WallInit(36+kGapOne, 0, 1, 7, kBlueGray);

  // SecondGap
  int kGapTwo = 31 + kGapOne;
  WallInit(37+kGapOne, 0, kGapTwo - kGapOne, 7, kGreen);
  WallInit(40 + kGapOne, 7, 1, 2, kGreen);
  EnemyInit(43+kGapOne, 7.1f, true);
  FlyingEnemyInit(45+kGapOne, 7.1f, false);
  EnemyInit(47+kGapOne, 7.1f, true);
  WallInit(49+kGapOne, 7, 4, 2, kGreen);
  RepairInit(51+kGapOne, 10.7f);
  FlyingEnemyInit(55+kGapOne, 7.1f, false);
  WallInit(57+kGapOne, 9, 2, 1, kGreen);
  EnemyInit(59+kGapOne, 7.1f, true);
  FlyingEnemyInit(61+kGapOne, 7.1f, true);
  WallInit(63+kGapOne, 7, 1, 4.3f, kGreen);

  checkpoints_.push_back(b2Vec2(65+kGapOne,4.8f));

  // Floating hunter zone
  WallInit(42+kGapTwo, 8.8f, 0.5f, 3.2f, kCyan); //Hunter shield
  WallInit(40+kGapTwo, 4, 2, 1, kCyan);
  WallInit(44+kGapTwo, 1.2f, 1.2f, 1, kCyan);
  WallInit(46+kGapTwo, 7.2f, 1.5f, 1, kCyan);
  WallInit(50+kGapTwo, 8.5f, 0.5f, 1, kCyan);
  HunterInit(50.25f+kGapTwo, 9.55f);
  WallInit(49+kGapTwo, 4.6f, 1.4f, 0.7f, kCyan);
  WallInit(55.5f+kGapTwo, 2.5f, 1.4f, 0.7f, kCyan);
  WallInit(58.2f+kGapTwo, 5.1f, 1.2f, 0.6f, kCyan);
  HunterInit(58.8f+kGapTwo, 6.4f);
  GroundInit(62+kGapTwo, 70+kGapTwo);
  end_position_ = 70+kGapTwo;
}

void MyApp::LevelThree() {
  PlayerWorldInit(1, 4);
  window_shift_ = 0;
  left_window_bound_ = 0;
  won_level_ = false;

  WallInit(-0.1f, 0, 0.1f, getWindowHeight()/kPixelsPerMeter, kYellow);
  checkpoints_.push_back(b2Vec2(1,4));
  RepairInit(3, 2);

  //Opening Diamond Cage
  GroundInit(0, 50);
  WallInit(5, 1.5f, 0.6f, 10, kGoldGreen);
  WallInit(5, 11, 22, 0.5f, kGoldGreen);
  WallInit(7, 0.2f, 0.4f, 1, kGoldGreen);
  EnemyInit(9, 0.5f, false);
  EnemyInit(16, 0.5f, true);
  EnemyInit(23, 0.5f, true);
  WallInit(10, 5.9f, 2, 0.5f, kGoldGreen);
  FlyingEnemyInit(11, 6.6f, true);
  WallInit(15, 2.7f, 2, 0.5f, kGoldGreen);
  WallInit(15, 9.3f, 2, 0.5f, kGoldGreen);
  FlyingEnemyInit(16, 9.8f, false);
  WallInit(20,  5.9f, 2, 0.5f, kGoldGreen);
  FlyingEnemyInit(21, 6.2f, true);
  WallInit(24.6f, 0.2f, 0.4f, 1, kGoldGreen);
  WallInit(26.4f, 1.5f, 0.6f, 10, kGoldGreen);
  //Secret Repair Kit
  RepairInit(15, 11.9f);

  //Reverse Hunter Stairs
  RepairInit(28, 2);
  WallInit(27, 9.5f, 1.2f, 0.4f, kHunterGreen);
  HunterInit(27.5f, 10);
  WallInit(28.2f, 9.5f, 0.1f, 0.7f, kHunterGreen); //Hunter's safety railing
  WallInit(33, 0, 5, 3, kHunterGreen);
  EnemyInit(37.3f, 3.1f, false);
  WallInit(38, 0, 5, 6, kHunterGreen);
  FlyingEnemyInit(40, 6.2f, true);
  EnemyInit(42.4f, 6.2f, false);
  WallInit(35, 8.8f, 1, 0.2f, kHunterGreen);
  WallInit(43, 0, 5, 9, kHunterGreen);
  EnemyInit(47.2f, 9.1f, false);
  WallInit(48, 0, 2, 11, kHunterGreen);

  //Small Gap
  checkpoints_.push_back(b2Vec2(49, 11.1f));
  WallInit(57.2f, 6.7f, 2, 0.7f, kDarkBlue);
  RepairInit(58.2f, 7.7f);
  WallInit(65.3f, 2.4f, 1.3f, 0.7f, kDarkBlue);

  //Hunter Fortress
  int back_shift = 73 - 157; // This was copied from levelzero's "the pit," so it must be shifted. Uses the start of ground in this - start of ground in level 0
  GroundInit(73, 260+back_shift);
  WallInit(164+back_shift, 0, 2, 3, kGray);
  WallInit(166+back_shift, 0, 2, 6, kGray);
  WallInit(168+back_shift, 0, 2, 9, kGray);
  WallInit(170+back_shift, 0, 8, 5, kGray);
  HunterInit(171+back_shift, 5.1f);
  EnemyInit(175+back_shift, 5.1f, true);
  WallInit(177.9f+back_shift, 5, 0.1f, 0.4f, kGray);
  WallInit(179.4f+back_shift, 2, 1.6f, 3, kGray);
  WallInit(179.4f+back_shift, 5, 0.1f, 0.4f, kGray);
  HunterInit(180+back_shift, 5.1f);
  WallInit(181+back_shift, 2, 2, 9, kGray);

  //Hunting range with jetpacks
  WallInit(192+back_shift, 0, 1.5f, 2, kDeepRed);
  FlyingEnemyInit(196+back_shift, 1, true);
  FlyingEnemyInit(199+back_shift, 1, true);
  FlyingEnemyInit(202+back_shift, 1, true);
  FlyingEnemyInit(205+back_shift, 1, true);
  WallInit(209+back_shift, 0, 1.5f, 3, kDeepRed);
  HunterInit(209.7f+back_shift, 3.1f);

  //Hunter's Pyramid
  back_shift = 215+back_shift - 217;
  RepairInit(214+back_shift, 2);
  WallInit(217+back_shift, 1.5f, 1, 12, kOrange);
  WallInit(221+back_shift, 0, 21, 2, kOrange);
  WallInit(224+back_shift, 0, 15, 4, kOrange);
  WallInit(227+back_shift, 0, 9, 6, kOrange);
  WallInit(230+back_shift, 0, 3, 8, kOrange);
  WallInit(246+back_shift, 1.5f, 1, 12, kOrange);
  WallInit(218+back_shift, 11.8f, 28, 0.3f, kOrange);
  FlyingEnemyInit(220+back_shift, 1, false);
  FlyingEnemyInit(226+back_shift, 6, true);
  EnemyInit(228+back_shift, 6.3f, true);
  HunterInit(230.5f+back_shift, 8.5f);
  HunterInit(232.5f+back_shift, 8.5f);
  EnemyInit(235+back_shift, 6.3f, false);
  FlyingEnemyInit(238+back_shift, 4.5f, false);
  FlyingEnemyInit(244+back_shift, 0.5f, true);

  end_position_ = 255+back_shift;

}

void MyApp::LevelFour() {
  PlayerWorldInit(1, 4);
  checkpoints_.push_back(b2Vec2(1,4));
  window_shift_ = 0;
  left_window_bound_ = 0;
  won_level_ = false;
  WallInit(-0.2, 0, 0.2, 12, kBlack);

  //Intro to moving walls
  GroundInit(0, 10);
  RepairInit(6, 2);
  MovingWallInit(10, 2, 2, 1, kRed, false, 10, 17, 4);
  MovingWallInit(22, 5, 2, 1, kRed, false, 17, 24, 4);
  WallInit(25, 0, 2, 9.5f, kRed);
  GroundInit(25, 59);

  // Falling walls and gunfire
  checkpoints_.push_back(b2Vec2(27.7f, 2.3f));
  MovingWallInit(37, 4, 2, 6, kGreen, true, 0.2f, 11, 5);
  MovingWallInit(42, 1, 2, 6, kGreen, true, 0.2f, 11, 5);
  EnemyInit(45, 0.4f, true);
  MovingWallInit(47, 5, 2, 6, kGreen, true, 0.2f, 11, 5);
  MovingWallInit(52, 2, 2, 6, kGreen, true, 0.2f, 11, 5);
  EnemyInit(56, 0.5f, true);
  WallInit(57, 0, 2, 3, kGreen);

  //Falling walls on moving platforms
  checkpoints_.push_back(b2Vec2(60, 6.5f));
  WallInit(59, 0, 2, 6, kDarkBlue);
  MovingWallInit(61, 5, 5, 1, kDarkBlue, false, 61, 75.5f, 2);
  MovingWallInit(66, 10, 2, 4, kDeepRed, true, 6, 14, 4);
  MovingWallInit(71, 6, 2, 4, kDeepRed, true, 6, 14, 4);
  MovingWallInit(78, 10, 2, 4, kDeepRed, true, 6, 14, 4);
  MovingWallInit(83, 6, 2, 4, kDeepRed, true, 6, 14, 4);
  MovingWallInit(85, 5, 5, 1, kDarkBlue, false, 75.5f, 90, 2);
  WallInit(90, 0, 2, 9, kDarkBlue);

  // Floating Repair kit
  checkpoints_.push_back(b2Vec2(91, 10));
  GroundInit(90, 220);
  MovingWallInit(92, 8, 2, 1, kGold, false, 92, 101, 4);
  RepairInit(100.5f, 11.3f);
  EnemyInit(95, 0.5f, true);
  EnemyInit(104, 0.3f, true);
  WallInit(107, 0, 1, 3, kGold);

  // Flying Enemy Cage, with moving walls
  checkpoints_.push_back(b2Vec2(107.5f, 4));
  int shift = -74;
  WallInit(188+shift, 1.5, 2, 10.3f, kMiddleBlue); //left
  WallInit(206+shift, 0, 2, 9.2f, kMiddleBlue); //right

  MovingWallInit(204+shift, 1, 2, 1, kMiddleBlue, false, 190+shift, 206+shift, 2);
  MovingWallInit(192+shift, 4.2, 2, 1, kMiddleBlue, false, 190+shift, 206+shift, 4.2);
  MovingWallInit(198+shift, 7.3f, 2, 1, kMiddleBlue, false, 190+shift, 206+shift, 3.3);

  WallInit(203+shift, 9.2f, 5, 0.5f, kMiddleBlue);
  WallInit(188+shift, 11.8f, 20, 0.3f, kMiddleBlue);//ceiling
  FlyingEnemyInit(195+shift, 1, true);
  FlyingEnemyInit(202+shift, 0.5f, false);
  FlyingEnemyInit(192.5f+shift, 6, false);
  FlyingEnemyInit(205+shift, 10, true);

  end_position_ = 220+shift;
}

void MyApp::LevelFive() {
  PlayerWorldInit(1, 4);
  checkpoints_.push_back(b2Vec2(1,4));
  window_shift_ = 0;
  left_window_bound_ = 0;
  won_level_ = false;
  WallInit(-0.2, 0, 0.2, 12, kBlack);

  //Moving Hunter
  GroundInit(0, 26);
  Hunter* first_hunter = HunterInit(4.5, 10);
  InactiveMovingWallInit(4, 9, 1, 1, kHunterGreen, false, 4, 18, 3, first_hunter);
  WallInit(20.5, 8, 1, 3, kHunterGreen);
  RepairInit(3.4, 3);
  WallInit(19, 3, 2, 0.5, kHunterGreen);
  WallInit(17, 5, 0.7, 2, kHunterGreen);
  WallInit(13, 0.2, 1.3, 2.6, kHunterGreen);
  WallInit(6, 11.6, 16, 0.3, kHunterGreen);
  WallInit(0, 11.6, 1.4, 4, kHunterGreen);
  RepairInit(1, 11);

  // Moving ladder space
  checkpoints_.push_back(b2Vec2(23,1));
  MovingWallInit(26, -0.5, 3, 1, kOrange, false, 26, 37, 3);
  MovingWallInit(34, 2, 2, 0.7, kOrange, false, 32, 43.5, 4);
  MovingWallInit(23, 4, 2.4, 1, kOrange, false, 27, 35, 4.5);
  MovingWallInit(32, 7, 1.9, 0.5, kOrange, false, 22, 29, 2.8);
  MovingWallInit(29, 9.2, 1.5, .7, kOrange, false, 29, 36, 3.4);
  MovingWallInit(35, 10.5, 2.3, 0.6, kOrange, false, 35, 43, 3.6);
  WallInit(44, 0, 2, 11.6, kOrange);

  //Descent
  checkpoints_.push_back(b2Vec2(44.5, 12.2));
  WallInit(51, 8, 2, 1, kPurple);
  RepairInit(60, 9);
  WallInit(60, 5, 2, 1, kPurple);
  GroundInit(68, 95);

  //Low flying hunter
  checkpoints_.push_back(b2Vec2(69, 1));
  EnemyInit(75, 0.5, true);
  EnemyInit(77, 0.2, false);
  EnemyInit(78.5, 0.5, false);
  EnemyInit(79.8, 0.2, false);
  EnemyInit(81, 0.5, true);
  WallInit(84, 0, 1, 3, kGoldGreen);
  Hunter* low_hunter = HunterInit(76.5, 2.5);
  InactiveMovingWallInit(76, 1.5, 1, 1, kGoldGreen, false, 72, 84, 3.6, low_hunter);

  end_position_ = 95;
}

bool MyApp::isEntityFixture(b2Fixture* b2Fixture) {
  return b2Fixture->GetUserData() != NULL && (int) b2Fixture->GetUserData() > 0;
}
}  // namespace myapp
