//
// Created by natha on 4/30/2020.
//

#include "mylibrary/Wall.h"
#include <cinder/app/App.h>

const int kPixelsPerMeter = 90; //TODO Create unified declaration

namespace mylibrary {

Wall::Wall(b2World* world, float x_loc, float y_loc, float half_width, float half_height, ci::Color color) {
  b2BodyDef bodyDef;
  bodyDef.type = b2_staticBody;
  bodyDef.position.Set(x_loc, y_loc);
  body_ = world->CreateBody(&bodyDef);
  b2PolygonShape box;
  box.SetAsBox(half_width, half_height);
  body_->CreateFixture(&box, 0);

  half_width_ = half_width;
  half_height_ = half_height;
  color_ = color;
}

void Wall::Draw() {
  b2Vec2 position = body_->GetPosition();
  ci::Rectf rect(
      (position.x - half_width_)*kPixelsPerMeter,
      ci::app::getWindowHeight() - ((position.y - half_height_) * kPixelsPerMeter),
      (position.x + half_width_) * kPixelsPerMeter,
      ci::app::getWindowHeight() - ((position.y + half_height_) * kPixelsPerMeter));

  ci::gl::color(color_);
  ci::gl::drawSolidRect(rect);
}

}