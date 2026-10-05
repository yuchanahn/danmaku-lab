#pragma once

enum class EnemyKind { Enemy1, Enemy2, MidBoss, FinalBoss };
enum class EnemyLifeState { Alive, Dying, Removed };

class Enemy {
public:
  Enemy() = default;
  Enemy(EnemyKind kind, float x, float y);
  void Reset() noexcept;
  void Update(double fixedDeltaSeconds);
  [[nodiscard]] bool ConsumeShot(double intervalSeconds = 0.0) noexcept;
  [[nodiscard]] bool TryTakeDamage(int damage = 1) noexcept;
  [[nodiscard]] int GetHp() const noexcept { return hp_; }
  [[nodiscard]] int GetMaxHp() const noexcept { return maxHp_; }
  [[nodiscard]] EnemyKind GetKind() const noexcept { return kind_; }
  [[nodiscard]] EnemyLifeState GetLifeState() const noexcept {
    return lifeState_;
  }
  [[nodiscard]] float GetHitRadius() const noexcept { return hitRadius_; }
  [[nodiscard]] float GetDissolveProgress() const noexcept;
  [[nodiscard]] double GetAnimationTimeSeconds() const noexcept {
    return aliveSeconds_;
  }
  [[nodiscard]] float GetX() const noexcept { return x_; }
  [[nodiscard]] float GetY() const noexcept { return y_; }
  [[nodiscard]] float GetWidth() const noexcept { return width_; }
  [[nodiscard]] float GetHeight() const noexcept { return height_; }

private:
  static float ComputeHorizontalOffset(double elapsedSeconds, float amplitude,
                                       double periodSeconds);
  static constexpr double kDissolveDurationSeconds = 0.8;
  EnemyKind kind_ = EnemyKind::MidBoss;
  EnemyLifeState lifeState_ = EnemyLifeState::Alive;
  int maxHp_ = 120;
  int hp_ = 120;
  float spawnX_ = 360.0f;
  float spawnY_ = 140.0f;
  float x_ = 360.0f;
  float y_ = 140.0f;
  float width_ = 160.0f;
  float height_ = 160.0f;
  float hitRadius_ = 24.0f;
  float movementSpeed_ = 0.0f;
  float horizontalAmplitude_ = 0.0f;
  double horizontalPeriodSeconds_ = 3.0;
  double aliveSeconds_ = 0.0;
  double deathSeconds_ = 0.0;
  double shotIntervalSeconds_ = 1.5;
  double shotCooldownSeconds_ = 1.0;
};
