//
// Created by natha on 6/5/2020.
//

#include "mylibrary/MovingWall.h"
#include <mylibrary/entity.h>
#include <mylibrary/EffectiveDimensions.h>
#include <cmath>

using ci::vec2;

const int kStandardWidth = 1920; //TODO, redefinition from my_app.h, fix this maybe?
const int kStandardHeight = 1080;
const int kHitboxRatio = 8; // Ratio of (half-height of) hitbox around spikes to kSpikeSize

namespace mylibrary {

MovingWall::MovingWall(b2World* world, float x_loc, float y_loc, float half_width,
                       float half_height, float lower_limit, float upper_limit,
                       float velocity, bool moves_vertically, ci::Color color) {
  b2BodyDef bodyDef;
  bodyDef.type = b2_kinematicBody;
  bodyDef.position.Set(x_loc, y_loc);
  body_ = world->CreateBody(&bodyDef);
  b2PolygonShape box;
  box.SetAsBox(half_width, half_height);
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &box;
  fixtureDef.friction = 0.5f;
  b2Fixture* main_body = body_->CreateFixture(&fixtureDef);
  main_body->SetUserData((void*)kMovingWallID);

  b2PolygonShape left_spikebox_shape;
  if (moves_vertically) {
    left_spikebox_shape.SetAsBox(half_width * 0.9f, (kSpikeSize / kHitboxRatio), b2Vec2(0, -1 * half_height - (kSpikeSize / 2)), 0);
  } else {
    left_spikebox_shape.SetAsBox((kSpikeSize / kHitboxRatio), half_height * 0.9f, b2Vec2(-1 * half_width - (kSpikeSize / 2), 0), 0);
  }
  b2FixtureDef left_spikebox_def;
  left_spikebox_def.shape = &left_spikebox_shape;
  b2Fixture* left_spikebox = body_->CreateFixture(&left_spikebox_def);
  left_spikebox->SetUserData((void*)kSpikeID);

  b2PolygonShape right_spikebox_shape;
  if (moves_vertically) {
    right_spikebox_shape.SetAsBox(half_width * 0.9f, (kSpikeSize / kHitboxRatio), b2Vec2(0, half_height + (kSpikeSize / 2)), 0);
  } else {
    right_spikebox_shape.SetAsBox((kSpikeSize / kHitboxRatio), half_height * 0.9f, b2Vec2(half_width + (kSpikeSize / 2), 0), 0);
  }
  b2FixtureDef right_spikebox_def;
  right_spikebox_def.shape = &right_spikebox_shape;
  b2Fixture* right_spikebox = body_->CreateFixture(&right_spikebox_def);
  right_spikebox->SetUserData((void*)kSpikeID);

  half_width_ = half_width;
  half_height_ = half_height;
  lower_limit_ = lower_limit;
  upper_limit_ = upper_limit;
  moves_vertically_ = moves_vertically;
  color_ = color;
  activator_ = nullptr;
  initial_velocity_ = velocity;
}

MovingWall::MovingWall(b2World* world, float x_loc, float y_loc, float half_width,
float half_height, float lower_limit, float upper_limit, float velocity,
bool moves_vertically, ci::Color color, Enemy* activator)
: MovingWall(world, x_loc, y_loc, half_width, half_height, lower_limit, upper_limit, velocity, moves_vertically, color) {

  activator_ = activator;
};

void MovingWall::VelocityUpdate() {
  if (activator_ != nullptr && !(activator_->isActive())) {
    body_->SetLinearVelocity(b2Vec2_zero);
    return;
  }

  b2Vec2 velocity = body_->GetLinearVelocity();

  //Initialize velocity
  if (velocity == b2Vec2_zero && (activator_ == nullptr || activator_->isActive())) {
    if (moves_vertically_) {
      body_->SetLinearVelocity(b2Vec2(0, initial_velocity_));
    } else {
      body_->SetLinearVelocity(b2Vec2(initial_velocity_, 0));
    }

    return;
  }


  if (moves_vertically_) {
    if (velocity.y > 0) {
      if (body_->GetPosition().y + half_height_ + kSpikeSize >= upper_limit_) {
        body_->SetLinearVelocity(-1 * velocity);
      }

    } else { // velocity < 0, moves_vertically = true;
      if (body_->GetPosition().y - half_height_ - kSpikeSize <= lower_limit_) {
        body_->SetLinearVelocity(-1 * velocity);
      }
    }

  } else { //moves horizontally
    if (velocity.x > 0) {
      if (body_->GetPosition().x + half_width_ + kSpikeSize >= upper_limit_) {
        body_->SetLinearVelocity(-1 * velocity);
      }

    } else { // velocity < 0, moves horizontally
      if (body_->GetPosition().x - half_width_ - kSpikeSize <= lower_limit_) {
        body_->SetLinearVelocity(-1 * velocity);
      }
    }
  }
}

void MovingWall::Draw() {
  b2Vec2 position = body_->GetPosition();
  int w_height = EffectiveDimensions::GetEffectiveHeight();

  ci::Rectf rect(
      (position.x - half_width_)*kPixelsPerMeter,
      w_height - ((position.y - half_height_) * kPixelsPerMeter),
      (position.x + half_width_) * kPixelsPerMeter,
      w_height - ((position.y + half_height_) * kPixelsPerMeter));

  ci::gl::color(color_);
  ci::gl::drawSolidRect(rect);

  ci::gl::color(kSpikeColor);

  //Spikes now
  if (moves_vertically_) {
    int num_spikes = floor(half_width_ * 2 / kSpikeSize) - 1;
    float spike_shift = fmod((half_width_ * 2), kSpikeSize) / 2;

    for (size_t i = 0; i < num_spikes; i++) {
      //top side
      ci::gl::drawSolidTriangle(
          vec2((position.x - half_width_ + spike_shift + i*kSpikeSize) * kPixelsPerMeter, //Bottom left point
               w_height - ((position.y + half_height_) * kPixelsPerMeter)),
          vec2((position.x - half_width_ + spike_shift + (i+1)*kSpikeSize) * kPixelsPerMeter, //Bottom right point
               w_height - ((position.y + half_height_) * kPixelsPerMeter)),
          vec2((position.x - half_width_ + spike_shift + ((float) i+0.5f)*kSpikeSize) * kPixelsPerMeter, //Top Middle point
               w_height - ((position.y + half_height_ + kSpikeSize) * kPixelsPerMeter)));

      //bottom side
      ci::gl::drawSolidTriangle(
          vec2((position.x - half_width_ + spike_shift + i*kSpikeSize) * kPixelsPerMeter, //Bottom left point
               w_height - ((position.y - half_height_) * kPixelsPerMeter)),
          vec2((position.x - half_width_ + spike_shift + (i+1)*kSpikeSize) * kPixelsPerMeter, //Bottom right point
               w_height - ((position.y - half_height_) * kPixelsPerMeter)),
      vec2((position.x - half_width_ + spike_shift + ((float) i+0.5f)*kSpikeSize) * kPixelsPerMeter, //Top Middle point
           w_height - ((position.y - half_height_ - kSpikeSize) * kPixelsPerMeter)));
    }
  } else { // moves horizontally
    int num_spikes = floor(half_height_ * 2 / kSpikeSize) - 1;
    float spike_shift = fmod((half_height_ * 2), kSpikeSize) / 2;

    for (size_t i = 0; i < num_spikes; i++) {
      //right side
      ci::gl::drawSolidTriangle(
          vec2((position.x + half_width_) * kPixelsPerMeter, //Bottom left point
               w_height - ((position.y - half_height_ + i*kSpikeSize + spike_shift) * kPixelsPerMeter)),
          vec2((position.x + half_width_) * kPixelsPerMeter, //Top Left point
               w_height - ((position.y - half_height_ + (i+1)*kSpikeSize + spike_shift) * kPixelsPerMeter)),
      vec2((position.x + half_width_ + kSpikeSize) * kPixelsPerMeter, //Middle Right point
           w_height - ((position.y - half_height_ + ((float) i+0.5f)*kSpikeSize + spike_shift) * kPixelsPerMeter)));

      //left side
      ci::gl::drawSolidTriangle(
          vec2((position.x - half_width_) * kPixelsPerMeter, //Bottom left point
               w_height - ((position.y - half_height_ + i*kSpikeSize + spike_shift) * kPixelsPerMeter)),
          vec2((position.x - half_width_) * kPixelsPerMeter, //Top Left point
               w_height - ((position.y - half_height_ + (i+1)*kSpikeSize + spike_shift) * kPixelsPerMeter)),
      vec2((position.x - half_width_ - kSpikeSize) * kPixelsPerMeter, //Middle Right point
           w_height - ((position.y - half_height_ + ((float) i+0.5f)*kSpikeSize + spike_shift) * kPixelsPerMeter)));
    }
  }

}

}
