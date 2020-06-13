// Copyright (c) 2020 CS126SP20. All rights reserved.

#ifndef FINALPROJECT_APPS_MYAPP_H_
#define FINALPROJECT_APPS_MYAPP_H_

#include <set>
#include <Box2D/Box2D.h>
#include <cinder/app/App.h>
#include <cinder/audio/audio.h>
#include <mylibrary/Bullet.h>
#include <mylibrary/SuperBullet.h>
#include <mylibrary/player.h>
#include <mylibrary/enemy.h>
#include <mylibrary/flying_enemy.h>
#include <mylibrary/Hunter.h>
#include <mylibrary/Wall.h>
#include <mylibrary/MovingWall.h>
#include <mylibrary/RepairKit.h>

namespace myapp {

const int kStandardWidth = 1920;
const int kStandardHeight = 1080;

class MyApp : public cinder::app::App {

 public:
  MyApp();
  void setup() override;
  void update() override;
  void draw() override;
  void keyDown(cinder::app::KeyEvent) override;
  void keyUp(cinder::app::KeyEvent) override;

  /**
   * Handles Collision response if a bullet is involved
   * public since it needs to be called by ContactListener class
   * @param bullet - the fixture in the collision that is a bullet
   * @param other - the fixture the bullet collided with
   * */
  void BulletCollision(b2Fixture* bullet, b2Fixture* other);

 private:
  void DestroyEntity(unsigned int entity_ID);

  /**
   * Deletes all data allocated on the heap
   * Reassigns data to starting values found in constructor
   * Calls Setup();
   * Used to restart game.
   * */
  void Restart();

  /**
   * For use of level designer, prints x and y coordinates in meters across screen
   * x limits are meant to be modified. Game lags if range is more than 25.
   * */
  void DrawDeveloperMode();
  void DrawTitleScreen();

  // Sets scale_ to a new value depending on the window's size.
  void RescaleWindow();

  void AudioSetup();
  void LevelZero();
  void LevelOne();
  void LevelTwo();
  void LevelThree();
  void LevelFour();

  // These are helper functions to Update()
  void ActivateEnemies();
  void UpdateActiveEnemies();
  void ScrollWindow();
  void RespawnPlayer();

  // These functions simplify level design, so every element can be added
  // by only calling this function
  void EnemyInit(float x_loc, float y_loc, bool is_facing_right);
  void FlyingEnemyInit(float x_loc, float y_loc, bool is_facing_right);
  void HunterInit(float x_loc, float y_loc);
  void WallInit(float x_loc, float y_loc, float width, float height, ci::Color color);
  void GroundInit(float start, float end);
  void PlayerWorldInit(float x_loc, float y_loc);
  void RepairInit(float x_loc, float y_loc);
  void MovingWallInit(float start_x, float start_y, float width, float height,
      ci::Color color, bool moves_vertically, float lower_limit, float upper_limit, float velocity);

  // takes a b2fixture, returns true if it is an entity (meaning its user data is a positive value)
  static bool isEntityFixture(b2Fixture*);

  b2World* world_;
  mylibrary::Player* player_;
  std::map<unsigned int, mylibrary::Entity*> entity_manager_;
  std::map<unsigned int, mylibrary::Enemy*> enemy_shooters_;
  std::map<unsigned int, mylibrary::Enemy*> asleep_enemies_;
  std::map<unsigned int, mylibrary::FlyingEnemy*> flying_enemies_;

  std::vector<mylibrary::Wall*> walls_;
  std::vector<mylibrary::MovingWall*> moving_walls_;

  // Bodies can't be destroyed in callback, so must be tracked here to be destroyed later.
  // Uses a set, since a body can collide with two things within the same step,
  // and therefore could have the same body attempting to be destroyed after it has already been destroyed
  std::set<unsigned int> entities_to_destroy_;

  ci::audio::VoiceSamplePlayerNodeRef music_player_;
  ci::audio::VoiceRef laser_sound_;
  ci::audio::VoiceRef boom_sound_;
  ci::audio::VoiceRef damage_sound_;

  ci::gl::Texture2dRef finish_line_bot_;
  ci::gl::Texture2dRef finish_line_bot_win_;

  int repair_value_;
  int level_;
  int lives_;

  /**
   * Coordinates of locations for player to respawn after death by falling
   * At least one checkpoint must be given. Checkpoints must be ordered by
   * increasing x value.
   * */
  std::vector<b2Vec2> checkpoints_;

  //The player may still be able to get a large jump by holding down the up key
  //when near the ground, so this prevents that by giving a cooldown.
  int jump_timer_;

  //Require reload time in between shots.
  int shooting_timer_;
  int enemy_shooting_timer_;

  int sensor_contacts_;
  bool can_press_up; // ensures the player cannot jump, unless they have unpressed and repressed up first.

  int window_shift_;
  int left_window_bound_;
  float end_position_;
  bool won_level_;
  bool developer_mode_;
  bool title_screen_;
  bool spike_death_; // limited use variable to label the player as dying from spikes, but not doing anything until update()

  //The factor that the game is scaled by to fit the view window
  float scale_;
  //True if the horizontal has a smaller current size / standard size ratio than the vertical
  bool is_x_limiting_;

  /**
   * The Internal ContactListener class is a child of the b2ContactListener class
   * This class manages all collisions between objects
   * This allows us to define specific object behavior on collision
   * */
  class ContactListener : public b2ContactListener {
    /**
     * These functions allow us to define object interaction at the beginning
     * and end of a collision
     * These are automatically called when objects collide (or stop colliding)
     * during world_->step(), which is called in ci::update()
     * */
    void BeginContact(b2Contact* contact);
    void EndContact(b2Contact* contact);

   public:
    // Needs to be able to call functions that change state of MyApp.
    MyApp* myApp_;
  };

  //This is a private member variable of MyApp, type is the class just declared
  ContactListener contact_listener_;
};

}  // namespace myapp

#endif  // FINALPROJECT_APPS_MYAPP_H_
