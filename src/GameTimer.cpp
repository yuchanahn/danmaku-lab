#include "GameTimer.h"

#include <algorithm>
#include <stdexcept>

GameTimer::GameTimer() {
  if (QueryPerformanceFrequency(&frequency_) == 0) {
    throw std::runtime_error("QueryPerformanceFrequency failed.");
  }

  Reset();
}

void GameTimer::Reset() {
  if (QueryPerformanceCounter(&previousCounter_) == 0) {
    throw std::runtime_error("QueryPerformanceCounter failed.");
  }
  elapsedSeconds_ = 0.0;
}

double GameTimer::Tick() {
  LARGE_INTEGER currentCounter{};
  if (QueryPerformanceCounter(&currentCounter) == 0) {
    throw std::runtime_error("QueryPerformanceCounter failed.");
  }

  const auto elapsedCounts = currentCounter.QuadPart - previousCounter_.QuadPart;
  previousCounter_ = currentCounter;

  const double deltaSeconds =
      static_cast<double>(elapsedCounts) / static_cast<double>(frequency_.QuadPart);

  const double realDeltaSeconds = std::max(deltaSeconds, 0.0);
  elapsedSeconds_ += realDeltaSeconds;
  return realDeltaSeconds;
}
