//
// Created by natha on 4/25/2020.
//

#ifndef FINALPROJECT_ENTITY_H
#define FINALPROJECT_ENTITY_H

namespace mylibrary {

//Originally called Player, Bullet, Enemy, caused name conflicts with class names
enum EntityType{type_player, type_bullet, type_enemy};

class Entity {
 public:
  static unsigned int GetEntityID();
  EntityType GetEntityType();

 protected:
  // Maintains a count for entity IDs. Increments whenever a new one is made,
  // so it will always be unique.
  static unsigned int entity_ID_;
  EntityType entity_type_;
};

}
#endif  // FINALPROJECT_ENTITY_H