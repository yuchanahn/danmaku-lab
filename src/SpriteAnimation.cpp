#include "SpriteDrawData.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

std::array<float, 4>
CalculateSpriteAnimationUv(const SpriteAnimationData &animation,
                           double elapsedSeconds) {
  if (animation.columns == 0 || animation.rows == 0 ||
      animation.columns > std::numeric_limits<std::size_t>::max() /
                              animation.rows ||
      animation.frameCount == 0 ||
      !std::isfinite(animation.framesPerSecond) ||
      animation.framesPerSecond <= 0.0 || !std::isfinite(elapsedSeconds)) {
    throw std::invalid_argument("Invalid sprite animation dimensions or timing.");
  }
  const auto cellCount = animation.columns * animation.rows;
  if (animation.firstFrame >= cellCount ||
      animation.frameCount > cellCount - animation.firstFrame) {
    throw std::invalid_argument("Sprite animation frames exceed the sheet.");
  }

  const double elapsedFrames =
      std::floor(std::max(0.0, elapsedSeconds) * animation.framesPerSecond);
  if (!std::isfinite(elapsedFrames)) {
    throw std::invalid_argument("Sprite animation time is too large.");
  }
  const auto offset = static_cast<std::size_t>(
      animation.loop
          ? std::fmod(elapsedFrames, static_cast<double>(animation.frameCount))
          : std::min(elapsedFrames,
                     static_cast<double>(animation.frameCount - 1)));
  const auto frame = animation.firstFrame + offset;
  const auto column = frame % animation.columns;
  const auto row = frame / animation.columns;
  const float cellWidth = 1.0f / static_cast<float>(animation.columns);
  const float cellHeight = 1.0f / static_cast<float>(animation.rows);
  return {static_cast<float>(column) * cellWidth,
          static_cast<float>(row) * cellHeight,
          static_cast<float>(column + 1) * cellWidth,
          static_cast<float>(row + 1) * cellHeight};
}
