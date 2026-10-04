#pragma once

#include <windows.h>

class GameTimer {
public:
  GameTimer();

  void Reset();
  [[nodiscard]] double Tick();
  [[nodiscard]] double GetElapsedSeconds() const noexcept { return elapsedSeconds_; }

private:
  LARGE_INTEGER frequency_{};
  LARGE_INTEGER previousCounter_{};
  double elapsedSeconds_ = 0.0;
};
