#include "Application.h"
#include <stdexcept>

int Application::RunSmokeTest() {
  const auto require = [](bool ok, const char *message) {
    if (!ok)
      throw std::runtime_error(message);
  };
  const auto full = PlayfieldLayout::FromClientSize(1920.0f, 1080.0f);
  const auto half = PlayfieldLayout::FromClientSize(960.0f, 540.0f);
  require(full.left == 600.0f && full.top == 60.0f && full.scale == 1.0f &&
              half.left == 300.0f && half.top == 30.0f && half.scale == 0.5f,
          "Playfield layout changed during extraction.");
  const SpriteDrawData centerSprite{{360.0f, 480.0f}, {48.0f, 48.0f}};
  const auto screen = full.ToScreen(centerSprite);
  require(screen.center[0] == 960.0f && screen.center[1] == 540.0f &&
              screen.size[0] == 48.0f,
          "Playfield sprite mapping failed.");

  Render();
  ChangeGameState(GameState::Playing);
  Render();
  const auto size = window_.GetClientSize();
  const auto layout = PlayfieldLayout::FromClientSize(
      static_cast<float>(size.width), static_cast<float>(size.height));
  gameScene_.RunSmokeTest(graphics_, layout);
  RunFixedUpdates(kFixedDeltaSeconds);
  require(gameState_ == GameState::Ending,
          "Scene outcome did not change the screen.");
  Render();
  const float progress = gameScene_.GetDeathDissolveProgress();
  paused_ = true;
  RunFixedUpdates(0.2);
  require(gameScene_.GetDeathDissolveProgress() == progress,
          "Application pause advanced the scene's death effect.");
  paused_ = false;
  input_.SetKeyDown(VK_RETURN, true);
  HandleGameStateControls();
  require(gameState_ == GameState::Ending, "Enter skipped the ending effect.");
  input_.Reset();
  for (int i = 0; i < 48; ++i)
    RunFixedUpdates(kFixedDeltaSeconds);
  require(gameState_ == GameState::Result,
          "Finished scene did not reach Result.");
  Render();
  ChangeGameState(GameState::Title);
  Render();
  ChangeGameState(GameState::Playing);
  require(gameScene_.GetOutcome() == BattleOutcome::None &&
              gameScene_.GetGameTimeSeconds() == 0.0 &&
              gameScene_.GetDeathDissolveProgress() == 0.0f &&
              gameScene_.GetStatistics().score == 0,
          "Application restart did not reset the scene.");
  input_.SetKeyDown(VK_LEFT, true);
  input_.SetKeyDown('Z', true);
  const float oldX = gameScene_.GetPlayer().GetX();
  RunFixedUpdates(kFixedDeltaSeconds);
  require(gameScene_.GetPlayer().GetX() < oldX &&
              gameScene_.GetStatistics().shotRequests == 1,
          "Application did not pass movement/shooting input to the scene.");
  Render();
  input_.Reset();
  return 0;
}
