//
// Created by natha on 5/9/2020.
//

#include "mylibrary/Hunter.h"
#include <mylibrary/SuperBullet.h>
#include <cinder/app/App.h>

namespace mylibrary {

Hunter::Hunter(b2World* world, b2Vec2 position) : Enemy(world, position, false) {
  enemyType = hunter;

  right_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("HunterRight.png")));
  left_image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("HunterLeft.png")));
}

Bullet* Hunter::Shoot(b2World* world, b2Vec2 player_pos) {
  //Face Right if player is to the right of the hunter
  facing_right_ = body_->GetPosition().x < player_pos.x;

  Bullet* bullet = new SuperBullet(world, Calculate_Bullet_Spawn(), player_pos);
  return bullet;
}

}
