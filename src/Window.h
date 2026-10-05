#pragma once

#include <optional>
#include <string_view>
#include <vector>
#include <windows.h>

class Window {
public:
  struct ClientSize {
    UINT width;
    UINT height;
  };

  struct KeyEvent {
    UINT virtualKey;
    bool isDown;
  };

  Window(HINSTANCE instance, int showCommand);
  ~Window();

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(Window &&) = delete;

  [[nodiscard]] HWND GetHandle() const noexcept;
  [[nodiscard]] ClientSize GetClientSize() const noexcept;
  [[nodiscard]] bool IsMinimized() const noexcept { return minimized_; }
  [[nodiscard]] std::optional<ClientSize> ConsumePendingResize() noexcept;
  [[nodiscard]] std::vector<KeyEvent> ConsumeKeyEvents();
  [[nodiscard]] bool ConsumeFocusLost() noexcept;
  [[nodiscard]] std::optional<int> ProcessMessages() const;
  void SetTitle(std::wstring_view title) const;

private:
  static constexpr wchar_t kWindowClassName[] = L"DanmakuShooterWindowClass";
  static constexpr wchar_t kWindowTitle[] = L"Danmaku Lab - DirectX 11";
  static constexpr int kInitialWidth = 1920;
  static constexpr int kInitialHeight = 1080;
  static constexpr int kMinimumClientWidth = 960;
  static constexpr int kMinimumClientHeight = 540;

  static LRESULT CALLBACK SetupWindowProc(HWND window, UINT message,
                                          WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK ForwardWindowProc(HWND window, UINT message,
                                            WPARAM wParam, LPARAM lParam);
  LRESULT HandleMessage(HWND window, UINT message, WPARAM wParam,
                        LPARAM lParam);

  HINSTANCE instance_{};
  HWND handle_{};
  ClientSize clientSize_{kInitialWidth, kInitialHeight};
  bool minimized_ = false;
  bool focusLost_ = false;
  std::optional<ClientSize> pendingResize_;
  std::vector<KeyEvent> pendingKeyEvents_;
};
