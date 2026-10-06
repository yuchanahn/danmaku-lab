#include "BulletSpriteInstanceData.h"

#include <cmath>

BulletSpriteInstanceData
MakeBulletSpriteInstanceData(const SpriteDrawData &screenSprite) {
  BulletSpriteInstanceData instance{};
  instance.center = screenSprite.center;
  instance.size = screenSprite.size;
  instance.tint = screenSprite.tint;
  instance.shape = static_cast<float>(screenSprite.shape);

  const float cosine = std::cos(screenSprite.rotationRadians);
  const float sine = std::sin(screenSprite.rotationRadians);
  instance.rotationXAxis = {cosine, sine};
  instance.rotationYAxis = {-sine, cosine};

  return instance;
}
