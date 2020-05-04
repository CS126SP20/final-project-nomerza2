//
// Created by natha on 5/3/2020.
//

#ifndef FINALPROJECT_FLYING_ENEMY_H
#define FINALPROJECT_FLYING_ENEMY_H

#include <mylibrary/enemy.h>

namespace mylibrary {

class FlyingEnemy : public Enemy {
 public:
  FlyingEnemy(b2World* world, b2Vec2 position, bool is_facing_right);
  void Draw();

  /**
   * If the robot is not already flying upward, there will be a random chance
   * that it will activate jetpack, which applies a random vertical linear impulse
   * */
  void Fly();

 private:
  ci::gl::Texture2dRef right_flying_image_;
  ci::gl::Texture2dRef left_flying_image_;
};

}

#endif  // FINALPROJECT_FLYING_ENEMY_H
