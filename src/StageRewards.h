#pragma once
#include "Player.h"
#include <array>
#include <random>

class GameSpriteRenderer;

struct UpgradePickup {
  PlayerUpgrade kind = PlayerUpgrade::FireRate;
  float x = 0.0f;
  float y = 0.0f;
  bool active = false;
};

class StageRewards {
public:
  void Reset() noexcept { pickups_ = {}; elapsedSeconds_ = 0.0; }
  void DropPair(const Player &player, float width, float height);
  void Update(Player &player, double deltaSeconds);
  void Render(const GameSpriteRenderer &renderer, double timeSeconds) const;
  [[nodiscard]] bool Empty() const noexcept;
  [[nodiscard]] const auto &GetPickups() const noexcept { return pickups_; }
private:
  std::array<UpgradePickup, 2> pickups_{};
  float width_ = 720.0f;
  float height_ = 960.0f;
  double elapsedSeconds_ = 0.0;
  std::mt19937 random_{std::random_device{}()};
};
