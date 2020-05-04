// Copyright (c) 2020 CS126SP20. All rights reserved.

#ifndef FINALPROJECT_APPS_MYAPP_H_
#define FINALPROJECT_APPS_MYAPP_H_

#include <set>
#include <Box2D/Box2D.h>
#include <cinder/app/App.h>
#include <cinder/audio/audio.h>
#include <mylibrary/Bullet.h>
#include <mylibrary/player.h>
#include <mylibrary/enemy.h>
#include <mylibrary/flying_enemy.h>
#include <mylibrary/Wall.h>

namespace myapp {

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
  void EndGame();

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

  void AudioSetup();

  // These are helper functions to Update()
  void ActivateEnemies();
  void UpdateActiveEnemies();
  void ScrollWindow();

  // These functions simplify level design, so every element can be added
  // by only calling this function
  void EnemyInit(float x_loc, float y_loc, bool is_facing_right);
  void WallInit(float x_loc, float y_loc, float width, float height, ci::Color color);
  void GroundInit(float start, float end);

  b2World* world_;
  mylibrary::Player* player_;
  std::map<unsigned int, mylibrary::Entity*> entity_manager_;
  std::map<unsigned int, mylibrary::Enemy*> enemy_shooters_;
  std::map<unsigned int, mylibrary::Enemy*> asleep_enemies_;
  std::map<unsigned int, mylibrary::FlyingEnemy*> flying_enemies_;

  std::vector<mylibrary::Wall*> walls_;

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

  int lives_;

  //The player may still be able to get a large jump by holding down the up key
  //when near the ground, so this prevents that by giving a cooldown.
  int jump_timer_;

  //Require reload time in between shots.
  int shooting_timer_;
  int enemy_shooting_timer_;

  int window_shift_;
  int sensor_contacts_;
  float end_position_;
  float finish_width_;
  bool won_game_;
  bool developer_mode_;
  bool title_screen_;

  //This is a declaration for the internal ContactListener Class
  class ContactListener : public b2ContactListener {
    void BeginContact(b2Contact* contact);
    void EndContact(b2Contact* contact);

   public:
    // Needs to be able to call functions that change state of MyApp.
    MyApp* myApp_;
  };

  //This is a private member variable of MyApp
  ContactListener contact_listener_;
};

}  // namespace myapp

#endif  // FINALPROJECT_APPS_MYAPP_H_
