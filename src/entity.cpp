//
// Created by natha on 4/25/2020.
//

#include "mylibrary/entity.h"

namespace mylibrary {
unsigned int Entity::entity_ID_ = 0;

unsigned int Entity::GetEntityID() { return entity_ID_;}
EntityType Entity::GetEntityType() { return entity_type_;}
}