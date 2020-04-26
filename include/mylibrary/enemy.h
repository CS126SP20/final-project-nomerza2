//
// Created by natha on 4/25/2020.
//

#ifndef FINALPROJECT_ENEMY_H
#define FINALPROJECT_ENEMY_H

#include <mylibrary/entity.h>
#include <Box2D/Box2D.h>
#include <cinder/gl/Texture.h>

//TODO make different from player sizes?
const float kEnemyWidth = 1.0f;
const float kEnemyHeight = (105.0f/90.0f);

namespace mylibrary {

class Enemy : public Entity {
 public:
  Enemy(b2World* world, b2Vec2 position, bool is_facing_right);
  void Draw();
  b2Vec2 Calculate_Bullet_Spawn();

 private:
  bool facing_right_;
  ci::gl::Texture2dRef right_image_;
  ci::gl::Texture2dRef left_image_;
};

}

#endif  // FINALPROJECT_ENEMY_H
