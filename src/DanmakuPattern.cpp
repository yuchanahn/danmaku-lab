#include "DanmakuPattern.h"

#include "BulletSystem.h"
#include <cmath>
#include <numbers>

namespace DanmakuPattern {

void SpawnRing(BulletSystem &bulletSystem, float centerX, float centerY,
               int bulletCount, float speedPixelsPerSecond,
               float startAngleRadians) {
  if (bulletCount <= 0 || speedPixelsPerSecond <= 0.0f) {
    return;
  }
  for (int i = 0; i < bulletCount; i++) {
    float r = static_cast<float>(std::numbers::pi * 2.0) *
              (static_cast<float>(i) + 1.f) / bulletCount;

    bulletSystem.Spawn(centerX, centerY,
                       std::cos(r + startAngleRadians) * speedPixelsPerSecond,
                       std::sin(r + startAngleRadians) * speedPixelsPerSecond,
                       BulletOwner::Enemy);
  }
}

void SpawnFan(BulletSystem &bulletSystem, float centerX, float centerY,
              int bulletCount, float speedPixelsPerSecond,
              float centerAngleRadians, float spreadAngleRadians) {
  if (bulletCount <= 0 || speedPixelsPerSecond <= 0.0f ||
      spreadAngleRadians < 0.0f) {
    return;
  }

  if (bulletCount == 1) {
    bulletSystem.Spawn(centerX, centerY,
                       std::cos(centerAngleRadians) * speedPixelsPerSecond,
                       std::sin(centerAngleRadians) * speedPixelsPerSecond,
                       BulletOwner::Enemy);
    return;
  }

  for (int i = 0; i < bulletCount; i++) {
    float t = static_cast<float>(i) / static_cast<float>(bulletCount - 1);
    float r = (centerAngleRadians - (spreadAngleRadians / 2)) +
              spreadAngleRadians * t;

    bulletSystem.Spawn(centerX, centerY, std::cos(r) * speedPixelsPerSecond,
                       std::sin(r) * speedPixelsPerSecond,
                       BulletOwner::Enemy);
  }
}

} // namespace DanmakuPattern
