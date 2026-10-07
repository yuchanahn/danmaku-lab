#include "GameScene.h"
#include "GameSpriteRenderer.h"
#include "Graphics.h"
#include <algorithm>
#include <ranges>
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
  const auto pickUpAll = [&] {
    for (const auto item : rewards_.GetPickups()) {
      if (!item.active) continue;
      player_.Update((item.x - player_.GetX()) / 240.0f,
                     (item.y - player_.GetY()) / 240.0f, 1.0);
      rewards_.Update(player_, 0.0);
    }
  };
  const auto collectRewards = [&](int nextStage) {
    require(rewardPending_ && !rewards_.Empty(), "Missing stage clear rewards.");
    require(std::ranges::count(rewards_.GetPickups(), true, &UpgradePickup::active) == 2,
            "A clear must drop exactly two items.");
    const int upgrades = player_.GetUpgradeCount();
    UpdateStageProgress();
    render();
    noInput.Reset();
    pickUpAll();
    for (int i = 0; i < 300 && stage_.GetStageNumber() != nextStage; ++i)
      (void)Update(noInput, 1.0 / 60.0);
    require(stage_.GetStageNumber() == nextStage && rewards_.Empty() &&
                player_.GetUpgradeCount() == upgrades + 2 && !rewardPending_,
            "Pickup duplicated upgrades or did not advance the stage.");
  };
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
  collectRewards(2);
  require(
      stage_.GetStageNumber() == 2 && HasBoss() &&
          enemy_.GetKind() == EnemyKind::MidBoss && minions_.empty() &&
          bulletSystem_.GetActiveCount() == 0 && score_ == 1234 &&
          player_.GetHp() == retainedHp && gameTimeSeconds_ > 6.0,
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
  collectRewards(3);
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
  SetInstancedBullets(false);
  render();
  SetInstancedBullets(true);
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
  pickUpAll();
  for (int i = 0; i < 48; ++i)
    (void)Update(noInput, 1.0 / 60.0);
  require(IsFinished() && minions_.empty(),
          "Final victory effects did not finish.");
  require(player_.GetUpgradeCount() == 6 && rewards_.Empty(),
          "Final clear did not award its two items.");
  Reset();
  require(stage_.GetStageNumber() == 1 && stage_.GetSpawnedCount() == 1 &&
              !HasBoss() && bulletSystem_.GetSpawnRequests() == 0 &&
              bulletSystem_.GetDroppedSpawnRequests() == 0 && score_ == 0 &&
              player_.GetUpgradeCount() == 0 && rewards_.Empty() &&
              pendingPlayerShots_.empty() && !rewardPending_,
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
  player_.ApplyUpgrade(PlayerUpgrade::FireRate);
  player_.ApplyUpgrade(PlayerUpgrade::Damage);
  player_.ApplyUpgrade(PlayerUpgrade::Burst);
  require(std::abs(player_.GetShotInterval() - 0.12) < 0.000001 &&
              player_.GetDamage() == 2 && player_.GetBurstCount() == 2,
          "Weapon upgrade values are incorrect.");
  require(UpdatePlayerShooting(true, 1.0 / 60.0) &&
              bulletSystem_.GetActiveCount() == 1 && pendingPlayerShots_.size() == 1,
          "Burst emitted both bullets at once.");
  player_.ApplyUpgrade(PlayerUpgrade::Damage);
  for (int i = 0; i < 2; ++i)
    require(!UpdatePlayerShooting(false, 1.0 / 60.0), "Burst delay was too short.");
  require(UpdatePlayerShooting(false, 1.0 / 60.0) &&
              bulletSystem_.GetActiveCount() == 2 && pendingPlayerShots_.empty(),
          "Queued burst did not emit after its delay.");
  require(bulletSystem_.GetBullets()[0].damage == 2 &&
              bulletSystem_.GetBullets()[1].damage == 2,
          "An upgrade retroactively changed a queued/in-flight shot.");
  bulletSystem_.Clear();
  const int hpBefore = minions_.front().GetHp();
  bulletSystem_.Spawn(minions_.front().GetX(), minions_.front().GetY(),
                      0.0f, -480.0f, BulletOwner::Player, BulletType::Thin,
                      player_.GetDamage());
  CheckEnemyPlayerBulletCollisions();
  require(minions_.front().GetHp() == hpBefore - 3 && bulletSystem_.GetActiveCount() == 0,
          "Upgraded bullet damage was not applied on collision.");
  DropStageRewards();
  rewardPending_ = true;
  noInput.Reset();
  noInput.SetKeyDown(VK_RIGHT, true);
  noInput.SetKeyDown('Z', true);
  const float beforePickupX = player_.GetX();
  (void)Update(noInput, 1.0 / 60.0);
  require(player_.GetX() > beforePickupX && bulletSystem_.GetActiveCount() == 0,
          "Reward collection did not allow movement or emitted combat bullets.");
  Reset();
  require(player_.GetUpgradeCount() == 0 && player_.GetDamage() == 1 &&
              player_.GetBurstCount() == 1 && rewards_.Empty() && pendingPlayerShots_.empty(),
          "Restart retained upgrades or rewards.");
  DropStageRewards();
  const float initialItemX = rewards_.GetPickups()[0].x;
  const float initialItemY = rewards_.GetPickups()[0].y;
  player_.Update(-1.0f, 0.0f, 1.0);
  rewards_.Update(player_, 0.5);
  require(player_.GetUpgradeCount() == 0 && !rewards_.Empty() &&
              std::abs(rewards_.GetPickups()[0].x - initialItemX) < 10.0f &&
              std::abs(rewards_.GetPickups()[0].y - initialItemY - 30.0f) < 0.001f,
          "Rewards still home toward the player instead of drifting.");
  pickUpAll();
  rewards_.Update(player_, 0.0);
  require(rewards_.Empty() && player_.GetUpgradeCount() == 2,
          "Contact pickup duplicated or missed upgrades.");
  Reset();
  DropStageRewards();
  rewardPending_ = true;
  noInput.Reset();
  player_.Update(-1.0f, 0.0f, 1.0);
  (void)Update(noInput, 10.0);
  require(rewards_.Empty() && player_.GetUpgradeCount() == 0 &&
              stage_.GetStageNumber() == 2 && !rewardPending_,
          "Expired rewards granted upgrades or blocked stage progression.");
  Reset();
  stage_.Advance();
  stage_.Advance();
  StartStage();
  const int bossHp = enemy_.GetHp();
  bulletSystem_.Spawn(enemy_.GetX() + 50.0f, enemy_.GetY(), 0.0f, 0.0f,
                      BulletOwner::Player, BulletType::Thin);
  CheckEnemyPlayerBulletCollisions();
  require(enemy_.GetHp() == bossHp - 1, "Expanded boss hitbox missed a body hit.");
  bulletSystem_.Spawn(enemy_.GetX() + 75.0f, enemy_.GetY(), 0.0f, 0.0f,
                      BulletOwner::Player, BulletType::Thin);
  CheckEnemyPlayerBulletCollisions();
  require(enemy_.GetHp() == bossHp - 1, "Expanded boss hitbox accepted an outside hit.");
  DamageEnemy(enemy_, enemy_.GetHp());
  CheckBattleOutcome();
  const float endingPlayerX = player_.GetX();
  noInput.SetKeyDown(VK_RIGHT, true);
  (void)Update(noInput, 1.0 / 60.0);
  require(player_.GetX() > endingPlayerX, "Final reward collection blocked movement.");
  noInput.Reset();
  player_.Update(-1.0f, 0.0f, 1.0);
  (void)Update(noInput, 10.0);
  require(IsFinished() && player_.GetUpgradeCount() == 0,
          "Final missed rewards blocked Result or granted upgrades.");
  Reset();
  std::cout << "UPGRADE TEST PASSED: two rewards per clear, retained/reset stats, timed burst and hit damage\n";
}
