#include "BulletSpriteInstanceData.h"
#include "GameScene.h"
#include "GameSpriteRenderer.h"
#include <cmath>
#include <numbers>
#include <algorithm>
#include <ranges>

void GameScene::Render(const GameSpriteRenderer &renderer) const {
  background_.Render(renderer, gameTimeSeconds_);
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
  for (const auto &minion : minions_) {
    if (minion.GetLifeState() == EnemyLifeState::Removed)
      continue;
    SpriteDrawData sprite{
        {minion.GetX(), minion.GetY()},
        {minion.GetWidth(), minion.GetHeight()},
        minion.GetKind() == EnemyKind::Enemy1 ? SpriteTextureId::Enemy1
                                              : SpriteTextureId::Enemy2,
        {1.0f, 1.0f, 1.0f, 1.0f},
    };
    sprite.dissolveProgress = minion.GetLifeState() == EnemyLifeState::Dying
                                  ? minion.GetDissolveProgress()
                                  : visuals_.dissolvePreviewProgress;
    renderer.DrawGameSprite(sprite, SpriteAnimationData{8, 1, 0, 8, 12.0, true},
                            minion.GetAnimationTimeSeconds());
  }
  if (!bossPresent_)
    return;
  SpriteDrawData enemySprite{
      {enemy_.GetX(), enemy_.GetY()},
      {enemy_.GetWidth(), enemy_.GetHeight()},
      enemy_.GetKind() == EnemyKind::FinalBoss ? SpriteTextureId::FinalBoss
                                               : SpriteTextureId::MidBoss,
      {1.0f, 1.0f, 1.0f, 1.0f},
      SpriteTimeSource::Game,
      {0.0f, 0.0f, 1.0f, 1.0f},
      SpriteShape::Rectangle,
      SpriteBlendMode::Alpha,
  };
  enemySprite.dissolveProgress =
      battleOutcome_ == BattleOutcome::None
          ? (enemy_.GetHp() == 0 ? enemy_.GetDissolveProgress()
                                 : visuals_.dissolvePreviewProgress)
      : enemy_.GetHp() == 0 ? GetDeathDissolveProgress()
                            : 0.0f;
  if (enemy_.GetHp() > 0 || IsEnding() ||
      stage_.GetPhase() == StagePhase::Transitioning) {
    const SpriteAnimationData animation{
        .columns = 8,
        .rows = 1,
        .firstFrame = 0,
        .frameCount = 8,
        .framesPerSecond = 12.0,
        .loop = true,
    };
    renderer.DrawGameSprite(enemySprite, animation,
                            enemy_.GetAnimationTimeSeconds());
  }
}

void GameScene::RenderBullets(const GameSpriteRenderer &renderer) const {
  if (visuals_.enhancedBullets && !visuals_.instancedBullets) {
    // 최적화 전 경로: 전체 후광 다음 전체 몸체, 탄환마다 상수 갱신/Draw.
    std::ranges::for_each(std::views::iota(0, 2), [&](int layer) {
      const bool isGlow = layer == 0;
      auto activeBullets = bulletSystem_.GetBullets() |
          std::views::filter([](const auto &bullet) { return bullet.active; });
      std::ranges::for_each(activeBullets, [&](const auto &bullet) {
        SpriteDrawData sprite{
            {bullet.x, bullet.y},
            {bullet.width, bullet.height},
            SpriteTextureId::White,
            bullet.owner == BulletOwner::Player
                ? std::array<float, 4>{0.15f, 0.75f, 1.0f, 1.0f}
                : std::array<float, 4>{1.0f, 0.18f, 0.12f, 1.0f},
        };
        if (bullet.type == BulletType::Thin &&
            (bullet.velocityX != 0.0f || bullet.velocityY != 0.0f))
          sprite.rotationRadians = std::atan2(bullet.velocityY, bullet.velocityX);
        sprite.shape = isGlow ? SpriteShape::GlowCircle : SpriteShape::BulletBody;
        sprite.blendMode = isGlow ? visuals_.bulletBlendMode : SpriteBlendMode::Alpha;
        if (isGlow) {
          sprite.size[0] += 16.0f;
          sprite.size[1] += 16.0f;
          sprite.tint[3] = 0.25f;
        }
        renderer.DrawGameSprite(sprite);
      });
    });
    return;
  }
  if (visuals_.enhancedBullets) {
    // clear는 용량을 유지하므로 프레임 간 저장 공간을 재사용한다.
    bulletGlowInstances_.clear();
    bulletBodyInstances_.clear();
    bulletGlowInstances_.reserve(bulletSystem_.GetBullets().size());
    bulletBodyInstances_.reserve(bulletSystem_.GetBullets().size());

    auto activeBullets = bulletSystem_.GetBullets() |
        std::views::filter([](const auto &bullet) { return bullet.active; });
    std::ranges::for_each(activeBullets, [&](const auto &bullet) {
      SpriteDrawData body{
          {bullet.x, bullet.y},
          {bullet.width, bullet.height},
          SpriteTextureId::White,
          bullet.owner == BulletOwner::Player
              ? std::array<float, 4>{0.15f, 0.75f, 1.0f, 1.0f}
              : std::array<float, 4>{1.0f, 0.18f, 0.12f, 1.0f},
      };
      if (bullet.type == BulletType::Thin &&
          (bullet.velocityX != 0.0f || bullet.velocityY != 0.0f)) {
        body.rotationRadians = std::atan2(bullet.velocityY, bullet.velocityX);
      }
      body.shape = SpriteShape::BulletBody;
      body.blendMode = SpriteBlendMode::Alpha;
      auto glow = body;
      glow.shape = SpriteShape::GlowCircle;
      glow.blendMode = visuals_.bulletBlendMode;
      constexpr float kGlowPadding = 16.0f;
      glow.size[0] += kGlowPadding;
      glow.size[1] += kGlowPadding;
      glow.tint[3] = 0.25f;

      bulletBodyInstances_.push_back(renderer.MakeScreenBulletInstance(body));
      bulletGlowInstances_.push_back(renderer.MakeScreenBulletInstance(glow));
    });

    // 모든 후광을 먼저 그리고 몸체를 올려 탄의 경계를 유지한다.
    renderer.DrawBulletInstances(bulletGlowInstances_, visuals_.bulletBlendMode);
    renderer.DrawBulletInstances(bulletBodyInstances_, SpriteBlendMode::Alpha);
    return;
  }
  auto activeBullets = bulletSystem_.GetBullets() |
      std::views::filter([](const auto &bullet) { return bullet.active; });
  std::ranges::for_each(activeBullets, [&](const auto &bullet) {
    const float rotation =
        bullet.type == BulletType::Thin &&
                (bullet.velocityX != 0.0f || bullet.velocityY != 0.0f)
            ? std::atan2(bullet.velocityY, bullet.velocityX)
            : 0.0f;

    const std::array<float, 4> bulletTint =
        bullet.owner == BulletOwner::Player
            ? std::array<float, 4>{0.15f, 0.75f, 1.0f, 1.0f}
            : std::array<float, 4>{1.0f, 0.18f, 0.12f, 1.0f};

    if (visuals_.bulletShape == SpriteShape::GlowCircle) {
      auto glowTint = bulletTint;
      std::ranges::for_each(glowTint | std::views::take(3),
                            [](float &channel) { channel *= 1.8f; });
      glowTint[3] = 0.65f;
      const SpriteDrawData glowSprite{
          {bullet.x, bullet.y},
          {bullet.width * 3.0f, bullet.height * 3.0f},
          SpriteTextureId::White,
          glowTint,
          SpriteTimeSource::Game,
          {0.0f, 0.0f, 1.0f, 1.0f},
          SpriteShape::GlowCircle,
          visuals_.bulletBlendMode,
          rotation,
      };
      renderer.DrawGameSprite(glowSprite);

      const SpriteDrawData coreSprite{
          {bullet.x, bullet.y},
          {bullet.width, bullet.height},
          SpriteTextureId::White,
          bulletTint,
          SpriteTimeSource::Game,
          {0.0f, 0.0f, 1.0f, 1.0f},
          SpriteShape::SoftCircle,
          SpriteBlendMode::Alpha,
          rotation,
      };
      renderer.DrawGameSprite(coreSprite);
      return;
    }

    const SpriteDrawData bulletSprite{
        {bullet.x, bullet.y},
        {bullet.width, bullet.height},
        SpriteTextureId::White,
        bulletTint,
        SpriteTimeSource::Game,
        {0.0f, 0.0f, 1.0f, 1.0f},
        visuals_.bulletShape,
        visuals_.bulletBlendMode,
        rotation,
    };
    renderer.DrawGameSprite(bulletSprite);
  });
}
