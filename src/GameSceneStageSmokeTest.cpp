#include "GameScene.h"
#include "GameSpriteRenderer.h"
#include "Graphics.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

void GameScene::RunStageSmokeTest(Graphics &graphics,
                                  const PlayfieldLayout &layout) {
  const auto render = [&] {
    graphics.BeginFrame(gameTimeSeconds_, 0.0);
    const GameSpriteRenderer renderer(graphics, layout, gameTimeSeconds_, 0.0);
    Render(renderer);
    graphics.EndFrame();
  };
  const auto require = [](bool ok, const char *message) {
    if (!ok)
      throw std::runtime_error(message);
  };
  Input noInput;
  Reset();
  minions_.emplace_back(EnemyKind::Enemy2, 600.0f, 90.0f);
  const int cheatPlayerHp = player_.GetHp();
  ApplyDamageCheat();
  ApplyDamageCheat();
  require(minions_[0].GetHp() == 0 && minions_[1].GetHp() == 0 &&
              minions_[0].GetLifeState() == EnemyLifeState::Dying &&
              score_ == 200 && player_.GetHp() == cheatPlayerHp &&
              stage_.GetStageNumber() == 1,
          "Damage cheat missed enemies, duplicated score or skipped spawns.");
  Reset();
  stage_.Advance();
  StartStage();
  ApplyDamageCheat();
  require(enemy_.GetHp() == 70 && score_ == 0,
          "Damage cheat did not apply exactly 50 boss damage.");
  ApplyDamageCheat();
  ApplyDamageCheat();
  require(enemy_.GetHp() == 0 && score_ == 1000 &&
              stage_.GetPhase() == StagePhase::Transitioning &&
              battleOutcome_ == BattleOutcome::None,
          "Damage cheat bypassed midboss death/transition.");
  ApplyDamageCheat();
  require(score_ == 1000, "Transition cheat duplicated a boss reward.");
  Reset();
  stage_.Advance();
  stage_.Advance();
  StartStage();
  for (int i = 0; i < 5; ++i)
    ApplyDamageCheat();
  require(enemy_.GetHp() == 0 && battleOutcome_ == BattleOutcome::Clear &&
              IsEnding() && score_ == 5000,
          "Damage cheat did not produce final victory through normal ending.");
  Reset();
  require(stage_.GetStageNumber() == 1 && !HasBoss() && minions_.size() == 1,
          "New run did not start with stage one enemies.");
  render();
  while (minions_.front().GetHp() > 0)
    (void)minions_.front().TryTakeDamage();
  UpdateMinions(0.8);
  UpdateStageProgress();
  require(stage_.GetStageNumber() == 1 && minions_.empty(),
          "An empty gap advanced before the scheduled spawns finished.");
  stage_.Update(14.0);
  SpawnScheduledEnemies();
  SpawnScheduledEnemies();
  require(stage_.GetSpawnedCount() == 10 && minions_.size() == 9,
          "Stage one missed or duplicated a scheduled enemy.");
  render();
  for (auto &minion : minions_) {
    while (minion.GetHp() > 0)
      (void)minion.TryTakeDamage();
  }
  UpdateStageProgress();
  require(stage_.GetStageNumber() == 1,
          "Stage transition discarded unfinished death effects.");
  score_ = 1234;
  gameTimeSeconds_ = 6.0;
  (void)player_.TryTakeDamage();
  const int retainedHp = player_.GetHp();
  bulletSystem_.Spawn(100.0f, 100.0f);
  UpdateMinions(0.8);
  UpdateStageProgress();
  require(
      stage_.GetStageNumber() == 2 && HasBoss() &&
          enemy_.GetKind() == EnemyKind::MidBoss && minions_.empty() &&
          bulletSystem_.GetActiveCount() == 0 && score_ == 1234 &&
          player_.GetHp() == retainedHp && gameTimeSeconds_ == 6.0,
      "Stage one to midboss transition lost run state or retained bullets.");
  const float playerX = player_.GetX();
  while (enemy_.GetHp() > 0)
    (void)enemy_.TryTakeDamage();
  CheckBattleOutcome();
  UpdateStageProgress();
  require(battleOutcome_ == BattleOutcome::None &&
              stage_.GetPhase() == StagePhase::Transitioning,
          "Midboss death ended the entire game.");
  noInput.SetKeyDown(VK_RIGHT, true);
  noInput.SetKeyDown('Z', true);
  for (int i = 0; i < 24; ++i)
    (void)Update(noInput, 1.0 / 60.0);
  require(stage_.GetStageNumber() == 2 && player_.GetX() == playerX &&
              std::abs(enemy_.GetDissolveProgress() - 0.5f) < 0.00001f,
          "Midboss dissolve did not freeze combat during transition.");
  render();
  for (int i = 0; i < 24; ++i)
    (void)Update(noInput, 1.0 / 60.0);
  require(stage_.GetStageNumber() == 3 && HasBoss() &&
              enemy_.GetKind() == EnemyKind::FinalBoss &&
              battleOutcome_ == BattleOutcome::None && score_ == 1234 &&
              player_.GetHp() == retainedHp,
          "Final boss did not follow midboss without a Result/reset.");
  noInput.Reset();
  while (enemy_.GetHp() > enemy_.GetMaxHp() / 2)
    (void)enemy_.TryTakeDamage();
  std::size_t peakBullets = 0;
  // Deterministic pattern simulation without player collision, not a benchmark.
  for (int i = 0; i < 24 * 60; ++i) {
    bulletSystem_.Update(1.0 / 60.0);
    UpdateEnemyShooting(1.0 / 60.0);
    UpdateMinions(1.0 / 60.0);
    bulletSystem_.RemoveOutside(kPlayfieldWidth, kPlayfieldHeight);
    peakBullets = std::max(peakBullets, bulletSystem_.GetActiveCount());
    require(minions_.size() <= 8, "Final boss summons exceeded their bound.");
  }
  require(peakBullets >= 1000 && !minions_.empty() &&
              bulletSystem_.GetDroppedSpawnRequests() == 0,
          "Final boss did not sustain dense bullets and bounded summons.");
  render();
  std::cout << "STAGE TEST PASSED: 1 -> 2 -> 3, peak bullets=" << peakBullets
            << ", dropped=" << bulletSystem_.GetDroppedSpawnRequests() << '\n';
  while (enemy_.GetHp() > 0)
    (void)enemy_.TryTakeDamage();
  CheckBattleOutcome();
  require(battleOutcome_ == BattleOutcome::Clear && IsEnding() &&
              bulletSystem_.GetActiveCount() == 0 &&
              std::all_of(minions_.begin(), minions_.end(),
                          [](const Enemy &e) {
                            return e.GetLifeState() == EnemyLifeState::Dying;
                          }),
          "Final victory did not stop bullets and dissolve summons.");
  for (int i = 0; i < 48; ++i)
    (void)Update(noInput, 1.0 / 60.0);
  require(IsFinished() && minions_.empty(),
          "Final victory effects did not finish.");
  Reset();
  require(stage_.GetStageNumber() == 1 && stage_.GetSpawnedCount() == 1 &&
              !HasBoss() && bulletSystem_.GetSpawnRequests() == 0 &&
              bulletSystem_.GetDroppedSpawnRequests() == 0 && score_ == 0,
          "Restart retained stage or pattern/capacity statistics.");
  for (int stageNumber = 1; stageNumber <= 3; ++stageNumber) {
    Reset();
    for (int i = 1; i < stageNumber; ++i)
      stage_.Advance();
    StartStage();
    while (player_.GetHp() > 0) {
      (void)player_.TryTakeDamage();
      player_.Update(0.0f, 0.0f, 1.0);
    }
    CheckBattleOutcome();
    require(battleOutcome_ == BattleOutcome::Failed && IsEnding(),
            "Player death did not fail the current stage.");
  }
  Reset();
}
