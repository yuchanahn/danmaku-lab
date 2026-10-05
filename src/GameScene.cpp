#include "GameScene.h"
#include "Collision.h"
#include "DanmakuPattern.h"
#include <algorithm>
#include <cmath>
#include <numbers>

void GameScene::Reset() {
  player_.Reset();
  enemy_.Reset();
  bulletSystem_.Reset();
  enemyBulletGrid_.Clear();
  battleOutcome_ = BattleOutcome::None;
  gameTimeSeconds_ = 0.0;
  battleEndingSeconds_ = 0.0;
  enemyShotCooldownSeconds_ = 0.0;
  enemyFanAngleRadians_ = 0.0f;
  playerMotion_ = PlayerMotion::Idle;
  visuals_.dissolvePreviewProgress = 0.0f;
  playerHit_ = false;
  playerGraze_ = false;
  score_ = 0;
  grazeCount_ = 0;
  shotRequestCount_ = 0;
  activeEnemyBulletCount_ = 0;
  collisionCandidateCount_ = 0;
}

bool GameScene::Update(const Input &input, double fixedDeltaSeconds) {
  if (battleOutcome_ == BattleOutcome::None) {
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
  bulletSystem_.RemoveOutside(kPlayfieldWidth, kPlayfieldHeight);
  return fired;
}

void GameScene::UpdateEnemyShooting(double fixedDeltaSeconds) {
  constexpr float kFanAngularSpeedRadiansPerSecond = 0.7f;
  enemyFanAngleRadians_ +=
      kFanAngularSpeedRadiansPerSecond * static_cast<float>(fixedDeltaSeconds);

  enemyShotCooldownSeconds_ -= fixedDeltaSeconds;
  if (enemy_.GetHp() > enemy_.GetMaxHp() / 2 &&
      enemyShotCooldownSeconds_ <= 0.0) {
    constexpr int kFanBulletCount = 7;
    constexpr float kFanBulletSpeed = 140.0f;
    constexpr float kFanCenterAngle =
        static_cast<float>(std::numbers::pi / 2.0);
    constexpr float kFanSpreadAngle =
        static_cast<float>(std::numbers::pi / 2.0);
    float fanCenterAngle = kFanCenterAngle;
    fanCenterAngle += enemyFanAngleRadians_;
    DanmakuPattern::SpawnFan(bulletSystem_, enemy_.GetX(), enemy_.GetY(),
                             kFanBulletCount, kFanBulletSpeed, fanCenterAngle,
                             kFanSpreadAngle);
    enemyShotCooldownSeconds_ = 1.5;
  } else if (enemy_.GetHp() > 0 && enemyShotCooldownSeconds_ <= 0.0) {
    constexpr int kFanBulletCount = 16;
    constexpr float kFanBulletSpeed = 160.0f;

    DanmakuPattern::SpawnRing(bulletSystem_, enemy_.GetX(), enemy_.GetY(),
                              kFanBulletCount, kFanBulletSpeed,
                              enemyFanAngleRadians_);
    enemyShotCooldownSeconds_ = 1.0;
  }
}

void GameScene::CheckEnemyPlayerBulletCollisions() {

  constexpr float kEnemyHitRadius = 24.0f;
  constexpr float kPlayerBulletHitRadius = 4.0f;
  const CircleHitbox Hitbox{
      enemy_.GetX(),
      enemy_.GetY(),
      kEnemyHitRadius,
  };

  for (auto &bullet : bulletSystem_.GetBullets()) {
    if (!bullet.active || bullet.owner != BulletOwner::Player)
      continue;
    const CircleHitbox bulletHitbox{
        bullet.x,
        bullet.y,
        kPlayerBulletHitRadius,
    };
    const bool bulletHits = Intersects(Hitbox, bulletHitbox);

    if (bulletHits) {
      bullet.active = false;
      (void)enemy_.TryTakeDamage();
    }
  }
}

void GameScene::RebuildEnemyBulletGrid() {
  enemyBulletGrid_.Clear();
  activeEnemyBulletCount_ = 0;

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
  constexpr float kEnemyBulletHitRadius = 8.0f;

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

  auto &bullets = bulletSystem_.GetBullets();
  for (std::size_t cellY = minCellY; cellY <= maxCellY; ++cellY) {
    for (std::size_t cellX = minCellX; cellX <= maxCellX; ++cellX) {
      for (const std::size_t bulletIndex :
           enemyBulletGrid_.GetCell(cellX, cellY)) {
        auto &bullet = bullets[bulletIndex];
        if (!bullet.active) {
          continue;
        }

        const CircleHitbox bulletHitbox{
            bullet.x,
            bullet.y,
            kEnemyBulletHitRadius,
        };

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
          grazeCount_++;
          score_ += kGrazeScore;
        }
      }
    }
  }
}

void GameScene::CheckBattleOutcome() {
  if (battleOutcome_ != BattleOutcome::None)
    return;
  if (player_.GetHp() == 0) {
    battleOutcome_ = BattleOutcome::Failed;
  } else if (enemy_.GetHp() == 0) {
    battleOutcome_ = BattleOutcome::Clear;
  }
  if (battleOutcome_ != BattleOutcome::None)
    battleEndingSeconds_ = 0.0;
}

void GameScene::UpdateBattleEnding(double fixedDeltaSeconds) {
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
