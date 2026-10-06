#include "GameScene.h"
#include "Collision.h"
#include "DanmakuPattern.h"
#include <algorithm>
#include <cmath>
#include <numbers>

void GameScene::Reset() {
  player_.Reset();
  enemy_.Reset();
  minions_.clear();
  stage_.Reset();
  bossPresent_ = false;
  bulletSystem_.Reset();
  enemyBulletGrid_.Clear();
  battleOutcome_ = BattleOutcome::None;
  gameTimeSeconds_ = 0.0;
  battleEndingSeconds_ = 0.0;
  summonCooldownSeconds_ = 3.0;
  bossVolleyIndex_ = 0;
  playerMotion_ = PlayerMotion::Idle;
  visuals_.dissolvePreviewProgress = 0.0f;
  playerHit_ = false;
  playerGraze_ = false;
  score_ = 0;
  grazeCount_ = 0;
  shotRequestCount_ = 0;
  activeEnemyBulletCount_ = 0;
  collisionCandidateCount_ = 0;
  StartStage();
}

bool GameScene::Update(const Input &input, double fixedDeltaSeconds) {
  if (battleOutcome_ == BattleOutcome::None) {
    stage_.Update(fixedDeltaSeconds);
    if (stage_.GetPhase() == StagePhase::Transitioning) {
      enemy_.Update(fixedDeltaSeconds);
      UpdateStageProgress();
      gameTimeSeconds_ += fixedDeltaSeconds;
      return false;
    }
    SpawnScheduledEnemies();
    const bool fired = UpdateCombat(input, fixedDeltaSeconds);
    gameTimeSeconds_ += fixedDeltaSeconds;
    return fired;
  }
  if (IsEnding())
    UpdateBattleEnding(fixedDeltaSeconds);
  return false;
}

bool GameScene::UpdateCombat(const Input &input, double fixedDeltaSeconds) {
  float directionX = static_cast<float>(input.IsDown(VK_RIGHT)) -
                     static_cast<float>(input.IsDown(VK_LEFT));
  float directionY = static_cast<float>(input.IsDown(VK_DOWN)) -
                     static_cast<float>(input.IsDown(VK_UP));
  playerMotion_ = directionX < 0.0f   ? PlayerMotion::Left
                  : directionX > 0.0f ? PlayerMotion::Right
                                      : PlayerMotion::Idle;

  const float lengthSquared = directionX * directionX + directionY * directionY;
  if (lengthSquared > 0.0f) {
    const float length = std::sqrt(lengthSquared);
    directionX /= length;
    directionY /= length;
  }

  player_.Update(directionX, directionY, fixedDeltaSeconds);
  player_.ClampToBounds(kPlayfieldWidth, kPlayfieldHeight);

  bulletSystem_.Update(fixedDeltaSeconds);

  UpdateEnemyShooting(fixedDeltaSeconds);
  UpdateMinions(fixedDeltaSeconds);

  const bool fired =
      player_.UpdateShooting(input.IsDown('Z'), fixedDeltaSeconds);
  if (fired) {
    ++shotRequestCount_;
    bulletSystem_.Spawn(player_.GetX(),
                        player_.GetY() - player_.GetHeight() * 0.5f);
  }
  RebuildEnemyBulletGrid();
  CheckEnemyPlayerBulletCollisions();
  CheckPlayerEnemyBulletCollisions();
  CheckBattleOutcome();
  if (battleOutcome_ == BattleOutcome::None)
    UpdateStageProgress();
  bulletSystem_.RemoveOutside(kPlayfieldWidth, kPlayfieldHeight);
  return fired;
}

void GameScene::CheckEnemyPlayerBulletCollisions() {
  for (auto &bullet : bulletSystem_.GetBullets()) {
    if (!bullet.active || bullet.owner != BulletOwner::Player)
      continue;
    const CircleHitbox bulletHitbox{
        bullet.x,
        bullet.y,
        GetBulletStyle(bullet.type).hitRadius,
    };
    const auto tryHit = [&](Enemy &target) {
      if (target.GetLifeState() != EnemyLifeState::Alive)
        return false;
      if (!Intersects({target.GetX(), target.GetY(), target.GetHitRadius()},
                      bulletHitbox))
        return false;
      bullet.active = false;
      DamageEnemy(target, 1);
      return true;
    };
    if (bossPresent_ && tryHit(enemy_))
      continue;
    for (auto &minion : minions_) {
      if (tryHit(minion))
        break;
    }
  }
}

void GameScene::DamageEnemy(Enemy &enemy, int damage) {
  if (enemy.TryTakeDamage(damage) && enemy.GetHp() == 0)
    score_ += enemy.GetKind() == EnemyKind::FinalBoss ? 5000
              : enemy.GetKind() == EnemyKind::MidBoss ? 1000
                                                      : 100;
}

void GameScene::ApplyDamageCheat() {
  if (battleOutcome_ != BattleOutcome::None ||
      stage_.GetPhase() != StagePhase::Running)
    return;
  if (bossPresent_)
    DamageEnemy(enemy_, 50);
  for (auto &minion : minions_)
    DamageEnemy(minion, 50);
  CheckBattleOutcome();
  if (battleOutcome_ == BattleOutcome::None)
    UpdateStageProgress();
}

void GameScene::UpdateMinions(double fixedDeltaSeconds) {
  for (auto &minion : minions_) {
    minion.Update(fixedDeltaSeconds);
    if (!minion.ConsumeShot())
      continue;
    const float angle = std::atan2(player_.GetY() - minion.GetY(),
                                   player_.GetX() - minion.GetX());
    if (minion.GetKind() == EnemyKind::Enemy1) {
      DanmakuPattern::SpawnFan(bulletSystem_, minion.GetX(), minion.GetY(), 1,
                               140.0f, angle, 0.0f, BulletType::Normal);
    } else {
      DanmakuPattern::SpawnFan(bulletSystem_, minion.GetX(), minion.GetY(), 3,
                               120.0f, angle, 0.5f, BulletType::Thin);
    }
  }
  std::erase_if(minions_, [](const Enemy &minion) {
    return minion.GetLifeState() == EnemyLifeState::Removed;
  });
}

void GameScene::RebuildEnemyBulletGrid() {
  activeEnemyBulletCount_ = 0;
  if (collisionMode_ == CollisionMode::LinearScan)
    return;
  enemyBulletGrid_.Clear();

  const auto &bullets = bulletSystem_.GetBullets();
  for (std::size_t bulletIndex = 0; bulletIndex < bullets.size();
       ++bulletIndex) {
    const auto &bullet = bullets[bulletIndex];
    if (!bullet.active || bullet.owner != BulletOwner::Enemy) {
      continue;
    }
    ++activeEnemyBulletCount_;

    if (bullet.x < 0.0f || bullet.y < 0.0f || bullet.x >= kPlayfieldWidth ||
        bullet.y >= kPlayfieldHeight) {
      continue;
    }

    auto cellX = static_cast<std::size_t>(bullet.x / kCollisionGridCellSize);
    auto cellY = static_cast<std::size_t>(bullet.y / kCollisionGridCellSize);

    if (cellY >= enemyBulletGrid_.GetRows() ||
        cellX >= enemyBulletGrid_.GetColumns()) {
      continue;
    }

    enemyBulletGrid_.Insert(cellX, cellY, bulletIndex);
  }
}

void GameScene::CheckPlayerEnemyBulletCollisions() {
  constexpr float kPlayerHitRadius = 5.0f;
  constexpr float kPlayerGrazeRadius = 24.0f;

  playerHit_ = false;
  playerGraze_ = false;
  collisionCandidateCount_ = 0;
  const CircleHitbox playerHitbox{
      player_.GetX(),
      player_.GetY(),
      kPlayerHitRadius,
  };
  const CircleHitbox playerGrazeHitbox{
      player_.GetX(),
      player_.GetY(),
      kPlayerGrazeRadius,
  };

  auto &bullets = bulletSystem_.GetBullets();
  // 두 경로는 후보 선택만 다르고 명중/무적/Graze 처리는 공유한다.
  const auto checkBullet = [&](Bullet &bullet) {
    if (!bullet.active || bullet.owner != BulletOwner::Enemy)
      return;
    const CircleHitbox bulletHitbox{bullet.x, bullet.y,
                                    GetBulletStyle(bullet.type).hitRadius};
    ++collisionCandidateCount_;
    const bool bulletHitsPlayer = Intersects(playerHitbox, bulletHitbox);
    if (bulletHitsPlayer) {
      const bool damageApplied = player_.TryTakeDamage();
      playerHit_ = playerHit_ || damageApplied;
      bullet.active = false;
    }
    const bool isGraze =
        Intersects(playerGrazeHitbox, bulletHitbox) && !bulletHitsPlayer;
    playerGraze_ = playerGraze_ || isGraze;
    if (isGraze && !bullet.grazed) {
      bullet.grazed = true;
      ++grazeCount_;
      score_ += kGrazeScore;
    }
  };

  if (collisionMode_ == CollisionMode::LinearScan) {
    activeEnemyBulletCount_ = 0;
    for (auto &bullet : bullets) {
      if (bullet.active && bullet.owner == BulletOwner::Enemy)
        ++activeEnemyBulletCount_;
      checkBullet(bullet);
    }
    return;
  }

  const std::size_t playerCellX =
      static_cast<std::size_t>(player_.GetX() / kCollisionGridCellSize);
  const std::size_t playerCellY =
      static_cast<std::size_t>(player_.GetY() / kCollisionGridCellSize);

  const std::size_t minCellX = playerCellX > 0 ? playerCellX - 1 : 0;
  const std::size_t minCellY = playerCellY > 0 ? playerCellY - 1 : 0;
  const std::size_t maxCellX =
      std::min(playerCellX + 1, enemyBulletGrid_.GetColumns() - 1);
  const std::size_t maxCellY =
      std::min(playerCellY + 1, enemyBulletGrid_.GetRows() - 1);

  for (std::size_t cellY = minCellY; cellY <= maxCellY; ++cellY) {
    for (std::size_t cellX = minCellX; cellX <= maxCellX; ++cellX) {
      for (const std::size_t bulletIndex :
           enemyBulletGrid_.GetCell(cellX, cellY)) {
        checkBullet(bullets[bulletIndex]);
      }
    }
  }
}

void GameScene::SetCollisionMode(CollisionMode mode) {
  if (collisionMode_ == mode)
    return;
  collisionMode_ = mode;
  enemyBulletGrid_.Clear();
  collisionCandidateCount_ = 0;
}

void GameScene::CheckBattleOutcome() {
  if (battleOutcome_ != BattleOutcome::None)
    return;
  if (player_.GetHp() == 0) {
    battleOutcome_ = BattleOutcome::Failed;
  } else if (stage_.GetStageNumber() == 3 && bossPresent_ &&
             enemy_.GetHp() == 0) {
    battleOutcome_ = BattleOutcome::Clear;
    stage_.Finish();
    bulletSystem_.Clear();
    activeEnemyBulletCount_ = collisionCandidateCount_ = 0;
    for (auto &minion : minions_) {
      while (minion.GetHp() > 0)
        (void)minion.TryTakeDamage();
    }
  }
  if (battleOutcome_ != BattleOutcome::None)
    battleEndingSeconds_ = 0.0;
}

void GameScene::UpdateBattleEnding(double fixedDeltaSeconds) {
  for (auto &minion : minions_) {
    if (minion.GetLifeState() == EnemyLifeState::Dying)
      minion.Update(fixedDeltaSeconds);
  }
  std::erase_if(minions_, [](const Enemy &minion) {
    return minion.GetLifeState() == EnemyLifeState::Removed;
  });
  battleEndingSeconds_ = std::min(kDeathDissolveDurationSeconds,
                                  battleEndingSeconds_ + fixedDeltaSeconds);
}

float GameScene::GetDeathDissolveProgress() const noexcept {
  return static_cast<float>(std::clamp(
      battleEndingSeconds_ / kDeathDissolveDurationSeconds, 0.0, 1.0));
}

bool GameScene::IsEnding() const {
  return battleOutcome_ != BattleOutcome::None && !IsFinished();
}

bool GameScene::IsFinished() const {
  return battleOutcome_ != BattleOutcome::None &&
         battleEndingSeconds_ >= kDeathDissolveDurationSeconds;
}

SceneStatistics GameScene::GetStatistics() const {
  return {score_,
          grazeCount_,
          shotRequestCount_,
          activeEnemyBulletCount_,
          collisionCandidateCount_,
          playerHit_,
          playerGraze_};
}

void GameScene::HandleVisualControls(const Input &input, bool allowPreview) {
  if (input.GetKeyState('B') == Input::KeyState::Pressed) {
    visuals_.enhancedBullets = !visuals_.enhancedBullets;
  }
  if (allowPreview && input.GetKeyState(VK_F3) == Input::KeyState::Pressed) {
    visuals_.dissolvePreviewProgress += 0.25f;
    if (visuals_.dissolvePreviewProgress > 1.0f) {
      visuals_.dissolvePreviewProgress = 0.0f;
    }
  }
  if (input.GetKeyState('1') == Input::KeyState::Pressed) {
    visuals_.playerUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
  }
  if (input.GetKeyState('2') == Input::KeyState::Pressed) {
    visuals_.playerUvRect = {0.0f, 0.0f, 0.5f, 0.5f};
  }
  if (input.GetKeyState('3') == Input::KeyState::Pressed) {
    visuals_.playerUvRect = {1.0f, 0.0f, 0.0f, 1.0f};
  }
  if (input.GetKeyState('4') == Input::KeyState::Pressed) {
    visuals_.bulletShape = SpriteShape::Rectangle;
  }
  if (input.GetKeyState('5') == Input::KeyState::Pressed) {
    visuals_.bulletShape = SpriteShape::SoftCircle;
  }
  if (input.GetKeyState('6') == Input::KeyState::Pressed) {
    visuals_.bulletShape = SpriteShape::GlowCircle;
  }
  if (input.GetKeyState('7') == Input::KeyState::Pressed) {
    visuals_.bulletBlendMode = SpriteBlendMode::Alpha;
  }
  if (input.GetKeyState('8') == Input::KeyState::Pressed) {
    visuals_.bulletBlendMode = SpriteBlendMode::Additive;
  }
  if (input.GetKeyState('9') == Input::KeyState::Pressed) {
    visuals_.playerRotationRadians += 0.25f;
  }
  if (input.GetKeyState('0') == Input::KeyState::Pressed) {
    visuals_.playerRotationRadians = 0.0f;
  }
  if (input.GetKeyState('N') == Input::KeyState::Pressed) {
    visuals_.playerSamplerMode = SpriteSamplerMode::Point;
  }
  if (input.GetKeyState('L') == Input::KeyState::Pressed) {
    visuals_.playerSamplerMode = SpriteSamplerMode::Linear;
  }
  if (input.GetKeyState('C') == Input::KeyState::Pressed) {
    visuals_.playerAddressMode = SpriteAddressMode::Clamp;
    visuals_.playerUvRect = {0.0f, 0.0f, 2.0f, 2.0f};
  }
  if (input.GetKeyState('W') == Input::KeyState::Pressed) {
    visuals_.playerAddressMode = SpriteAddressMode::Wrap;
    visuals_.playerUvRect = {0.0f, 0.0f, 2.0f, 2.0f};
  }
}
