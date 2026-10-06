#include "BulletSystem.h"
#include <algorithm>
#include <cmath>
#include <cassert>
#include <chrono>
#include <functional>

namespace {
using Clock = std::chrono::steady_clock;
double ElapsedMilliseconds(Clock::time_point start) {
  return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}
}

BulletSystem::BulletSystem() {
  bullets_.resize(kPoolCapacity);
  freeIndices_.reserve(kPoolCapacity);
  RebuildFreeIndices();
}

void BulletSystem::RebuildFreeIndices() noexcept {
  freeIndices_.clear();
  if (allocationMode_ == BulletAllocationMode::LinearScan)
    return;
  for (std::size_t index = 0; index < bullets_.size(); ++index)
    if (!bullets_[index].active)
      freeIndices_.push_back(index);
  std::ranges::make_heap(freeIndices_, std::greater<>{});
}

void BulletSystem::SetAllocationMode(BulletAllocationMode mode) {
  allocationMode_ = mode;
  RebuildFreeIndices();
}

void BulletSystem::Reset() noexcept {
  measuring_ = false;
  measurement_ = {};
  spawnRequests_ = droppedSpawnRequests_ = 0;
  for (auto &bullet : bullets_) {
    bullet = Bullet{};
  }
  RebuildFreeIndices();
}
void BulletSystem::Clear() noexcept {
  for (auto &bullet : bullets_)
    bullet.active = false;
  RebuildFreeIndices();
}

void BulletSystem::Release(std::size_t index) noexcept {
  if (index >= bullets_.size() || !bullets_[index].active)
    return;
  const auto start = measuring_ ? Clock::now() : Clock::time_point{};
  bullets_[index].active = false;
  if (allocationMode_ == BulletAllocationMode::FreeIndexHeap) {
    freeIndices_.push_back(index);
    std::ranges::push_heap(freeIndices_, std::greater<>{});
  }
  if (measuring_) {
    measurement_.releaseMilliseconds += ElapsedMilliseconds(start);
    ++measurement_.released;
  }
}

void BulletSystem::Release(Bullet &bullet) noexcept {
  // 이 풀에서 가져온 참조만 받는다. 슬롯 배열은 수명 동안 재할당하지 않는다.
  assert(&bullet >= bullets_.data() && &bullet < bullets_.data() + bullets_.size());
  Release(static_cast<std::size_t>(&bullet - bullets_.data()));
}

void BulletSystem::Spawn(float centerX, float centerY) {
  Spawn(centerX, centerY, 0.0f, -480.0f, BulletOwner::Player, BulletType::Thin);
}

void BulletSystem::Spawn(float centerX, float centerY, float velocityX,
                         float velocityY, BulletOwner owner, BulletType type) {
  ++spawnRequests_;
  const auto start = measuring_ ? Clock::now() : Clock::time_point{};
  auto target = bullets_.end();
  if (allocationMode_ == BulletAllocationMode::LinearScan) {
    target = std::ranges::find(bullets_, false, &Bullet::active);
    if (measuring_)
      measurement_.inspectedSlots += static_cast<std::size_t>(target - bullets_.begin()) +
                                    (target != bullets_.end() ? 1 : 0);
  } else if (!freeIndices_.empty()) {
    std::ranges::pop_heap(freeIndices_, std::greater<>{});
    target = bullets_.begin() + freeIndices_.back();
    freeIndices_.pop_back();
  }
  if (target != bullets_.end()) {
    const auto style = GetBulletStyle(type);
    *target = Bullet{centerX,      centerY, velocityX, velocityY, style.width,
                     style.height, owner,   false,     true,      type};
  } else {
    ++droppedSpawnRequests_;
  }
  if (measuring_) {
    measurement_.spawnMilliseconds += ElapsedMilliseconds(start);
    ++measurement_.spawnCalls;
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
      Release(bullet);
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
