#include "GameSpriteRenderer.h"
#include "BulletSpriteInstanceData.h"
#include "Graphics.h"
#include "SpriteDrawData.h"
#include <span>

void GameSpriteRenderer::DrawGameSprite(const SpriteDrawData &sprite) const {
  graphics_.DrawSprite(layout_.ToScreen(sprite));
}

BulletSpriteInstanceData GameSpriteRenderer::MakeScreenBulletInstance(
    const SpriteDrawData &gameSprite) const {
  return MakeBulletSpriteInstanceData(layout_.ToScreen(gameSprite));
}

void GameSpriteRenderer::DrawBulletInstances(
    std::span<const BulletSpriteInstanceData> screenInstances,
    SpriteBlendMode blendMode) const {
  graphics_.DrawBulletInstances(screenInstances, blendMode);
}

void GameSpriteRenderer::DrawGameSprite(
    const SpriteDrawData &sprite, const SpriteAnimationData &animation) const {
  DrawGameSprite(sprite, animation,
                 sprite.timeSource == SpriteTimeSource::Game ? gameTime_
                                                             : realTime_);
}

void GameSpriteRenderer::DrawGameSprite(const SpriteDrawData &sprite,
                                        const SpriteAnimationData &animation,
                                        double time) const {
  const auto uv = CalculateSpriteAnimationUv(animation, time);
  const float width = uv[2] - uv[0];
  const float height = uv[3] - uv[1];
  auto frame = sprite;
  frame.uvRect = {
      uv[0] + sprite.uvRect[0] * width, uv[1] + sprite.uvRect[1] * height,
      uv[0] + sprite.uvRect[2] * width, uv[1] + sprite.uvRect[3] * height};
  DrawGameSprite(frame);
}

void GameSpriteRenderer::DrawSurround(float width, float height) const {
  const auto panel = [this](float x, float y, float w, float h) {
    if (w <= 0.0f || h <= 0.0f)
      return;
    const SpriteDrawData sprite{{x + w * 0.5f, y + h * 0.5f},
                                {w, h},
                                SpriteTextureId::White,
                                {0.015f, 0.02f, 0.035f, 1.0f}};
    graphics_.DrawSprite(sprite);
  };
  panel(0.0f, 0.0f, layout_.left, height);
  panel(layout_.Right(), 0.0f, width - layout_.Right(), height);
  panel(layout_.left, 0.0f, PlayfieldLayout::kWidth * layout_.scale,
        layout_.top);
  panel(layout_.left, layout_.Bottom(), PlayfieldLayout::kWidth * layout_.scale,
        height - layout_.Bottom());
}
