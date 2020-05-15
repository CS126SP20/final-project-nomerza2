//
// Created by natha on 5/15/2020.
//

#include "mylibrary/RepairKit.h"

#include <cinder/app/App.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>

namespace mylibrary {

RepairKit::RepairKit(b2World* world, b2Vec2 location) {
  entity_type_ = type_repair;

  b2BodyDef bodyDef;
  bodyDef.type = b2_staticBody;
  bodyDef.position.Set(location.x, location.y);
  body_ = world->CreateBody(&bodyDef);

  b2PolygonShape polygonShape;
  polygonShape.SetAsBox(0.5f, 0.5f);
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &polygonShape;

  b2Fixture* fixture = body_->CreateFixture(&fixtureDef);
  fixture->SetUserData((void*)++Entity::entity_ID_); //Used for collision callback
  //Increments first, so then entity_ID_ = the ID of the new entity afterwards
}

void RepairKit::Draw() {
  b2Vec2 position = body_->GetPosition();

  int pixel_x = position.x*kPixelsPerMeter - kRepairDimension*kPixelsPerMeter/2;
  int pixel_y = ci::app::getWindowHeight() - (position.y*kPixelsPerMeter + kRepairDimension*kPixelsPerMeter/2);

  ci::gl::Texture2dRef image_ref = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("RepairKit.png")));

  // Necessary or the image will be tinted the color of the last drawn object.
  ci::Color reset(1,1,1);
  ci::gl::color(reset);
  ci::gl::draw(image_ref, ci::vec2(pixel_x, pixel_y));
}

}
