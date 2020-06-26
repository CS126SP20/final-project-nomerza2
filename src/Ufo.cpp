//
// Created by natha on 6/26/2020.
//

#include "mylibrary/Ufo.h"
#include <mylibrary/EffectiveDimensions.h>
#include <cinder/app/App.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>

//TODO redefinition from my_app.h
const int kStandardWidth = 1920;
const int kStandardHeight = 1080;

using mylibrary::EffectiveDimensions;

Ufo::Ufo(b2World* world) {
  world_ = world;

  b2BodyDef bodyDef;
  bodyDef.type = b2_kinematicBody;
  bodyDef.fixedRotation = true;
  bodyDef.position.Set(10, 10);
  body_ = world->CreateBody(&bodyDef);

  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(kUfoWidth/2, kUfoHeight/2);
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.density = 2.0f;
  fixtureDef.friction = 0.0f;
  b2Fixture* fixture = body_->CreateFixture(&fixtureDef);

  image_ = ci::gl::Texture2d::create(ci::loadImage(cinder::app::loadAsset("UFO.png")));
  body_->SetLinearVelocity(b2Vec2(3, 0));

  lives_ = 5;
}

void Ufo::VelocityUpdate() {
  //Acts like a moving wall, with lower limit being always 0 and upper limit being kStandardWidth/kPPM
  if ((body_->GetPosition().x + (kUfoWidth / 2) >= kStandardWidth / kPixelsPerMeter)
    || (body_->GetPosition().x - (kUfoWidth / 2) <= 0)) {

      body_->SetLinearVelocity(-1 * body_->GetLinearVelocity());
    }
}

void Ufo::Draw() {
  b2Vec2 position = body_->GetPosition();

  int pixel_x = position.x*kPixelsPerMeter - kUfoWidth*kPixelsPerMeter/2;
  int pixel_y = EffectiveDimensions::GetEffectiveHeight() - (position.y*kPixelsPerMeter + kUfoHeight*kPixelsPerMeter/2);

  // Necessary or the image will be tinted the color of the last drawn object.
  ci::Color reset(1,1,1);
  ci::gl::color(reset);
  ci::gl::draw(image_, ci::vec2(pixel_x, pixel_y));


  //TODO this was copy/pasted from the PrintText function in main because I didn't feel like setting up a helper class that would let it be used here
  //Enemy Life counter
  std::string text = std::to_string(lives_);
  const cinder::ivec2 size = {100, 50};
  const cinder::vec2 loc(EffectiveDimensions::GetEffectiveWidth() - 50.0f,
      EffectiveDimensions::GetEffectiveHeight() - (kStandardHeight - 50.0f)); //Top-right Corner
  int font_size = 50;

  ci::Color color = ci::Color(1,0,0);
  cinder::gl::color(color);
  auto box = cinder::TextBox()
      .alignment(cinder::TextBox::CENTER)
      .font(cinder::Font("Arial", font_size))
      .size(size)
      .color(color)
      .backgroundColor(cinder::ColorA(0, 0, 0, 0))
      .text(text);

  const auto box_size = box.getSize();
  const cinder::vec2 locp = {loc.x - box_size.x / 2, loc.y - box_size.y / 2};
  const auto surface = box.render();
  const auto texture = cinder::gl::Texture::create(surface);
  cinder::gl::draw(texture, locp);
}