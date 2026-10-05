#pragma once

class GameSpriteRenderer;

[[nodiscard]] float CalculateBackgroundScrollOffset(double elapsedSeconds,
                                                    float speed,
                                                    float tileHeight);

// 표시용 배경. 시간은 Scene에서 전달받고 GPU 리소스는 캐시가 소유한다.
class ScrollingBackground {
public:
  void Render(const GameSpriteRenderer &renderer, double gameTimeSeconds) const;
};
