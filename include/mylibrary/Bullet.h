//
// Created by natha on 4/23/2020.
//

#ifndef FINALPROJECT_BULLET_H
#define FINALPROJECT_BULLET_H

#include <Box2D/Box2D.h>
#include <mylibrary/entity.h>

namespace mylibrary {

class Bullet : public Entity {
 public:
  Bullet(b2World* world, b2Vec2 position, bool player_made, float bullet_radius = 0.05f);
  void Draw();
  bool isPlayerMade() const;
  bool isSuper() const;

 protected:
  //True if is a player's bullet, false if an enemy's
  bool player_made_;
  bool super_;
  float bullet_radius_;
};
}
#endif  // FINALPROJECT_BULLET_H
