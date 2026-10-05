#pragma once
#include "BulletType.h"

class BulletSystem;

namespace DanmakuPattern {

void SpawnRing(BulletSystem &bulletSystem, float centerX, float centerY,
               int bulletCount, float speedPixelsPerSecond,
               float startAngleRadians, BulletType type = BulletType::Normal);

void SpawnFan(BulletSystem &bulletSystem, float centerX, float centerY,
              int bulletCount, float speedPixelsPerSecond,
              float centerAngleRadians, float spreadAngleRadians,
              BulletType type = BulletType::Normal);

} // namespace DanmakuPattern
