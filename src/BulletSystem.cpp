#include "BulletSystem.h"

BulletSystem::BulletSystem() { bullets_.resize(kPoolCapacity); }

void BulletSystem::Reset() noexcept {
  for (auto &bullet : bullets_) {
    bullet = Bullet{};
  }
}

void BulletSystem::Spawn(float centerX, float centerY) {
  Spawn(centerX, centerY, 0.0f, -480.0f, BulletOwner::Player);
}

void BulletSystem::Spawn(float centerX, float centerY, float velocityX,
                         float velocityY, BulletOwner owner) {
  auto target = std::ranges::find(bullets_, false, &Bullet::active);
  if (target != bullets_.end()) {
    *target = Bullet{centerX, centerY,
                     velocityX,
                     velocityY,
                     20.0f,   // width
                     20.0f,   // height
                     owner,
                     false,
                     true};
  }
}

void BulletSystem::Update(double fixedDeltaSeconds) {
  for (auto &bullet : bullets_) {
    if (!bullet.active) {
      continue;
    }
    bullet.x += bullet.velocityX * static_cast<float>(fixedDeltaSeconds);
    bullet.y += bullet.velocityY * static_cast<float>(fixedDeltaSeconds);
  }
}

void BulletSystem::RemoveOutside(float screenWidth, float screenHeight) {
  if (screenWidth <= 0.0f || screenHeight <= 0.0f) {
    return;
  }

  for (auto &bullet : bullets_) {
    if (!bullet.active) {
      continue;
    }

    const float halfWidth = bullet.width * 0.5f;
    const float halfHeight = bullet.height * 0.5f;
    const bool outside =
        bullet.x + halfWidth < 0.0f || bullet.x - halfWidth > screenWidth ||
        bullet.y + halfHeight < 0.0f || bullet.y - halfHeight > screenHeight;
    if (outside) {
      bullet.active = false;
    }
  }
}

std::size_t BulletSystem::GetActiveCount() const noexcept {
  std::size_t count = 0;
  for (const auto &bullet : bullets_) {
    if (bullet.active) {
      ++count;
    }
  }
  return count;
}
