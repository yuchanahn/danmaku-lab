#include "ScrollingBackground.h"
#include "GameSpriteRenderer.h"
#include "PlayfieldLayout.h"
#include <algorithm>
#include <cmath>

float CalculateBackgroundScrollOffset(double elapsedSeconds, float speed,
                                      float tileHeight) {
  if (speed <= 0.0f || tileHeight <= 0.0f)
    return 0.0f;
  return static_cast<float>(std::fmod(std::max(0.0, elapsedSeconds) * speed,
                                      static_cast<double>(tileHeight)));
}

void ScrollingBackground::Render(const GameSpriteRenderer &renderer,
                                 double gameTimeSeconds) const {
  // 현재 bg0/bg1 원본의 실제 크기. 가로폭을 맞추고 원래 비율을 보존한다.
  constexpr float kSourceWidth = 941.0f;
  constexpr float kSourceHeight = 1672.0f;
  constexpr float kTileWidth = PlayfieldLayout::kWidth;
  constexpr float kTileHeight = kTileWidth * kSourceHeight / kSourceWidth;
  static_assert(kTileHeight >= PlayfieldLayout::kHeight);
  const auto drawLayer = [&](SpriteTextureId texture, float speed,
                             std::array<float, 4> tint) {
    const float offset =
        CalculateBackgroundScrollOffset(gameTimeSeconds, speed, kTileHeight);
    SpriteDrawData sprite{{kTileWidth * 0.5f, offset + kTileHeight * 0.5f},
                          {kTileWidth, kTileHeight},
                          texture,
                          tint};
    sprite.samplerMode = SpriteSamplerMode::Linear;
    renderer.DrawGameSprite(sprite);
    sprite.center[1] -= kTileHeight;
    renderer.DrawGameSprite(sprite);
  };
  drawLayer(SpriteTextureId::ForestBackground, 40.0f, {1.0f, 1.0f, 1.0f, 1.0f});
  drawLayer(SpriteTextureId::FogBackground, 15.0f, {1.0f, 1.0f, 1.0f, 0.16f});
}
