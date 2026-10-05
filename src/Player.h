#pragma once

class Player {
public:
  void Reset() noexcept;
  void Update(float directionX, float directionY, double fixedDeltaSeconds);
  [[nodiscard]] bool UpdateShooting(bool shootHeld, double fixedDeltaSeconds);
  void ClampToBounds(float screenWidth, float screenHeight);
  [[nodiscard]] bool TryTakeDamage() noexcept;

  [[nodiscard]] int GetHp() const noexcept { return hp_; }
  [[nodiscard]] int GetMaxHp() const noexcept { return kMaxHp; }
  [[nodiscard]] double GetInvulnerabilitySeconds() const noexcept {
    return invulnerabilitySeconds_;
  }
  [[nodiscard]] bool IsInvulnerable() const noexcept {
    return invulnerabilitySeconds_ > 0.0;
  }

  [[nodiscard]] float GetX() const noexcept { return x_; }
  [[nodiscard]] float GetY() const noexcept { return y_; }
  [[nodiscard]] float GetWidth() const noexcept { return width_; }
  [[nodiscard]] float GetHeight() const noexcept { return height_; }

private:
  static constexpr int kMaxHp = 3;
  static constexpr double kInvulnerabilityDurationSeconds = 1.0;
  int hp_ = kMaxHp;
  double invulnerabilitySeconds_ = 0.0;
  float x_ = 360.0f;
  float y_ = 800.0f;
  float width_ = 48.0f;
  float height_ = 48.0f;
  float speedPixelsPerSecond_ = 240.0f;
  static constexpr double kShotIntervalSeconds = 0.15;
  double shotCooldownSeconds_ = 0.0;
};
