#pragma once

#include "BulletType.h"

#include <cstddef>
#include <cstdint>
#include <vector>

enum class BulletAllocationMode { LinearScan, FreeIndexHeap };

struct BulletPoolMeasurement {
  std::uint64_t spawnCalls = 0;
  std::uint64_t released = 0;
  std::uint64_t inspectedSlots = 0;
  double spawnMilliseconds = 0.0;
  double releaseMilliseconds = 0.0;
};

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
  int damage = 1;
};

class BulletSystem {
public:
  BulletSystem();
  BulletSystem(const BulletSystem &) = delete;
  BulletSystem &operator=(const BulletSystem &) = delete;

  void Reset() noexcept;
  void Clear() noexcept;
  // 같은 슬롯의 중복 반납을 막고 빈 인덱스 목록도 함께 갱신한다.
  void Release(std::size_t index) noexcept;
  void Release(Bullet &bullet) noexcept;
  void SetAllocationMode(BulletAllocationMode mode);
  void BeginPoolMeasurement() noexcept { measurement_ = {}; measuring_ = true; }
  void EndPoolMeasurement() noexcept { measuring_ = false; }
  [[nodiscard]] BulletPoolMeasurement GetPoolMeasurement() const noexcept {
    return measurement_;
  }
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
             BulletOwner owner, BulletType type = BulletType::Normal, int damage = 1);
  void Update(double fixedDeltaSeconds);
  void RemoveOutside(float screenWidth, float screenHeight);

  [[nodiscard]] std::size_t GetActiveCount() const noexcept;

  [[nodiscard]] const std::vector<Bullet> &GetBullets() const noexcept {
    return bullets_;
  }

private:
  friend class GameScene;
  // 위치/Graze 변경은 허용하지만 수명 종료는 반드시 Release를 거친다.
  [[nodiscard]] std::vector<Bullet> &GetMutableBullets() noexcept { return bullets_; }
  void RebuildFreeIndices() noexcept;
  static constexpr std::size_t kPoolCapacity = 8192;
  std::vector<Bullet> bullets_;
  std::vector<std::size_t> freeIndices_;
  BulletAllocationMode allocationMode_ = BulletAllocationMode::FreeIndexHeap;
  bool measuring_ = false;
  BulletPoolMeasurement measurement_;
  std::size_t spawnRequests_ = 0;
  std::size_t droppedSpawnRequests_ = 0;
};
