// Copyright (c) 2020 CS126SP20. All rights reserved.

#ifndef FINALPROJECT_APPS_MYAPP_H_
#define FINALPROJECT_APPS_MYAPP_H_

#include <cinder/app/App.h>
#include <Box2D/Box2D.h>
#include <mylibrary/player.h>
#include <mylibrary/Bullet.h>

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

 protected:
  //TODO better name for sensor_contacts?
  //int sensor_contacts; // TODO is making this static ok / will it cause problems

 private:
  b2World* world;
  b2Body* centerBody;
  mylibrary::Player* player_;
  std::vector<mylibrary::Bullet> bullet_manager_;

  //The player may still be able to get a large jump by holding down the up key
  //even with the contact listener, so this gives a slight cooldown time to
  //prevent it. //todo shorten and clarify comment
  int jump_timer;

  //This is a declaration for the subclass
  class ContactListener : public b2ContactListener { //TODO see if it is ok to have 2 classes in one file (OR subclass 'cause I guess that's what I'm doing)?
    void BeginContact(b2Contact* contact);
    void EndContact(b2Contact* contact);
  };

  //This is a private member variable of MyApp
  ContactListener contactListener;

};

}  // namespace myapp

#endif  // FINALPROJECT_APPS_MYAPP_H_
