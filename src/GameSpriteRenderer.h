#pragma once

#include "PlayfieldLayout.h"

class Graphics;

// 한 프레임 동안 Graphics를 빌려 쓰며 전투 좌표와 애니메이션 UV를 변환한다.
class GameSpriteRenderer {
public:
  GameSpriteRenderer(Graphics &graphics, const PlayfieldLayout &layout,
                     double gameTime, double realTime)
      : graphics_(graphics), layout_(layout), gameTime_(gameTime),
        realTime_(realTime) {}
  void DrawGameSprite(const SpriteDrawData &sprite) const;
  void DrawGameSprite(const SpriteDrawData &sprite,
                      const SpriteAnimationData &animation) const;
  void DrawGameSprite(const SpriteDrawData &sprite,
                      const SpriteAnimationData &animation, double time) const;
  void DrawSurround(float width, float height) const;

private:
  Graphics &graphics_;
  PlayfieldLayout layout_;
  double gameTime_;
  double realTime_;
};
