#pragma once

#include "BulletType.h"

#include <cstddef>
#include <vector>

enum class BulletOwner {
  Player,
  Enemy,
};

struct Bullet {
  float x = 0.0f;
  float y = 0.0f;
  float velocityX = 0.0f;
  float velocityY = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
  BulletOwner owner = BulletOwner::Player;
  bool grazed = false;
  bool active = false;
  BulletType type = BulletType::Normal;
};

class BulletSystem {
public:
  BulletSystem();

  void Reset() noexcept;
  void Clear() noexcept;
  [[nodiscard]] std::size_t GetCapacity() const noexcept {
    return bullets_.size();
  }
  [[nodiscard]] std::size_t GetSpawnRequests() const noexcept {
    return spawnRequests_;
  }
  [[nodiscard]] std::size_t GetDroppedSpawnRequests() const noexcept {
    return droppedSpawnRequests_;
  }
  void Spawn(float centerX, float centerY);
  void Spawn(float centerX, float centerY, float velocityX, float velocityY,
             BulletOwner owner, BulletType type = BulletType::Normal);
  void Update(double fixedDeltaSeconds);
  void RemoveOutside(float screenWidth, float screenHeight);

  [[nodiscard]] std::size_t GetActiveCount() const noexcept;

  [[nodiscard]] std::vector<Bullet> &GetBullets() noexcept { return bullets_; }

  [[nodiscard]] const std::vector<Bullet> &GetBullets() const noexcept {
    return bullets_;
  }

private:
  static constexpr std::size_t kPoolCapacity = 8192;
  std::vector<Bullet> bullets_;
  std::size_t spawnRequests_ = 0;
  std::size_t droppedSpawnRequests_ = 0;
};
