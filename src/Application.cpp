#include "Application.h"
#include "GameSpriteRenderer.h"
#include <algorithm>
#include <format>
#include <stdexcept>

Application::Application(HINSTANCE instance, int showCommand)
    : window_(instance, showCommand),
      graphics_(window_.GetHandle(), window_.GetClientSize().width,
                window_.GetClientSize().height) {}

int Application::Run() {
  while (true) {
    input_.BeginFrame();

    if (const auto exitCode = window_.ProcessMessages()) {
      return *exitCode;
    }

    HandlePendingInput();

    if (window_.IsMinimized()) {
      if (!WaitMessage()) {
        throw std::runtime_error("Failed to wait for a window message.");
      }
      (void)timer_.Tick();
      accumulatorSeconds_ = 0.0;
      continue;
    }

    const double realDeltaSeconds = timer_.Tick();

    HandlePendingResize();
    HandleGameStateControls();
    HandleVisualControls();
    const bool pauseChanged =
        (gameState_ == GameState::Playing || gameState_ == GameState::Ending) &&
        input_.GetKeyState('P') == Input::KeyState::Pressed;
    if (pauseChanged) {
      paused_ = !paused_;
      accumulatorSeconds_ = 0.0;
    }
    if ((gameState_ == GameState::Playing || gameState_ == GameState::Ending) &&
        !paused_ && !pauseChanged) {
      RunFixedUpdates(std::min(realDeltaSeconds, kMaxGameDeltaSeconds));
    }
    UpdateFps(realDeltaSeconds);
    Render();
  }
}

void Application::HandlePendingInput() {
  if (window_.ConsumeFocusLost()) {
    input_.Reset();
  }
  for (const auto &event : window_.ConsumeKeyEvents()) {
    input_.SetKeyDown(event.virtualKey, event.isDown);
  }
}

void Application::HandleGameStateControls() {
  if (input_.GetKeyState(VK_RETURN) != Input::KeyState::Pressed) {
    return;
  }

  if (gameState_ == GameState::Title) {
    ChangeGameState(GameState::Playing);
  } else if (gameState_ == GameState::Result) {
    ChangeGameState(GameState::Title);
  }
}

void Application::ChangeGameState(GameState nextState) {
  bool allowed = false;
  switch (nextState) {
  case GameState::Title:
    allowed = gameState_ == GameState::Result;
    break;
  case GameState::Playing:
    allowed = gameState_ == GameState::Title;
    break;
  case GameState::Ending:
    allowed = gameState_ == GameState::Playing;
    break;
  case GameState::Result:
    allowed = gameState_ == GameState::Ending;
    break;
  }

  if (!allowed) {
    return;
  }

  gameState_ = nextState;
  EnterGameState(nextState);
}

void Application::EnterGameState(GameState state) {
  if (state == GameState::Playing) {
    gameScene_.Reset();
    accumulatorSeconds_ = 0.0;
    paused_ = false;
    audio_.PlayBgm();
  }
  if (state == GameState::Ending) {
    paused_ = false;
    audio_.StopBgm();
  }
}

void Application::HandleVisualControls() {
  if (input_.GetKeyState(VK_F1) == Input::KeyState::Pressed)
    debugOverlayVisible_ = !debugOverlayVisible_;
  if (input_.GetKeyState(VK_F2) == Input::KeyState::Pressed)
    brightBackground_ = !brightBackground_;
  gameScene_.HandleVisualControls(input_, gameState_ == GameState::Playing);
}

void Application::HandlePendingResize() {
  const auto t = window_.ConsumePendingResize();
  if (t) {
    graphics_.Resize(t->width, t->height);
  }
}

void Application::RunFixedUpdates(double deltaSeconds) {
  if (paused_)
    return;
  accumulatorSeconds_ += deltaSeconds;
  while (accumulatorSeconds_ >= kFixedDeltaSeconds) {
    if (gameScene_.Update(input_, kFixedDeltaSeconds))
      audio_.PlayShot();
    if (gameState_ == GameState::Playing &&
        gameScene_.GetOutcome() != BattleOutcome::None)
      ChangeGameState(GameState::Ending);
    if (gameState_ == GameState::Ending && gameScene_.IsFinished())
      ChangeGameState(GameState::Result);
    accumulatorSeconds_ -= kFixedDeltaSeconds;
    if (gameState_ != GameState::Playing && gameState_ != GameState::Ending) {
      accumulatorSeconds_ = 0.0;
      break;
    }
  }
}

void Application::UpdateFps(double deltaSeconds) {
  frameCount++;
  elapsedSeconds += deltaSeconds;
  if (elapsedSeconds >= 1.f) {
    currentFps_ = frameCount / elapsedSeconds;
    window_.SetTitle(std::format(
        L"Danmaku Lab | {} | FPS: {:.1f} | Shots: {} | Bullets: {} | Game: "
        L"{:.1f}s | Real: "
        L"{:.1f}s",
        paused_ ? L"PAUSED (P to resume)" : L"RUNNING (P to pause)",
        currentFps_, gameScene_.GetStatistics().shotRequests,
        gameScene_.GetBulletSystem().GetActiveCount(),
        gameScene_.GetGameTimeSeconds(), timer_.GetElapsedSeconds()));
    elapsedSeconds = 0.0f;
    frameCount = 0;
  }
}

void Application::Render() {
  const auto size = window_.GetClientSize();
  const float width = static_cast<float>(size.width);
  const float height = static_cast<float>(size.height);
  const double realTime = timer_.GetElapsedSeconds();
  const auto layout = PlayfieldLayout::FromClientSize(width, height);
  const GameSpriteRenderer renderer(graphics_, layout,
                                    gameScene_.GetGameTimeSeconds(), realTime);
  const HudFrame frame{gameState_,           width,       height,  paused_,
                       debugOverlayVisible_, currentFps_, realTime};
  graphics_.BeginFrame(gameScene_.GetGameTimeSeconds(), realTime);
  gameHud_.RenderBackground(graphics_, width, height, brightBackground_);
  if (gameState_ != GameState::Title) {
    gameScene_.Render(renderer);
    if (gameState_ == GameState::Playing || gameState_ == GameState::Ending)
      gameHud_.RenderHealthBars(gameScene_, renderer);
    renderer.DrawSurround(width, height);
  }
  gameHud_.RenderDebug(graphics_, gameScene_, frame);
  gameHud_.RenderScreen(graphics_, gameScene_, layout, frame);
  graphics_.EndFrame();
}
