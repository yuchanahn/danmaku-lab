#pragma once

#include <array>
#include <windows.h>

class Input {
public:
  enum class KeyState {
    Up,
    Pressed,
    Held,
    Released,
  };

  void BeginFrame() noexcept;
  void SetKeyDown(UINT virtualKey, bool isDown) noexcept;

  [[nodiscard]] KeyState GetKeyState(UINT virtualKey) const noexcept;
  [[nodiscard]] bool IsDown(UINT virtualKey) const noexcept;

private:
  static constexpr std::size_t kKeyCount = 256;

  std::array<bool, kKeyCount> current_{};
  std::array<bool, kKeyCount> previous_{};
};
