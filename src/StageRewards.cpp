#include "StageRewards.h"
#include "GameSpriteRenderer.h"
#include <algorithm>
#include <cmath>
#include <ranges>

void StageRewards::DropPair(const Player &player, float width, float height) {
  width_ = width;
  height_ = height;
  elapsedSeconds_ = 0.0;
  std::uniform_int_distribution<int> roll(0, 2);
  for (std::size_t i = 0; i < pickups_.size(); ++i) {
    pickups_[i] = {static_cast<PlayerUpgrade>(roll(random_)),
                  std::clamp(player.GetX() + (i == 0 ? -80.0f : 80.0f), 28.0f, width - 28.0f),
                  std::clamp(player.GetY() - 160.0f, 28.0f, height - 28.0f), true};
  }
}

void StageRewards::Update(Player &player, double deltaSeconds) {
  elapsedSeconds_ += deltaSeconds;
  for (std::size_t i = 0; i < pickups_.size(); ++i) {
    auto &item = pickups_[i];
    if (!item.active) continue;
    item.x += std::cos(static_cast<float>(elapsedSeconds_) * 2.0f + static_cast<float>(i) * 3.1415927f)
              * 18.0f * static_cast<float>(deltaSeconds);
    item.y += 60.0f * static_cast<float>(deltaSeconds);
    // Expire before testing pickup: an off-screen item cannot award an upgrade.
    if (item.x < 0.0f || item.x > width_ || item.y < 0.0f || item.y > height_) {
      item.active = false;
      continue;
    }
    const float dx = player.GetX() - item.x;
    const float dy = player.GetY() - item.y;
    const float distance = std::hypot(dx, dy);
    if (distance <= 32.0f) {
      player.ApplyUpgrade(item.kind);
      item.active = false;
    }
  }
}

bool StageRewards::Empty() const noexcept {
  return std::ranges::none_of(pickups_, &UpgradePickup::active);
}

void StageRewards::Render(const GameSpriteRenderer &renderer, double timeSeconds) const {
  for (const auto &item : pickups_) {
    if (!item.active) continue;
    const std::array<float, 4> color = item.kind == PlayerUpgrade::FireRate
        ? std::array{0.1f, 0.95f, 1.0f, 1.0f}
        : item.kind == PlayerUpgrade::Damage ? std::array{1.0f, 0.35f, 0.1f, 1.0f}
                                            : std::array{0.8f, 0.3f, 1.0f, 1.0f};
    const float pulse = 86.0f + 8.0f * std::sin(static_cast<float>(timeSeconds) * 6.0f);
    SpriteDrawData glow{{item.x, item.y}, {pulse, pulse}, SpriteTextureId::White, color};
    glow.shape = SpriteShape::GlowCircle;
    glow.blendMode = SpriteBlendMode::Additive;
    renderer.DrawGameSprite(glow);
    renderer.DrawGameSprite({{item.x, item.y}, {50.0f, 50.0f}, SpriteTextureId::White});
    renderer.DrawGameSprite({{item.x, item.y}, {42.0f, 42.0f}, SpriteTextureId::Checker, color});
  }
}
