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
  [[nodiscard]] std::optional<ClientSize> ConsumePendingResize() noexcept;
  [[nodiscard]] std::vector<KeyEvent> ConsumeKeyEvents();
  [[nodiscard]] std::optional<int> ProcessMessages() const;
  void SetTitle(std::wstring_view title) const;

private:
  static constexpr wchar_t kWindowClassName[] = L"DanmakuShooterWindowClass";
  static constexpr wchar_t kWindowTitle[] = L"Danmaku Shooter - DirectX 11";
  static constexpr int kInitialWidth = 1280;
  static constexpr int kInitialHeight = 720;

  static LRESULT CALLBACK SetupWindowProc(HWND window, UINT message,
                                          WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK ForwardWindowProc(HWND window, UINT message,
                                            WPARAM wParam, LPARAM lParam);
  LRESULT HandleMessage(HWND window, UINT message, WPARAM wParam,
                        LPARAM lParam);

  HINSTANCE instance_{};
  HWND handle_{};
  ClientSize clientSize_{kInitialWidth, kInitialHeight};
  std::optional<ClientSize> pendingResize_;
  std::vector<KeyEvent> pendingKeyEvents_;
};
