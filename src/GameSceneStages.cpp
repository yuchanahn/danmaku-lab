#include "DanmakuPattern.h"
#include "GameScene.h"
#include <algorithm>
#include <cmath>
#include <numbers>

void GameScene::SpawnScheduledEnemies() {
  while (const auto spawn = stage_.ConsumeSpawn())
    minions_.emplace_back(spawn->kind, spawn->x, spawn->y);
}

void GameScene::StartStage() {
  pendingPlayerShots_.clear();
  bulletSystem_.Clear();
  enemyBulletGrid_.Clear();
  activeEnemyBulletCount_ = collisionCandidateCount_ = 0;
  minions_.clear();
  bossPresent_ = stage_.GetStageNumber() != 1;
  bossVolleyIndex_ = 0;
  summonCooldownSeconds_ = 3.0;
  if (bossPresent_)
    enemy_ = Enemy(stage_.GetStageNumber() == 2 ? EnemyKind::MidBoss
                                                : EnemyKind::FinalBoss,
                   360.0f, 140.0f);
  SpawnScheduledEnemies();
}

void GameScene::UpdateStageProgress() {
  if (rewardPending_) return;
  if (stage_.GetStageNumber() == 2 && bossPresent_ && enemy_.GetHp() == 0 &&
      stage_.GetPhase() == StagePhase::Running) {
    stage_.BeginBossTransition();
    pendingPlayerShots_.clear();
    bulletSystem_.Clear();
    enemyBulletGrid_.Clear();
    activeEnemyBulletCount_ = collisionCandidateCount_ = 0;
  }
  if (stage_.CanAdvance(minions_.empty(),
                        bossPresent_ &&
                            enemy_.GetLifeState() == EnemyLifeState::Removed)) {
    DropStageRewards();
    rewardPending_ = true;
  }
}

void GameScene::DropStageRewards() {
  bulletSystem_.Clear();
  enemyBulletGrid_.Clear();
  pendingPlayerShots_.clear();
  activeEnemyBulletCount_ = collisionCandidateCount_ = 0;
  playerHit_ = playerGraze_ = false;
  rewards_.DropPair(player_, kPlayfieldWidth, kPlayfieldHeight);
}

void GameScene::UpdateEnemyShooting(double fixedDeltaSeconds) {
  if (!bossPresent_ || enemy_.GetLifeState() != EnemyLifeState::Alive)
    return;
  enemy_.Update(fixedDeltaSeconds);
  const float age = static_cast<float>(enemy_.GetAnimationTimeSeconds());
  const float aimedAngle = std::atan2(player_.GetY() - enemy_.GetY(),
                                      player_.GetX() - enemy_.GetX());
  const bool enraged = enemy_.GetHp() <= enemy_.GetMaxHp() / 2;
  if (enemy_.GetKind() == EnemyKind::MidBoss) {
    if (!enemy_.ConsumeShot(enraged ? 0.65 : 1.0))
      return;
    if (enraged) {
      DanmakuPattern::SpawnRing(bulletSystem_, enemy_.GetX(), enemy_.GetY(), 24,
                                155.0f, age * 0.7f);
      DanmakuPattern::SpawnFan(bulletSystem_, enemy_.GetX(), enemy_.GetY(), 5,
                               190.0f, aimedAngle, 0.65f, BulletType::Thin);
    } else {
      DanmakuPattern::SpawnFan(bulletSystem_, enemy_.GetX(), enemy_.GetY(), 9,
                               150.0f, aimedAngle, 1.4f, BulletType::Thin);
    }
    return;
  }

  summonCooldownSeconds_ -= fixedDeltaSeconds;
  if (summonCooldownSeconds_ <= 0.0) {
    // Count both living and dissolving minions toward the summon limit.
    if (minions_.size() <= 6) {
      minions_.emplace_back(EnemyKind::Enemy1,
                            std::clamp(enemy_.GetX() - 140.0f, 110.0f, 610.0f),
                            enemy_.GetY() + 85.0f);
      minions_.emplace_back(EnemyKind::Enemy2,
                            std::clamp(enemy_.GetX() + 140.0f, 110.0f, 610.0f),
                            enemy_.GetY() + 85.0f);
    }
    summonCooldownSeconds_ = enraged ? 4.0 : 5.5;
  }
  if (!enemy_.ConsumeShot(enraged ? 0.20 : 0.30))
    return;
  ++bossVolleyIndex_;
  const int count = enraged ? 96 : 48;
  const float phase = age * 0.65f;
  const float halfStep = static_cast<float>(std::numbers::pi) / count;
  DanmakuPattern::SpawnRing(bulletSystem_, enemy_.GetX(), enemy_.GetY(), count,
                            110.0f, phase, BulletType::Normal);
  DanmakuPattern::SpawnRing(bulletSystem_, enemy_.GetX(), enemy_.GetY(), count,
                            145.0f, -phase + halfStep, BulletType::Thin);
  if (bossVolleyIndex_ % 3 == 0) {
    DanmakuPattern::SpawnFan(bulletSystem_, enemy_.GetX(), enemy_.GetY(),
                             enraged ? 11 : 7, 200.0f, aimedAngle, 0.8f,
                             BulletType::Thin);
  }
}
