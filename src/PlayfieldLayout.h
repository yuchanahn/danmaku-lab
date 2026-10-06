#pragma once

#include "SpriteDrawData.h"
#include <algorithm>
#include <array>

// 전투 영역의 논리 크기와 화면 배치. GPU 리소스나 게임 상태는 소유하지 않는다.
struct PlayfieldLayout {
  static constexpr float kWidth = 720.0f;
  static constexpr float kHeight = 960.0f;
  float left = 0.0f;
  float top = 0.0f;
  float scale = 1.0f;

  [[nodiscard]] static PlayfieldLayout FromClientSize(float width,
                                                      float height) {
    const float scale = std::min(width / 1920.0f, height / 1080.0f);
    return {(width - kWidth * scale) * 0.5f, (height - kHeight * scale) * 0.5f,
            scale};
  }
  [[nodiscard]] float Right() const { return left + kWidth * scale; }
  [[nodiscard]] float Bottom() const { return top + kHeight * scale; }
  [[nodiscard]] SpriteDrawData ToScreen(const SpriteDrawData &sprite) const {
    auto result = sprite;
    result.center = {left + sprite.center[0] * scale,
                     top + sprite.center[1] * scale};
    result.size = {sprite.size[0] * scale, sprite.size[1] * scale};
    return result;
  }

};
