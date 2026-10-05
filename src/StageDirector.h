#pragma once
#include "Enemy.h"
#include <cstddef>
#include <optional>

enum class StagePhase { Running, Transitioning, Complete };
struct EnemySpawnEvent {
  double time;
  EnemyKind kind;
  float x;
  float y;
};

class StageDirector {
public:
  void Reset() noexcept;
  void Update(double deltaSeconds) noexcept;
  [[nodiscard]] std::optional<EnemySpawnEvent> ConsumeSpawn();
  [[nodiscard]] bool CanAdvance(bool enemiesRemoved,
                                bool bossRemoved) const noexcept;
  void BeginBossTransition() noexcept;
  void Advance() noexcept;
  void Finish() noexcept { phase_ = StagePhase::Complete; }
  [[nodiscard]] int GetStageNumber() const noexcept { return stage_; }
  [[nodiscard]] StagePhase GetPhase() const noexcept { return phase_; }
  [[nodiscard]] double GetElapsedSeconds() const noexcept { return elapsed_; }
  [[nodiscard]] std::size_t GetSpawnedCount() const noexcept {
    return nextSpawn_;
  }
  [[nodiscard]] bool AllSpawnsIssued() const noexcept;
  [[nodiscard]] const wchar_t *GetName() const noexcept;

private:
  int stage_ = 1;
  StagePhase phase_ = StagePhase::Running;
  double elapsed_ = 0.0;
  double transitionSeconds_ = 0.0;
  std::size_t nextSpawn_ = 0;
};
