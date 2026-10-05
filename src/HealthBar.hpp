#pragma once

#include "SpriteDrawData.h"
#include <vector>

class HealthBar {
public:
  HealthBar();

  void SetValue(float value);
  void SetSize(std::array<float, 2> size);
  void SetPosition(std::array<float, 2> position);
  void setVisible(bool visible);
  [[nodiscard]] float GetHeight() const noexcept {
    return barBackgroundSprite_.size[1];
  }

  std::vector<SpriteDrawData> GetSprites() const {
    if (!visible_)
      return {};
    return {barBackgroundSprite_, barSprite_};
  };

private:
  SpriteDrawData barSprite_;
  SpriteDrawData barBackgroundSprite_;
  bool visible_;
  float value_;

  void UpdateSprites();
};
