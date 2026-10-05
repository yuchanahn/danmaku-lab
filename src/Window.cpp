#include "Window.h"

#include <string>
#include <stdexcept>

Window::Window(HINSTANCE instance, int showCommand) : instance_(instance) {
  WNDCLASSEXW windowClass{};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.style = CS_HREDRAW | CS_VREDRAW;
  windowClass.lpfnWndProc = SetupWindowProc;
  windowClass.hInstance = instance_;
  windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  windowClass.lpszClassName = kWindowClassName;

  if (RegisterClassExW(&windowClass) == 0) {
    throw std::runtime_error("Failed to register the Win32 window class.");
  }

  RECT windowRect{0, 0, kInitialWidth, kInitialHeight};
  if (AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE) == 0) {
    UnregisterClassW(kWindowClassName, instance_);
    throw std::runtime_error("Failed to calculate the Win32 window size.");
  }

  handle_ = CreateWindowExW(
      0, kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
      CW_USEDEFAULT, windowRect.right - windowRect.left,
      windowRect.bottom - windowRect.top, nullptr, nullptr, instance_, this);

  if (!handle_) {
    UnregisterClassW(kWindowClassName, instance_);
    throw std::runtime_error("Failed to create the Win32 window.");
  }

  RECT clientRect{};
  if (GetClientRect(handle_, &clientRect) != 0) {
    clientSize_.width = static_cast<UINT>(clientRect.right - clientRect.left);
    clientSize_.height = static_cast<UINT>(clientRect.bottom - clientRect.top);
  }

  ShowWindow(handle_, showCommand);
  UpdateWindow(handle_);
}

Window::~Window() {
  if (handle_ && IsWindow(handle_)) {
    DestroyWindow(handle_);
  }

  if (instance_) {
    UnregisterClassW(kWindowClassName, instance_);
  }
}

HWND Window::GetHandle() const noexcept { return handle_; }

Window::ClientSize Window::GetClientSize() const noexcept { return clientSize_; }

std::optional<Window::ClientSize> Window::ConsumePendingResize() noexcept {
  const auto pendingResize = pendingResize_;
  pendingResize_.reset();
  return pendingResize;
}

std::vector<Window::KeyEvent> Window::ConsumeKeyEvents() {
  std::vector<KeyEvent> events;
  events.swap(pendingKeyEvents_);
  return events;
}

bool Window::ConsumeFocusLost() noexcept {
  const bool lost = focusLost_;
  focusLost_ = false;
  return lost;
}

std::optional<int> Window::ProcessMessages() const {
  MSG message{};

  while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
    if (message.message == WM_QUIT) {
      return static_cast<int>(message.wParam);
    }

    TranslateMessage(&message);
    DispatchMessageW(&message);
  }

  return std::nullopt;
}

void Window::SetTitle(std::wstring_view title) const {
  const std::wstring ownedTitle(title);
  SetWindowTextW(handle_, ownedTitle.c_str());
}

LRESULT CALLBACK Window::SetupWindowProc(HWND window, UINT message,
                                         WPARAM wParam, LPARAM lParam) {
  if (message == WM_NCCREATE) {
    const auto *createStruct = reinterpret_cast<CREATESTRUCTW *>(lParam);
    auto *self = static_cast<Window *>(createStruct->lpCreateParams);

    SetWindowLongPtrW(window, GWLP_USERDATA,
                      reinterpret_cast<LONG_PTR>(self));
    SetWindowLongPtrW(window, GWLP_WNDPROC,
                      reinterpret_cast<LONG_PTR>(&ForwardWindowProc));

    return self->HandleMessage(window, message, wParam, lParam);
  }

  return DefWindowProcW(window, message, wParam, lParam);
}

LRESULT CALLBACK Window::ForwardWindowProc(HWND window, UINT message,
                                           WPARAM wParam, LPARAM lParam) {
  auto *self = reinterpret_cast<Window *>(
      GetWindowLongPtrW(window, GWLP_USERDATA));

  if (!self) {
    return DefWindowProcW(window, message, wParam, lParam);
  }

  return self->HandleMessage(window, message, wParam, lParam);
}

LRESULT Window::HandleMessage(HWND window, UINT message, WPARAM wParam,
                              LPARAM lParam) {
  switch (message) {
  case WM_GETMINMAXINFO: {
    RECT minimumRect{0, 0, kMinimumClientWidth, kMinimumClientHeight};
    if (AdjustWindowRect(&minimumRect, WS_OVERLAPPEDWINDOW, FALSE) == 0) {
      return DefWindowProcW(window, message, wParam, lParam);
    }
    auto *limits = reinterpret_cast<MINMAXINFO *>(lParam);
    limits->ptMinTrackSize.x = minimumRect.right - minimumRect.left;
    limits->ptMinTrackSize.y = minimumRect.bottom - minimumRect.top;
    return 0;
  }

  case WM_KILLFOCUS:
    pendingKeyEvents_.clear();
    focusLost_ = true;
    return 0;

  case WM_KEYDOWN:
    pendingKeyEvents_.push_back(
        {static_cast<UINT>(wParam), true});
    return 0;

  case WM_SYSKEYDOWN:
    pendingKeyEvents_.push_back({static_cast<UINT>(wParam), true});
    return DefWindowProcW(window, message, wParam, lParam);

  case WM_KEYUP:
    pendingKeyEvents_.push_back(
        {static_cast<UINT>(wParam), false});
    return 0;

  case WM_SYSKEYUP:
    pendingKeyEvents_.push_back({static_cast<UINT>(wParam), false});
    return DefWindowProcW(window, message, wParam, lParam);

  case WM_SIZE:
    minimized_ = wParam == SIZE_MINIMIZED;
    if (wParam != SIZE_MINIMIZED) {
      clientSize_.width = LOWORD(lParam);
      clientSize_.height = HIWORD(lParam);

      if (clientSize_.width > 0 && clientSize_.height > 0) {
        pendingResize_ = clientSize_;
      }
    }
    return 0;

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  default:
    return DefWindowProcW(window, message, wParam, lParam);
  }
}
