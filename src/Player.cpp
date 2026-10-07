#include "Player.h"

#include <algorithm>
#include <cmath>

void Player::Reset() noexcept {
  fireRateLevel_ = damageLevel_ = burstLevel_ = 0;
  x_ = 360.0f;
  y_ = 800.0f;
  shotCooldownSeconds_ = 0.0;
  hp_ = kMaxHp;
  invulnerabilitySeconds_ = 0.0;
}

void Player::Update(float directionX, float directionY,
                    double fixedDeltaSeconds) {

  invulnerabilitySeconds_ =
      std::max(0.0, invulnerabilitySeconds_ - fixedDeltaSeconds);

  x_ += directionX * static_cast<float>(fixedDeltaSeconds) *
        speedPixelsPerSecond_;
  y_ += directionY * static_cast<float>(fixedDeltaSeconds) *
        speedPixelsPerSecond_;
}

bool Player::TryTakeDamage() noexcept {

  if (hp_ == 0 || invulnerabilitySeconds_ > 0) {
    return false;
  }

  invulnerabilitySeconds_ = kInvulnerabilityDurationSeconds;
  hp_ -= 1;
  return true;
}

bool Player::UpdateShooting(bool shootHeld, double fixedDeltaSeconds) {
  shotCooldownSeconds_ -= fixedDeltaSeconds;
  shotCooldownSeconds_ = std::max(shotCooldownSeconds_, 0.0);

  if (shootHeld && shotCooldownSeconds_ <= 0.f) {
    shotCooldownSeconds_ = GetShotInterval();
    return true;
  }

  return false;
}

void Player::ApplyUpgrade(PlayerUpgrade kind) noexcept {
  switch (kind) {
  case PlayerUpgrade::FireRate: ++fireRateLevel_; break;
  case PlayerUpgrade::Damage: ++damageLevel_; break;
  case PlayerUpgrade::Burst: ++burstLevel_; break;
  }
}

double Player::GetShotInterval() const noexcept {
  return kShotIntervalSeconds * std::pow(0.8, fireRateLevel_);
}

void Player::ClampToBounds(float screenWidth, float screenHeight) {
  if (screenWidth <= 0.0f || screenHeight <= 0.0f) {
    return;
  }

  const float marginX = std::min(width_ * 0.5f, screenWidth * 0.5f);
  const float marginY = std::min(height_ * 0.5f, screenHeight * 0.5f);
  x_ = std::clamp(x_, marginX, screenWidth - marginX);
  y_ = std::clamp(y_, marginY, screenHeight - marginY);
}
