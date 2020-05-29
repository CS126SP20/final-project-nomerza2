//
// Created by natha on 5/25/2020.
//

#ifndef FINALPROJECT_EFFECTIVEDIMENSIONS_H
#define FINALPROJECT_EFFECTIVEDIMENSIONS_H

namespace mylibrary {

class EffectiveDimensions {
 private:
  static int effective_width_;
  static int effective_height_;

 public:
  static int GetEffectiveWidth();
  static void SetEffectiveWidth(int w);
  static int GetEffectiveHeight();
  static void SetEffectiveHeight(int h);
};

}

#endif  // FINALPROJECT_EFFECTIVEDIMENSIONS_H
