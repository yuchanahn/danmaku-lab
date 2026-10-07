#include "Enemy.h"
#include <algorithm>
#include <cmath>
#include <numbers>

Enemy::Enemy(EnemyKind kind, float x, float y)
    : kind_(kind), spawnX_(x), spawnY_(y) {
  if (kind == EnemyKind::Enemy1 || kind == EnemyKind::Enemy2) {
    maxHp_ = kind == EnemyKind::Enemy1 ? 6 : 8;
    width_ = height_ = 60.0f;
    hitRadius_ = 18.0f;
    movementSpeed_ = kind == EnemyKind::Enemy1 ? 24.0f : 18.0f;
    horizontalAmplitude_ = kind == EnemyKind::Enemy2 ? 64.0f : 0.0f;
    shotIntervalSeconds_ = kind == EnemyKind::Enemy1 ? 1.5 : 1.8;
  } else {
    maxHp_ = kind == EnemyKind::FinalBoss ? 240 : 120;
    hitRadius_ = kind == EnemyKind::FinalBoss ? 56.0f : 24.0f;
    horizontalAmplitude_ = kind == EnemyKind::FinalBoss ? 150.0f : 80.0f;
    horizontalPeriodSeconds_ = kind == EnemyKind::FinalBoss ? 4.0 : 5.0;
  }
  Reset();
}

void Enemy::Reset() noexcept {
  hp_ = maxHp_;
  x_ = spawnX_;
  y_ = spawnY_;
  lifeState_ = EnemyLifeState::Alive;
  aliveSeconds_ = deathSeconds_ = 0.0;
  shotCooldownSeconds_ = 1.0;
}

float Enemy::ComputeHorizontalOffset(double elapsedSeconds, float amplitude,
                                     double periodSeconds) {
  if (periodSeconds <= 0.0 || amplitude == 0.0f)
    return 0.0f;
  return static_cast<float>(
      std::sin(elapsedSeconds / periodSeconds * std::numbers::pi * 2.0) *
      amplitude);
}

void Enemy::Update(double fixedDeltaSeconds) {
  if (lifeState_ == EnemyLifeState::Removed)
    return;
  if (lifeState_ == EnemyLifeState::Dying) {
    deathSeconds_ =
        std::min(kDissolveDurationSeconds, deathSeconds_ + fixedDeltaSeconds);
    if (deathSeconds_ + 1e-9 >= kDissolveDurationSeconds) {
      deathSeconds_ = kDissolveDurationSeconds;
      lifeState_ = EnemyLifeState::Removed;
    }
    return;
  }
  aliveSeconds_ += fixedDeltaSeconds;
  shotCooldownSeconds_ -= fixedDeltaSeconds;
  y_ = std::min(spawnY_ + 80.0f,
                y_ + movementSpeed_ * static_cast<float>(fixedDeltaSeconds));
  if (horizontalAmplitude_ > 0.0f) {
    x_ = spawnX_ + ComputeHorizontalOffset(aliveSeconds_, horizontalAmplitude_,
                                           horizontalPeriodSeconds_);
  }
}

bool Enemy::ConsumeShot(double intervalSeconds) noexcept {
  if (lifeState_ != EnemyLifeState::Alive || shotCooldownSeconds_ > 0.0)
    return false;
  shotCooldownSeconds_ =
      intervalSeconds > 0.0 ? intervalSeconds : shotIntervalSeconds_;
  return true;
}

float Enemy::GetDissolveProgress() const noexcept {
  return static_cast<float>(
      std::clamp(deathSeconds_ / kDissolveDurationSeconds, 0.0, 1.0));
}

bool Enemy::TryTakeDamage(int damage) noexcept {
  if (lifeState_ != EnemyLifeState::Alive || damage <= 0) {
    return false;
  }
  hp_ = std::max(0, hp_ - damage);
  if (hp_ == 0) {
    lifeState_ = EnemyLifeState::Dying;
    deathSeconds_ = 0.0;
  }
  return true;
}
