#pragma once

enum class BulletType { Normal, Thin };

struct BulletStyle {
  float width;
  float height;
  float hitRadius;
};

// 논리 전투 좌표 기준. 표시 크기와 충돌 반경은 별도 값이다.
[[nodiscard]] constexpr BulletStyle GetBulletStyle(BulletType type) noexcept {
  switch (type) {
  case BulletType::Thin:
    return {28.0f, 8.0f, 3.0f};
  case BulletType::Normal:
  default:
    return {20.0f, 20.0f, 8.0f};
  }
}
