#include "BulletSystem.h"
#include <algorithm>
#include <cmath>

BulletSystem::BulletSystem() { bullets_.resize(kPoolCapacity); }

void BulletSystem::Reset() noexcept {
  spawnRequests_ = droppedSpawnRequests_ = 0;
  for (auto &bullet : bullets_) {
    bullet = Bullet{};
  }
}
void BulletSystem::Clear() noexcept {
  for (auto &bullet : bullets_)
    bullet.active = false;
}

void BulletSystem::Spawn(float centerX, float centerY) {
  Spawn(centerX, centerY, 0.0f, -480.0f, BulletOwner::Player, BulletType::Thin);
}

void BulletSystem::Spawn(float centerX, float centerY, float velocityX,
                         float velocityY, BulletOwner owner, BulletType type) {
  ++spawnRequests_;
  auto target = std::ranges::find(bullets_, false, &Bullet::active);
  if (target != bullets_.end()) {
    const auto style = GetBulletStyle(type);
    *target = Bullet{centerX,      centerY, velocityX, velocityY, style.width,
                     style.height, owner,   false,     true,      type};
  } else {
    ++droppedSpawnRequests_;
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

    // 회전 각도와 무관한 외접원으로 화면 밖 판정의 조기 반환을 방지한다.
    const float halfWidth = std::hypot(bullet.width, bullet.height) * 0.5f;
    const float halfHeight = halfWidth;
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
