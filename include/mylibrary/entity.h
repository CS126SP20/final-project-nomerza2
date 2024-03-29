//
// Created by natha on 4/25/2020.
//

#ifndef FINALPROJECT_ENTITY_H
#define FINALPROJECT_ENTITY_H

#include <Box2D/Box2D.h>

const int kPixelsPerMeter = 90;

namespace mylibrary {

//Originally called Player, Bullet, Enemy, caused name conflicts with class names
enum EntityType{type_bullet, type_enemy, type_repair};

class Entity {
 public:
  virtual void Draw() = 0;

  // Called when game is being restarted, to reset the EntityID to 0.
  static void ResetID();

  static unsigned int GetEntityID();
  EntityType GetEntityType();
  b2Body* getBody() const;

 protected:

  // Maintains a count for entity IDs. Increments whenever a new one is made,
  // so it will always be unique.
  static unsigned int entity_ID_;
  EntityType entity_type_;
  b2Body* body_;
};

}
#endif  // FINALPROJECT_ENTITY_H
