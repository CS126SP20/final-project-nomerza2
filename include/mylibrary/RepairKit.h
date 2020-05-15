//
// Created by natha on 5/15/2020.
//

#ifndef FINALPROJECT_REPAIRKIT_H
#define FINALPROJECT_REPAIRKIT_H

#include <mylibrary/entity.h>

const float kRepairDimension = 62/kPixelsPerMeter;

namespace mylibrary {

class RepairKit : public Entity{
 public:
  RepairKit(b2World* world, b2Vec2 location);
  void Draw();
};

}

#endif  // FINALPROJECT_REPAIRKIT_H
