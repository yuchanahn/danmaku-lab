#pragma once

class Enemy {
public:
  void Reset() noexcept;
  [[nodiscard]] bool TryTakeDamage() noexcept;
  [[nodiscard]] int GetHp() const noexcept { return hp_; }
  [[nodiscard]] int GetMaxHp() const noexcept { return kMaxHp; }
  [[nodiscard]] float GetX() const noexcept { return x_; }
  [[nodiscard]] float GetY() const noexcept { return y_; }
  [[nodiscard]] float GetWidth() const noexcept { return width_; }
  [[nodiscard]] float GetHeight() const noexcept { return height_; }

private:
  static constexpr int kMaxHp = 120;
  int hp_ = kMaxHp;
  float x_ = 360.0f;
  float y_ = 140.0f;
  float width_ = 56.0f;
  float height_ = 56.0f;
};
