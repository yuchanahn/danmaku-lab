#pragma once

struct CircleHitbox {
  float centerX = 0.0f;
  float centerY = 0.0f;
  float radius = 0.0f;
};

[[nodiscard]] bool Intersects(const CircleHitbox &a,
                              const CircleHitbox &b) noexcept;
