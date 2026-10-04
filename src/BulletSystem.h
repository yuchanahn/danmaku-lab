#pragma once

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
};

class BulletSystem {
public:
  BulletSystem();

  void Reset() noexcept;
  void Spawn(float centerX, float centerY);
  void Spawn(float centerX, float centerY, float velocityX, float velocityY,
             BulletOwner owner);
  void Update(double fixedDeltaSeconds);
  void RemoveOutside(float screenWidth, float screenHeight);

  [[nodiscard]] std::size_t GetActiveCount() const noexcept;

  [[nodiscard]] std::vector<Bullet> &GetBullets() noexcept {
    return bullets_;
  }

  [[nodiscard]] const std::vector<Bullet> &GetBullets() const noexcept {
    return bullets_;
  }

private:
  static constexpr std::size_t kPoolCapacity = 256;
  std::vector<Bullet> bullets_;
};
