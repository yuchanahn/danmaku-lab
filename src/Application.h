#pragma once

#include "AudioSystem.h"
#include "GameHud.h"
#include "GameScene.h"
#include "GameTimer.h"
#include "Graphics.h"
#include "Input.h"
#include "Window.h"

class Application {
public:
  Application(HINSTANCE instance, int showCommand);
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;
  Application(Application &&) = delete;
  Application &operator=(Application &&) = delete;
  int Run();
  int RunSmokeTest();

private:
  void HandlePendingResize();
  void HandlePendingInput();
  void HandleGameStateControls();
  void HandleVisualControls();
  void ChangeGameState(GameState nextState);
  void EnterGameState(GameState state);
  void RunFixedUpdates(double deltaSeconds);
  void UpdateFps(double deltaSeconds);
  void Render();

  static constexpr double kFixedDeltaSeconds = 1.0 / 60.0;
  static constexpr double kMaxGameDeltaSeconds = 0.25;
  GameState gameState_ = GameState::Title;
  Window window_;
  Graphics graphics_;
  AudioSystem audio_;
  Input input_;
  GameScene gameScene_;
  GameHud gameHud_;
  GameTimer timer_;
  double accumulatorSeconds_ = 0.0;
  bool paused_ = false;
  bool debugOverlayVisible_ = false;
  bool brightBackground_ = false;
  double currentFps_ = 0.0;
  int frameCount = 0;
  double elapsedSeconds = 0.0;
};
