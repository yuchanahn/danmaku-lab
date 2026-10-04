#include "Application.h"

#include <cstdlib>
#include <exception>
#include <windows.h>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
  const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (FAILED(comResult)) {
    MessageBoxA(nullptr, "Failed to initialize COM.", "DanmakuShooter error",
                MB_OK | MB_ICONERROR);
    return EXIT_FAILURE;
  }

  int exitCode = EXIT_FAILURE;
  try {
    Application application(instance, showCommand);
    exitCode = application.Run();
  } catch (const std::exception &exception) {
    MessageBoxA(nullptr, exception.what(), "DanmakuShooter error",
                MB_OK | MB_ICONERROR);
  }

  CoUninitialize();
  return exitCode;
}
