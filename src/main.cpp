#include "Application.h"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <windows.h>

namespace {
void ReportError(const char* message, bool smokeTest) {
  if (smokeTest) {
    std::cerr << "SMOKE TEST FAILED: " << message << '\n';
    return;
  }
  const int length = MultiByteToWideChar(CP_UTF8, 0, message, -1, nullptr, 0);
  if (length <= 0) {
    MessageBoxW(nullptr, L"An unexpected error occurred.", L"Danmaku Lab error",
                MB_OK | MB_ICONERROR);
    return;
  }
  std::wstring wideMessage(static_cast<std::size_t>(length), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, message, -1, wideMessage.data(), length);
  MessageBoxW(nullptr, wideMessage.c_str(), L"Danmaku Lab error",
              MB_OK | MB_ICONERROR);
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine,
                    int showCommand) {
  const bool smokeTest = std::wstring_view(commandLine) == L"--smoke-test";
  const bool compare = std::wstring_view(commandLine) == L"--benchmark-stage-fps-compare";
  const bool compareGrid = std::wstring_view(commandLine) == L"--benchmark-grid";
  const bool comparePool = std::wstring_view(commandLine) == L"--benchmark-pool";
  const bool benchmark = compare || compareGrid || comparePool || std::wstring_view(commandLine) == L"--benchmark-stage-fps";
  const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (FAILED(comResult)) {
    ReportError("Failed to initialize COM.", smokeTest);
    return EXIT_FAILURE;
  }

  int exitCode = EXIT_FAILURE;
  try {
    Application application(instance, smokeTest ? SW_HIDE : showCommand);
    exitCode = smokeTest ? application.RunSmokeTest()
                        : benchmark ? application.RunStageFpsBenchmark(compare, compareGrid, comparePool)
                                    : application.Run();
    if (smokeTest) {
      std::cout << "SMOKE TEST PASSED: resources, input, health bars, collision, "
                   "invulnerability, clear, failure and restart.\n";
    }
  } catch (const std::exception& exception) {
    ReportError(exception.what(), smokeTest || benchmark);
  }

  CoUninitialize();
  return exitCode;
}
