#include "HealthBar.hpp"
#include <algorithm>

HealthBar::HealthBar() {
  barSprite_ = {
      {0, 0},
      {30, 10},
      SpriteTextureId::Checker,
      {1.0f, 1.0f, 1.0f, 1.0f},
      SpriteTimeSource::Real,
      {0.f, 0.f, .5f, .5f},
      SpriteShape::Rectangle,
      SpriteBlendMode::Alpha,
      0,
      SpriteSamplerMode::Point,
      SpriteAddressMode::Clamp,
  };
  barBackgroundSprite_ = {
      {0, 0},
      {30, 10},
      SpriteTextureId::White,
      {0.0f, 0.0f, 0.0f, 1.0f},
      SpriteTimeSource::Real,
      {0.f, 0.f, .1f, .1f},
      SpriteShape::Rectangle,
      SpriteBlendMode::Alpha,
      0,
      SpriteSamplerMode::Point,
      SpriteAddressMode::Clamp,
  };
  value_ = 1.f;
  visible_ = true;
}

void HealthBar::SetValue(float value) {
  value_ = std::clamp(value, 0.0f, 1.0f);
  UpdateSprites();
}
void HealthBar::SetPosition(std::array<float, 2> position) {
  barBackgroundSprite_.center = position;
  barSprite_.center = position;
  UpdateSprites();
}

void HealthBar::setVisible(bool visible) { visible_ = visible; }

void HealthBar::SetSize(std::array<float, 2> size) {
  const std::array<float, 2> boundedSize{
      std::max(0.0f, size[0]), std::max(0.0f, size[1])};
  barBackgroundSprite_.size = boundedSize;
  barSprite_.size = boundedSize;
  UpdateSprites();
}

void HealthBar::UpdateSprites() {
  barSprite_.size[0] = value_ * barBackgroundSprite_.size[0];
  barSprite_.center[0] =
      barBackgroundSprite_.center[0] -
      (barBackgroundSprite_.size[0] - barSprite_.size[0]) / 2.f;
}
