//
// Created by natha on 5/25/2020.
//

#include "mylibrary/EffectiveDimensions.h"
#include <cinder/app/App.h>

namespace mylibrary {

int EffectiveDimensions::effective_width_ = 1920;
int EffectiveDimensions::effective_height_ = 1080;

int EffectiveDimensions::GetEffectiveWidth() { return effective_width_; }
void EffectiveDimensions::SetEffectiveWidth(int w) { effective_width_ = w; }
int EffectiveDimensions::GetEffectiveHeight(){ return effective_height_; }
//int EffectiveDimensions::GetEffectiveHeight(){ return effective_height_; }
void EffectiveDimensions::SetEffectiveHeight(int h){ effective_height_ = h; }
}