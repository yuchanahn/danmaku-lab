#pragma once

#include "SpriteDrawData.h"
#include <array>

// 인스턴스 입력 버퍼에 기록할 탄환 한 개의 표시 데이터. 좌표는 화면 픽셀이다.
// 텍스처/샘플러/블렌드는 같은 묶음에서 공유하며 GPU 객체는 소유하지 않는다.
struct BulletSpriteInstanceData {
  std::array<float, 2> center{};
  std::array<float, 2> size{};
  std::array<float, 4> tint{1.0f, 1.0f, 1.0f, 1.0f};
  std::array<float, 2> rotationXAxis{1.0f, 0.0f};
  std::array<float, 2> rotationYAxis{0.0f, 1.0f};
  float shape = 0.0f;
};

[[nodiscard]] BulletSpriteInstanceData
MakeBulletSpriteInstanceData(const SpriteDrawData &screenSprite);
