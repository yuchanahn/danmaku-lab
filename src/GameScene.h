#pragma once

#include "BulletSystem.h"
#include "BulletSpriteInstanceData.h"
#include "Enemy.h"
#include "GameState.h"
#include "Input.h"
#include "Player.h"
#include "PlayfieldLayout.h"
#include "ScrollingBackground.h"
#include "StageDirector.h"
#include "UniformGrid.h"
#include <array>
#include <cstdint>
#include <vector>

class GameSpriteRenderer;
class Graphics;

enum class CollisionMode { LinearScan, UniformGrid };

struct SceneVisualSettings {
  std::array<float, 4> playerUvRect{0.0f, 0.0f, 1.0f, 1.0f};
  SpriteShape bulletShape = SpriteShape::SoftCircle;
  SpriteBlendMode bulletBlendMode = SpriteBlendMode::Additive;
  float playerRotationRadians = 0.0f;
  SpriteSamplerMode playerSamplerMode = SpriteSamplerMode::Linear;
  SpriteAddressMode playerAddressMode = SpriteAddressMode::Clamp;
  float dissolvePreviewProgress = 0.0f;
  bool enhancedBullets = true;
  bool instancedBullets = true;
};

struct SceneStatistics {
  std::uint64_t score;
  std::uint64_t grazeCount;
  std::uint64_t shotRequests;
  std::size_t activeEnemyBullets;
  std::size_t collisionCandidates;
  bool playerHit;
  bool playerGraze;
};

struct CollisionBenchmarkStatistics {
  std::uint64_t ticks = 0;
  double buildMilliseconds = 0.0;
  double queryMilliseconds = 0.0;
  std::uint64_t candidates = 0;
};

class GameScene {
public:
  void Reset();
  // 발사 여부만 반환한다. 소리 재생과 화면 전환은 Application이 담당한다.
  [[nodiscard]] bool Update(const Input &input, double fixedDeltaSeconds);
  void HandleVisualControls(const Input &input, bool allowPreview);
  void ApplyDamageCheat();
  void SetGodMode(bool enabled) noexcept { godMode_ = enabled; }
  [[nodiscard]] bool IsGodMode() const noexcept { return godMode_; }
  void PrepareStageBenchmark(int stageNumber);
  void SetBulletAllocationMode(BulletAllocationMode mode) {
    bulletSystem_.SetAllocationMode(mode);
  }
  void BeginPoolMeasurement() noexcept { bulletSystem_.BeginPoolMeasurement(); }
  void EndPoolMeasurement() noexcept { bulletSystem_.EndPoolMeasurement(); }
  void SetInstancedBullets(bool enabled) noexcept {
    visuals_.instancedBullets = enabled;
  }
  void SetCollisionMode(CollisionMode mode);
  void BeginCollisionMeasurement() noexcept {
    collisionBenchmark_ = {};
    measureCollision_ = true;
  }
  void EndCollisionMeasurement() noexcept { measureCollision_ = false; }
  [[nodiscard]] CollisionBenchmarkStatistics GetCollisionMeasurement() const noexcept {
    return collisionBenchmark_;
  }
  [[nodiscard]] CollisionMode GetCollisionMode() const noexcept {
    return collisionMode_;
  }
  void Render(const GameSpriteRenderer &renderer) const;
  [[nodiscard]] const Player &GetPlayer() const { return player_; }
  [[nodiscard]] const Enemy &GetEnemy() const { return enemy_; }
  [[nodiscard]] bool HasBoss() const noexcept { return bossPresent_; }
  [[nodiscard]] const StageDirector &GetStage() const noexcept {
    return stage_;
  }
  [[nodiscard]] const std::vector<Enemy> &GetMinions() const {
    return minions_;
  }
  [[nodiscard]] const BulletSystem &GetBulletSystem() const {
    return bulletSystem_;
  }
  [[nodiscard]] const SceneVisualSettings &GetVisualSettings() const {
    return visuals_;
  }
  [[nodiscard]] SceneStatistics GetStatistics() const;
  [[nodiscard]] BattleOutcome GetOutcome() const { return battleOutcome_; }
  [[nodiscard]] double GetGameTimeSeconds() const { return gameTimeSeconds_; }
  [[nodiscard]] float GetDeathDissolveProgress() const noexcept;
  [[nodiscard]] bool IsEnding() const;
  [[nodiscard]] bool IsFinished() const;
  void RunSmokeTest(Graphics &graphics, const PlayfieldLayout &layout);

private:
  enum class PlayerMotion { Idle, Left, Right };
  bool UpdateCombat(const Input &input, double fixedDeltaSeconds);
  void UpdateEnemyShooting(double fixedDeltaSeconds);
  void StartStage();
  void UpdateStageProgress();
  void SpawnScheduledEnemies();
  void RunStageSmokeTest(Graphics &graphics, const PlayfieldLayout &layout);
  void UpdateMinions(double fixedDeltaSeconds);
  void RebuildEnemyBulletGrid();
  void CheckPlayerEnemyBulletCollisions();
  void CheckEnemyPlayerBulletCollisions();
  void DamageEnemy(Enemy &enemy, int damage);
  void CheckBattleOutcome();
  void UpdateBattleEnding(double fixedDeltaSeconds);
  void RenderPlayer(const GameSpriteRenderer &renderer) const;
  void RenderEnemy(const GameSpriteRenderer &renderer) const;
  void RenderBullets(const GameSpriteRenderer &renderer) const;

  static constexpr float kPlayfieldWidth = PlayfieldLayout::kWidth;
  static constexpr float kPlayfieldHeight = PlayfieldLayout::kHeight;
  static constexpr float kCollisionGridCellSize = 64.0f;
  static constexpr double kDeathDissolveDurationSeconds = 0.8;
  static constexpr std::uint64_t kGrazeScore = 100;

  Player player_;
  ScrollingBackground background_;
  Enemy enemy_;
  bool bossPresent_ = false;
  StageDirector stage_;
  std::vector<Enemy> minions_;
  BulletSystem bulletSystem_;
  // const 렌더에서 표시용 저장 공간만 갱신하며 게임 상태는 바꾸지 않는다.
  mutable std::vector<BulletSpriteInstanceData> bulletGlowInstances_;
  mutable std::vector<BulletSpriteInstanceData> bulletBodyInstances_;
  CollisionMode collisionMode_ = CollisionMode::UniformGrid;
  UniformGrid enemyBulletGrid_{
      static_cast<std::size_t>(
          (kPlayfieldWidth + kCollisionGridCellSize - 1.0f) /
          kCollisionGridCellSize),
      static_cast<std::size_t>(
          (kPlayfieldHeight + kCollisionGridCellSize - 1.0f) /
          kCollisionGridCellSize)};
  BattleOutcome battleOutcome_ = BattleOutcome::None;
  SceneVisualSettings visuals_;
  PlayerMotion playerMotion_ = PlayerMotion::Idle;
  double gameTimeSeconds_ = 0.0;
  double battleEndingSeconds_ = 0.0;
  double summonCooldownSeconds_ = 3.0;
  std::size_t bossVolleyIndex_ = 0;
  bool playerHit_ = false;
  bool godMode_ = false;
  bool playerGraze_ = false;
  std::uint64_t score_ = 0;
  std::uint64_t grazeCount_ = 0;
  std::uint64_t shotRequestCount_ = 0;
  std::size_t activeEnemyBulletCount_ = 0;
  std::size_t collisionCandidateCount_ = 0;
  bool measureCollision_ = false;
  CollisionBenchmarkStatistics collisionBenchmark_;
};
