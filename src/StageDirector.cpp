#include "StageDirector.h"
#include <array>

namespace {
constexpr std::array<EnemySpawnEvent, 10> kStageOneSpawns{{
    {0.0, EnemyKind::Enemy1, 120.0f, 90.0f},
    {1.5, EnemyKind::Enemy2, 600.0f, 90.0f},
    {3.0, EnemyKind::Enemy1, 240.0f, 110.0f},
    {4.5, EnemyKind::Enemy2, 480.0f, 110.0f},
    {6.0, EnemyKind::Enemy1, 360.0f, 90.0f},
    {8.0, EnemyKind::Enemy2, 120.0f, 160.0f},
    {9.5, EnemyKind::Enemy1, 600.0f, 160.0f},
    {11.0, EnemyKind::Enemy2, 240.0f, 170.0f},
    {12.5, EnemyKind::Enemy1, 480.0f, 170.0f},
    {14.0, EnemyKind::Enemy2, 360.0f, 170.0f},
}};
}
void StageDirector::Reset() noexcept {
  stage_ = 1;
  phase_ = StagePhase::Running;
  elapsed_ = transitionSeconds_ = 0.0;
  nextSpawn_ = 0;
}
void StageDirector::Update(double dt) noexcept {
  if (phase_ == StagePhase::Running)
    elapsed_ += dt;
  else if (phase_ == StagePhase::Transitioning)
    transitionSeconds_ += dt;
}
std::optional<EnemySpawnEvent> StageDirector::ConsumeSpawn() {
  if (stage_ != 1 || phase_ != StagePhase::Running || AllSpawnsIssued() ||
      kStageOneSpawns[nextSpawn_].time > elapsed_ + 1e-9)
    return std::nullopt;
  return kStageOneSpawns[nextSpawn_++];
}
bool StageDirector::AllSpawnsIssued() const noexcept {
  return stage_ == 1 && nextSpawn_ == kStageOneSpawns.size();
}
bool StageDirector::CanAdvance(bool enemiesRemoved,
                               bool bossRemoved) const noexcept {
  if (stage_ == 1)
    return AllSpawnsIssued() && enemiesRemoved;
  return stage_ == 2 && phase_ == StagePhase::Transitioning && bossRemoved &&
         transitionSeconds_ + 1e-9 >= 0.8;
}
void StageDirector::BeginBossTransition() noexcept {
  if (stage_ == 2 && phase_ == StagePhase::Running) {
    phase_ = StagePhase::Transitioning;
    transitionSeconds_ = 0.0;
  }
}
void StageDirector::Advance() noexcept {
  if (stage_ >= 3)
    return;
  ++stage_;
  phase_ = StagePhase::Running;
  elapsed_ = transitionSeconds_ = 0.0;
  nextSpawn_ = 0;
}
const wchar_t *StageDirector::GetName() const noexcept {
  return stage_ == 1   ? L"FOREST ASSAULT"
         : stage_ == 2 ? L"MID BOSS"
                       : L"FINAL BOSS";
}
