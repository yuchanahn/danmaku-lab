#include "Application.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

int Application::RunStageFpsBenchmark(bool compare, bool compareGrid, bool comparePool) {
  const auto directory = std::filesystem::path("out") / "benchmarks";
  std::filesystem::create_directories(directory);
  const auto stamp = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  const auto path = directory / ((comparePool ? "pool-compare-" : compareGrid ? "grid-compare-" : compare ? "stage-compare-" : "stage-fps-") +
                                std::to_string(stamp) + ".csv");
  std::ofstream csv(path);
  if (!csv)
    throw std::runtime_error("Failed to open benchmark CSV.");
  csv << "build,adapter,stage,repeat,width,height,game_start_s,game_end_s,frames,wall_s,"
         "fps,mean_frame_ms,p95_frame_ms,p99_frame_ms,bullets_min,bullets_mean,"
         "bullets_max,dropped,mode,collision_ticks,build_ms_per_tick,"
         "query_ms_per_tick,collision_ms_per_tick,candidates_per_tick,"
         "pool_timing,spawn_calls,release_calls,inspected_slots,spawn_ms,release_ms\n";
  csv.flush();
  LARGE_INTEGER frequency{};
  if (!QueryPerformanceFrequency(&frequency))
    throw std::runtime_error("Benchmark QueryPerformanceFrequency failed.");
  const auto now = [&]() {
    LARGE_INTEGER counter{};
    if (!QueryPerformanceCounter(&counter))
      throw std::runtime_error("Benchmark QueryPerformanceCounter failed.");
    return static_cast<double>(counter.QuadPart) / frequency.QuadPart;
  };
  const auto initialSize = window_.GetClientSize();
  const auto adapterName = graphics_.GetAdapterName();
  // 창이 숨겨진 상태의 Present 처리량을 정상 플레이 FPS로 측정하지 않는다.
  ShowWindow(window_.GetHandle(), SW_SHOWNOACTIVATE);
  gameState_ = GameState::Playing;
  paused_ = false;
  debugOverlayVisible_ = false;
  brightBackground_ = false;
  audio_.StopBgm();
  std::cout << "Stage FPS benchmark: " << path.string() << std::endl;
  for (int repeat = 1; repeat <= 3; ++repeat) {
    for (int stage = (compareGrid || comparePool) ? 3 : 1; stage <= 3; ++stage) {
      for (int pass = 0; pass < (comparePool ? 4 : (compare || compareGrid) ? 2 : 1); ++pass) {
      // 반복마다 순서를 반전해 한 방식만 항상 나중에 측정되지 않도록 한다.
      const bool secondMode = repeat % 2 == 1 ? pass % 2 == 1 : pass % 2 == 0;
      const bool poolTiming = comparePool && pass >= 2;
      const auto allocationMode = comparePool && !secondMode
          ? BulletAllocationMode::LinearScan : BulletAllocationMode::FreeIndexHeap;
      const char *allocationName = allocationMode == BulletAllocationMode::LinearScan ? "scan" : "heap";
      const bool instanced = !compareGrid && (!compare || secondMode);
      const auto collisionMode = compareGrid && !secondMode
          ? CollisionMode::LinearScan : CollisionMode::UniformGrid;
      const char *collisionName = collisionMode == CollisionMode::UniformGrid ? "grid" : "linear";
      const char *renderMode = instanced ? "instanced" : "individual";
      gameScene_.PrepareStageBenchmark(stage);
      gameScene_.SetInstancedBullets(instanced);
      gameScene_.SetCollisionMode(collisionMode);
      gameScene_.SetBulletAllocationMode(allocationMode);
      accumulatorSeconds_ = 0.0;
      timer_.Reset();
      elapsedSeconds = 0.0;
      frameCount = 0;
      std::vector<double> frameMilliseconds;
      frameMilliseconds.reserve(100000);
      std::size_t bulletMin = std::numeric_limits<std::size_t>::max();
      std::size_t bulletMax = 0;
      double bulletSum = 0.0;
      double gameStart = 0.0;
      double sampleStart = 0.0;
      const double warmupStart = now();
      double previousStart = warmupStart;
      bool measuring = false;
      while (true) {
        const double frameStart = now();
        if (!measuring && frameStart - warmupStart >= 2.0) {
          measuring = true;
          sampleStart = frameStart;
          gameStart = gameScene_.GetGameTimeSeconds();
          if (compareGrid)
            gameScene_.BeginCollisionMeasurement();
          if (poolTiming)
            gameScene_.BeginPoolMeasurement();
        }
        const double delta = frameStart - previousStart;
        previousStart = frameStart;
        input_.BeginFrame();
        if (window_.ProcessMessages())
          throw std::runtime_error("Benchmark cancelled: window closed.");
        HandlePendingInput();
        input_.Reset(); // 재현 조건을 유지하도록 사람의 입력은 측정에 섞지 않는다.
        const auto size = window_.GetClientSize();
        if (window_.IsMinimized() || size.width != initialSize.width ||
            size.height != initialSize.height)
          throw std::runtime_error("Benchmark cancelled: minimized or resized.");
        RunFixedUpdates(std::min(delta, kMaxGameDeltaSeconds));
        UpdateFps(delta);
        (void)timer_.Tick();
        Render();
        if (graphics_.WasPresentOccluded())
          throw std::runtime_error("Benchmark cancelled: presentation occluded.");
        const double frameEnd = now();
        if (!measuring)
          continue;
        frameMilliseconds.push_back((frameEnd - frameStart) * 1000.0);
        const auto bullets = gameScene_.GetBulletSystem().GetActiveCount();
        bulletMin = std::min(bulletMin, bullets);
        bulletMax = std::max(bulletMax, bullets);
        bulletSum += static_cast<double>(bullets);
        if (frameEnd - sampleStart < 5.0)
          continue;
        const double wall = frameEnd - sampleStart;
        const double fps = static_cast<double>(frameMilliseconds.size()) / wall;
        gameScene_.EndCollisionMeasurement();
        gameScene_.EndPoolMeasurement();
        const auto pool = gameScene_.GetBulletSystem().GetPoolMeasurement();
        const auto collision = gameScene_.GetCollisionMeasurement();
        const double ticks = static_cast<double>(collision.ticks);
        const double buildMs = ticks > 0 ? collision.buildMilliseconds / ticks : 0;
        const double queryMs = ticks > 0 ? collision.queryMilliseconds / ticks : 0;
        std::ranges::sort(frameMilliseconds);
        const auto percentile = [&](double fraction) {
          return frameMilliseconds[static_cast<std::size_t>(
              fraction * (frameMilliseconds.size() - 1))];
        };
#ifdef _DEBUG
        constexpr auto build = "Debug";
#else
        constexpr auto build = "Release";
#endif
        csv << build << ",\"" << adapterName << "\"," << stage << ',' << repeat << ',' << size.width << ','
            << size.height << ',' << gameStart << ','
            << gameScene_.GetGameTimeSeconds() << ',' << frameMilliseconds.size()
            << ',' << wall << ',' << fps << ',' << 1000.0 / fps << ','
            << percentile(0.95) << ',' << percentile(0.99) << ',' << bulletMin
            << ',' << bulletSum / frameMilliseconds.size() << ',' << bulletMax
            << ',' << gameScene_.GetBulletSystem().GetDroppedSpawnRequests()
            << ",live_fixed60_" << renderMode
            << '_' << collisionName << "_godmode_noinput_hud_bgmoff"
            << (comparePool ? std::string("_") + allocationName : "") << ','
            << collision.ticks << ',' << buildMs << ',' << queryMs << ','
            << buildMs + queryMs << ','
            << (ticks > 0 ? collision.candidates / ticks : 0) << ','
            << (poolTiming ? 1 : 0) << ',' << pool.spawnCalls << ','
            << pool.released << ',' << pool.inspectedSlots << ','
            << pool.spawnMilliseconds << ',' << pool.releaseMilliseconds << '\n';
        csv.flush();
        if (!csv)
          throw std::runtime_error("Failed to write benchmark CSV.");
        std::cout << "Stage " << stage << ' ' << renderMode << ' ' << collisionName << ' ' << allocationName << " repeat " << repeat << ": " << fps
                  << " FPS, bullets " << bulletMin << ".." << bulletMax
                  << std::endl;
        break;
      }
      }
    }
  }
  return 0;
}
