#pragma once

#include <array>
#include <cstddef>

// 행 우선으로 배열한 동일 크기 셀의 재생 규격. 재생 상태/GPU 리소스는 없다.
struct SpriteAnimationData {
  std::size_t columns = 1;
  std::size_t rows = 1;
  std::size_t firstFrame = 0;
  std::size_t frameCount = 1;
  double framesPerSecond = 12.0;
  bool loop = true;
};

[[nodiscard]] std::array<float, 4>
CalculateSpriteAnimationUv(const SpriteAnimationData &animation,
                           double elapsedSeconds);

enum class SpriteTextureId {
  Checker = 0,
  Yellow = 1,
  White = 2,
  Player = 3,
  Enemy = 4,
  ForestBackground = 5,
  FogBackground = 6,
  Enemy1,
  Enemy2,
  MidBoss,
  FinalBoss,
  Count,
};

enum class SpriteTimeSource {
  Game = 0,
  Real = 1,
};

enum class SpriteShape {
  Rectangle = 0,
  SoftCircle = 1,
  GlowCircle = 2,
  BulletBody = 3,
};

enum class SpriteBlendMode {
  Alpha = 0,
  Additive = 1,
};

enum class SpriteSamplerMode {
  Point = 0,
  Linear = 1,
};

enum class SpriteAddressMode {
  Clamp = 0,
  Wrap = 1,
};

// Sprite 하나를 표시하는 데 필요한 CPU 데이터. GPU 리소스를 소유하지 않는다.
struct SpriteDrawData {
  std::array<float, 2> center;
  std::array<float, 2> size;
  SpriteTextureId texture = SpriteTextureId::Checker;
  std::array<float, 4> tint{1.0f, 1.0f, 1.0f, 1.0f};
  SpriteTimeSource timeSource = SpriteTimeSource::Game;
  // (startU, startV, endU, endV)
  std::array<float, 4> uvRect{0.0f, 0.0f, 1.0f, 1.0f};
  SpriteShape shape = SpriteShape::Rectangle;
  SpriteBlendMode blendMode = SpriteBlendMode::Alpha;
  float rotationRadians = 0.0f;
  SpriteSamplerMode samplerMode = SpriteSamplerMode::Point;
  SpriteAddressMode addressMode = SpriteAddressMode::Clamp;
  float dissolveProgress = 0.0f;
  float dissolveNoiseScale = 14.0f;
  float dissolveEdgeWidth = 0.08f;
  float dissolveEdgeStrength = 2.0f;
  std::array<float, 4> dissolveEdgeColor{0.15f, 0.8f, 1.0f, 1.0f};
};
