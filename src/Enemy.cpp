#include "Enemy.h"

void Enemy::Reset() noexcept { hp_ = kMaxHp; }

bool Enemy::TryTakeDamage() noexcept {
  if (hp_ <= 0) {
    return false;
  }
  --hp_;
  return true;
}
