#pragma once

#include "AudioSystem.h"
#include "BulletSystem.h"
#include "Enemy.h"
#include "GameTimer.h"
#include "Graphics.h"
#include "HealthBar.hpp"
#include "Input.h"
#include "Player.h"
#include "UniformGrid.h"
#include "Window.h"
#include <array>
#include <cstdint>

class Application {
public:
  Application(HINSTANCE instance, int showCommand);

  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;
  Application(Application &&) = delete;
  Application &operator=(Application &&) = delete;

  int Run();

private:
  enum class GameState {
    Title,
    Playing,
    Result,
  };

  enum class BattleOutcome {
    None,
    Clear,
    Failed,
  };

  void HandlePendingResize();
  void HandlePendingInput();
  void HandleGameStateControls();
  void HandleVisualControls();
  void ChangeGameState(GameState nextState);
  void EnterGameState(GameState state);
  void RunFixedUpdates(double deltaSeconds);
  void UpdateFps(double deltaSeconds);
  void Update(double fixedDeltaSeconds);
  void UpdateEnemyShooting(double fixedDeltaSeconds);
  void RebuildEnemyBulletGrid();
  void CheckPlayerEnemyBulletCollisions();
  void CheckEnemyPlayerBulletCollisions();
  void CheckBattleOutcome();
  void Render();
  void RenderCheckerBackground();
  void RenderTitle();
  void RenderPlaying();
  void RenderResult();
  void RenderDebugOverlay();
  void RenderHud();
  void RenderGameUI();

  static constexpr double kFixedDeltaSeconds = 1.0 / 60.0;
  static constexpr double kMaxGameDeltaSeconds = 0.25;
  static constexpr float kCollisionGridCellSize = 64.0f;
  static constexpr std::uint64_t kGrazeScore = 100;

  GameState gameState_ = GameState::Title;
  BattleOutcome battleOutcome_ = BattleOutcome::None;
  Window window_;
  Graphics graphics_;
  AudioSystem audio_;
  Input input_;
  Player player_;
  HealthBar healthBar_;
  HealthBar enemyHealthBar_;
  Enemy enemy_;
  BulletSystem bulletSystem_;
  UniformGrid enemyBulletGrid_{20, 12};
  GameTimer timer_;
  double accumulatorSeconds_ = 0.0;
  double gameTimeSeconds_ = 0.0;
  double enemyShotCooldownSeconds_ = 0.0;
  float enemyFanAngleRadians_ = 0.0f;
  bool paused_ = false;
  std::array<float, 4> playerUvRect_{0.0f, 0.0f, 1.0f, 1.0f};
  SpriteShape bulletShape_ = SpriteShape::SoftCircle;
  SpriteBlendMode bulletBlendMode_ = SpriteBlendMode::Alpha;
  float playerRotationRadians_ = 0.0f;
  SpriteSamplerMode playerSamplerMode_ = SpriteSamplerMode::Point;
  SpriteAddressMode playerAddressMode_ = SpriteAddressMode::Clamp;
  bool debugOverlayVisible_ = true;
  bool playerHit_ = false;
  bool playerGraze_ = false;
  std::uint64_t grazeCount_ = 0;
  std::uint64_t score_ = 0;
  std::size_t activeEnemyBulletCount_ = 0;
  std::size_t collisionCandidateCount_ = 0;
  double currentFps_ = 0.0;
  int frameCount = 0;
  double elapsedSeconds = 0;
  std::uint64_t shotRequestCount_ = 0;
};
