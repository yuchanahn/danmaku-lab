#pragma once
#include "GameState.h"
#include "HealthBar.hpp"

class GameScene;
class GameSpriteRenderer;
class Graphics;
struct PlayfieldLayout;

struct HudFrame {
  GameState state;
  float width;
  float height;
  bool paused;
  bool debugVisible;
  double fps;
  double realTime;
};

// 표시용 데이터를 구성한다. HP/점수/전투 결과는 변경하지 않는다.
class GameHud {
public:
  GameHud();
  void RenderBackground(Graphics &graphics, float width, float height,
                        bool bright);
  void RenderHealthBars(const GameScene &scene,
                        const GameSpriteRenderer &renderer);
  void RenderScreen(Graphics &graphics, const GameScene &scene,
                    const PlayfieldLayout &layout, const HudFrame &frame);
  void RenderDebug(Graphics &graphics, const GameScene &scene,
                   const HudFrame &frame);

private:
  HealthBar healthBar_;
  HealthBar enemyHealthBar_;
  HealthBar minionHealthBar_;
};
