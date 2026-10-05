#include "Input.h"
#include <array>
#include <cstdint>

void Input::BeginFrame() noexcept { previous_ = current_; }

void Input::Reset() noexcept {
  current_.fill(false);
  previous_.fill(false);
}

void Input::SetKeyDown(UINT virtualKey, bool isDown) noexcept {
  if (virtualKey < kKeyCount) {
    current_[virtualKey] = isDown;
  }
}

Input::KeyState Input::GetKeyState(UINT virtualKey) const noexcept {
  if (virtualKey >= kKeyCount)
    return KeyState::Up;

  constexpr std::array<KeyState, 4> states = {
      KeyState::Up,       // 00
      KeyState::Pressed,  // 01
      KeyState::Released, // 10
      KeyState::Held      // 11
  };

  const std::uint8_t index =
      (static_cast<std::uint8_t>(previous_[virtualKey]) << 1) |
      static_cast<std::uint8_t>(current_[virtualKey]);

  return states[index];
}

bool Input::IsDown(UINT virtualKey) const noexcept {
  return virtualKey < kKeyCount && current_[virtualKey];
}
