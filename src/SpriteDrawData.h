#pragma once

#include <array>

enum class SpriteTextureId {
  Checker = 0,
  Yellow = 1,
  White = 2,
  Player = 3,
};

enum class SpriteTimeSource {
  Game = 0,
  Real = 1,
};

enum class SpriteShape {
  Rectangle = 0,
  SoftCircle = 1,
  GlowCircle = 2,
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
};
