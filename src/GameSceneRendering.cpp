#include "GameScene.h"
#include "GameSpriteRenderer.h"
#include <cmath>
#include <numbers>

void GameScene::Render(const GameSpriteRenderer &renderer) const {
  RenderPlayer(renderer);
  RenderEnemy(renderer);
  RenderBullets(renderer);
}

void GameScene::RenderPlayer(const GameSpriteRenderer &renderer) const {

  float alpha = 1.f;

  if (player_.GetHp() > 0 && player_.IsInvulnerable()) {
    alpha = static_cast<float>(std::sin(player_.GetInvulnerabilitySeconds() *
                                        2.f * std::numbers::pi * 6.f)) > 0.f
                ? 1.f
                : 0.25f;
  }

  std::array<float, 4> UV = visuals_.playerUvRect;

  if (playerMotion_ == PlayerMotion::Right) {
    UV = {1.f, 0.f, 0.f, 1.f};
  }

  SpriteDrawData playerSprite{
      {player_.GetX(), player_.GetY()},
      {player_.GetWidth(), player_.GetHeight()},
      SpriteTextureId::Player,
      {1.0f, 1.0f, 1.0f, alpha},
      SpriteTimeSource::Game,
      UV,
      SpriteShape::Rectangle,
      SpriteBlendMode::Alpha,
      visuals_.playerRotationRadians,
      visuals_.playerSamplerMode,
      visuals_.playerAddressMode,
  };
  playerSprite.dissolveProgress =
      battleOutcome_ == BattleOutcome::None ? visuals_.dissolvePreviewProgress
      : player_.GetHp() == 0                ? GetDeathDissolveProgress()
                                            : 0.0f;
  if (player_.GetHp() > 0 || IsEnding()) {
    const SpriteAnimationData animation{
        .columns = 8,
        .rows = 2,
        .firstFrame = playerMotion_ == PlayerMotion::Idle ? 0u : 8u,
        .frameCount = 8,
        .framesPerSecond = 12.0,
        .loop = true,
    };
    renderer.DrawGameSprite(playerSprite, animation);
  }
}

void GameScene::RenderEnemy(const GameSpriteRenderer &renderer) const {
  SpriteDrawData enemySprite{
      {enemy_.GetX(), enemy_.GetY()}, {enemy_.GetWidth(), enemy_.GetHeight()},
      SpriteTextureId::Enemy,         {1.0f, 1.0f, 1.0f, 1.0f},
      SpriteTimeSource::Game,         {0.0f, 0.0f, 1.0f, 1.0f},
      SpriteShape::SoftCircle,        SpriteBlendMode::Alpha,
  };
  enemySprite.dissolveProgress =
      battleOutcome_ == BattleOutcome::None ? visuals_.dissolvePreviewProgress
      : enemy_.GetHp() == 0                 ? GetDeathDissolveProgress()
                                            : 0.0f;
  if (enemy_.GetHp() > 0 || IsEnding()) {
    const SpriteAnimationData animation{
        .columns = 4,
        .rows = 2,
        .firstFrame = 0,
        .frameCount = 4,
        .framesPerSecond = 12.0,
        .loop = true,
    };
    renderer.DrawGameSprite(enemySprite, animation);
  }
}

void GameScene::RenderBullets(const GameSpriteRenderer &renderer) const {
  for (const auto &bullet : bulletSystem_.GetBullets()) {
    if (!bullet.active) {
      continue;
    }

    const std::array<float, 4> bulletTint =
        bullet.owner == BulletOwner::Player
            ? std::array<float, 4>{0.15f, 0.75f, 1.0f, 1.0f}
            : std::array<float, 4>{1.0f, 0.18f, 0.12f, 1.0f};

    if (visuals_.bulletShape == SpriteShape::GlowCircle) {
      auto glowTint = bulletTint;
      for (std::size_t channel = 0; channel < 3; ++channel) {
        glowTint[channel] *= 1.8f;
      }
      glowTint[3] = 0.65f;
      const SpriteDrawData glowSprite{
          {bullet.x, bullet.y},    {bullet.width * 3.0f, bullet.height * 3.0f},
          SpriteTextureId::White,  glowTint,
          SpriteTimeSource::Game,  {0.0f, 0.0f, 1.0f, 1.0f},
          SpriteShape::GlowCircle, visuals_.bulletBlendMode,
      };
      renderer.DrawGameSprite(glowSprite);

      const SpriteDrawData coreSprite{
          {bullet.x, bullet.y},    {bullet.width, bullet.height},
          SpriteTextureId::White,  bulletTint,
          SpriteTimeSource::Game,  {0.0f, 0.0f, 1.0f, 1.0f},
          SpriteShape::SoftCircle, SpriteBlendMode::Alpha,
      };
      renderer.DrawGameSprite(coreSprite);
      continue;
    }

    const SpriteDrawData bulletSprite{
        {bullet.x, bullet.y},   {bullet.width, bullet.height},
        SpriteTextureId::White, bulletTint,
        SpriteTimeSource::Game, {0.0f, 0.0f, 1.0f, 1.0f},
        visuals_.bulletShape,   visuals_.bulletBlendMode,
    };
    renderer.DrawGameSprite(bulletSprite);
  }
}
