// Copyright (c) 2020 CS126SP20. All rights reserved.

#ifndef FINALPROJECT_APPS_MYAPP_H_
#define FINALPROJECT_APPS_MYAPP_H_

#include <Box2D/Box2D.h>
#include <cinder/app/App.h>
#include <mylibrary/Bullet.h>
#include <mylibrary/player.h>
#include <mylibrary/enemy.h>
#include <mylibrary/Wall.h>

#include <set>

extern int sensor_contacts; //TODO is global ok? and should it be in this file?

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

  b2World* world;
  mylibrary::Player* player_;
  std::map<unsigned int, mylibrary::Entity*> entity_manager_;
  std::map<unsigned int, mylibrary::Enemy*> enemy_shooters_;
  std::map<unsigned int, mylibrary::Enemy*> asleep_enemies_;

  std::vector<mylibrary::Wall*> walls_;

  // Bodies can't be destroyed in callback, so must be tracked here to be destroyed later.
  // Uses a set, since a body can collide with two things within the same step,
  // and therefore could have the same body attempting to be destroyed after it has already been destroyed
  std::set<unsigned int> entities_to_destroy_;

  int lives_;

  //The player may still be able to get a large jump by holding down the up key
  //even with the contact listener, so this gives a slight cooldown time to
  //prevent it. //todo shorten and clarify comment
  int jump_timer; //TODO refactor these to goog style guide

  //Require reload time in between shots.
  int shooting_timer;

  int enemy_shooting_timer_;

  int scope_x_;

  int sensor_contacts;

  float end_position_;
  float finish_width_;
  bool won_game;
  bool developer_mode;

  //This is a declaration for the subclass
  class ContactListener : public b2ContactListener { //TODO see if it is ok to have 2 classes in one file (OR subclass 'cause I guess that's what I'm doing)?
    void BeginContact(b2Contact* contact);
    void EndContact(b2Contact* contact);
   public:
    MyApp* myApp;
  };

  //This is a private member variable of MyApp
  ContactListener contactListener;
};

}  // namespace myapp

#endif  // FINALPROJECT_APPS_MYAPP_H_
