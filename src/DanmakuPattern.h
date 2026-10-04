#pragma once

class BulletSystem;

namespace DanmakuPattern {

void SpawnRing(BulletSystem &bulletSystem, float centerX, float centerY,
               int bulletCount, float speedPixelsPerSecond,
               float startAngleRadians);

void SpawnFan(BulletSystem &bulletSystem, float centerX, float centerY,
              int bulletCount, float speedPixelsPerSecond,
              float centerAngleRadians, float spreadAngleRadians);

} // namespace DanmakuPattern
