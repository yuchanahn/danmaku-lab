#include "Collision.h"

bool Intersects(const CircleHitbox &a, const CircleHitbox &b) noexcept {
  float dx = a.centerX - b.centerX;
  float dy = a.centerY - b.centerY;
  float r = a.radius + b.radius;
  return dx * dx + dy * dy <= r * r;
}
