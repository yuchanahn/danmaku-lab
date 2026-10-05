#include "GameHud.h"
#include "GameScene.h"
#include "GameSpriteRenderer.h"
#include "Graphics.h"
#include <algorithm>
#include <format>

GameHud::GameHud() { enemyHealthBar_.SetSize({240.0f, 12.0f}); }

void GameHud::RenderBackground(Graphics &graphics_, float width, float height,
                               bool brightBackground_) {

  constexpr float kCheckerSize = 64.0f;
  const std::array lightCheckerTint =
      brightBackground_ ? std::array{0.78f, 0.79f, 0.81f, 1.0f}
                        : std::array{0.035f, 0.05f, 0.08f, 1.0f};
  const std::array darkCheckerTint =
      brightBackground_ ? std::array{0.48f, 0.50f, 0.53f, 1.0f}
                        : std::array{0.025f, 0.035f, 0.06f, 1.0f};

  int row = 0;
  for (float y = 0.0f; y < height; y += kCheckerSize, ++row) {
    int column = 0;
    for (float x = 0.0f; x < width; x += kCheckerSize, ++column) {
      const auto &checkerTint =
          ((row + column) % 2 == 0) ? lightCheckerTint : darkCheckerTint;
      const SpriteDrawData checkerTile{
          {x + kCheckerSize * 0.5f, y + kCheckerSize * 0.5f},
          {kCheckerSize, kCheckerSize},
          SpriteTextureId::White,
          checkerTint,
          SpriteTimeSource::Real,
          {0.0f, 0.0f, 1.0f, 1.0f},
          SpriteShape::Rectangle,
          SpriteBlendMode::Alpha,
      };
      graphics_.DrawSprite(checkerTile);
    }
  }
}

void GameHud::RenderHealthBars(const GameScene &scene,
                               const GameSpriteRenderer &renderer) {
  healthBar_.SetValue(static_cast<float>(scene.GetPlayer().GetHp()) /
                      static_cast<float>(scene.GetPlayer().GetMaxHp()));
  constexpr float kHealthBarGap = 8.0f;
  const float halfBarHeight = healthBar_.GetHeight() * 0.5f;
  const float barY = scene.GetPlayer().GetY() -
                     scene.GetPlayer().GetHeight() * 0.5f - kHealthBarGap -
                     halfBarHeight;
  healthBar_.SetPosition(
      {scene.GetPlayer().GetX(), std::max(halfBarHeight, barY)});
  enemyHealthBar_.SetValue(static_cast<float>(scene.GetEnemy().GetHp()) /
                           static_cast<float>(scene.GetEnemy().GetMaxHp()));
  enemyHealthBar_.SetPosition({PlayfieldLayout::kWidth * 0.5f, 32.0f});
  if (scene.GetEnemy().GetHp() > 0) {
    for (const auto &sprite : enemyHealthBar_.GetSprites()) {
      renderer.DrawGameSprite(sprite);
    }
  }
  if (scene.GetPlayer().GetHp() > 0) {
    for (const auto &sprite : healthBar_.GetSprites()) {
      renderer.DrawGameSprite(sprite);
    }
  }
}

void GameHud::RenderScreen(Graphics &graphics_, const GameScene &scene,
                           const PlayfieldLayout &layout,
                           const HudFrame &frame) {
  const auto stats = scene.GetStatistics();
  const float width = frame.width;
  const float height = frame.height;

  if (frame.state == GameState::Playing || frame.state == GameState::Ending) {
    const std::array bounds{layout.left, layout.top,
                            PlayfieldLayout::kWidth * layout.scale,
                            PlayfieldLayout::kHeight * layout.scale};
    const float left = bounds[0] + bounds[2] + 12.0f;
    const float panelWidth = std::min(300.0f, width - left - 12.0f);
    const auto text = std::format(
        L"HP  {}   INVULN  {:.1f}s\nENEMY HP  {}/{}\nSCORE  {}\nGRAZE  {}\n{}",
        scene.GetPlayer().GetHp(),
        scene.GetPlayer().GetInvulnerabilitySeconds(), scene.GetEnemy().GetHp(),
        scene.GetEnemy().GetMaxHp(), stats.score, stats.grazeCount,
        frame.paused                       ? L"PAUSED  [P: Resume]"
        : frame.state == GameState::Ending ? L"BATTLE ENDING...  P: Pause"
                                           : L"Z: Shoot  P: Pause   F1: Debug");
    graphics_.DrawUiPanel(text, {left, 12.0f, left + panelWidth, 148.0f});
    return;
  }

  const float panelWidth = std::min(320.0f, width);
  const float panelHeight = std::min(150.0f, height);
  const float left = (width - panelWidth) * 0.5f;
  const float top = (height - panelHeight) * 0.5f;
  const auto text =
      frame.state == GameState::Title
          ? std::wstring{L"DANMAKU LAB\n\nEnter: Start\nArrows: Move   Z: "
                         L"Shoot"}
          : std::format(L"{}\nSCORE  {}\nGRAZE  {}\n\nEnter: Back to Title",
                        scene.GetOutcome() == BattleOutcome::Clear ? L"CLEAR"
                        : scene.GetOutcome() == BattleOutcome::Failed
                            ? L"FAILED"
                            : L"RESULT",
                        stats.score, stats.grazeCount);
  graphics_.DrawUiPanel(text,
                        {left, top, left + panelWidth, top + panelHeight});
}

void GameHud::RenderDebug(Graphics &graphics_, const GameScene &scene,
                          const HudFrame &frame) {
  const auto stats = scene.GetStatistics();
  const auto &visuals = scene.GetVisualSettings();
  if (frame.debugVisible) {
    const wchar_t *gameStateName =
        frame.state == GameState::Title     ? L"Title"
        : frame.state == GameState::Playing ? L"Playing"
        : frame.state == GameState::Ending  ? L"Ending"
                                            : L"Result";
    const wchar_t *samplerName =
        visuals.playerSamplerMode == SpriteSamplerMode::Point ? L"Point"
                                                              : L"Linear";
    const wchar_t *addressName =
        visuals.playerAddressMode == SpriteAddressMode::Clamp ? L"Clamp"
                                                              : L"Wrap";
    const wchar_t *blendName = visuals.bulletBlendMode == SpriteBlendMode::Alpha
                                   ? L"Alpha"
                                   : L"Additive";
    const wchar_t *shapeName =
        visuals.bulletShape == SpriteShape::Rectangle    ? L"Rectangle"
        : visuals.bulletShape == SpriteShape::SoftCircle ? L"SoftCircle"
                                                         : L"GlowCircle";

    const std::wstring overlayText = std::format(
        L"DEBUG OVERLAY  [F1: hide]\n"
        L"State: {}\n"
        L"FPS: {:.1f}\n"
        L"Bullets: {} / {}\n"
        L"Enemy Bullets: {} | Collision Candidates: {}\n"
        L"Game: {:.1f}s | Real: {:.1f}s\n"
        L"Pause: {} | Player Hit: {} | Graze: {} | Graze Count: {}\n"
        L"\n"
        L"Render State\n"
        L"Sampler: {} | Address: {}\n"
        L"Bullet: {} | Blend: {}\n\n"
        L"Controls\n"
        L"Arrows Move   Z Shoot   P Pause   F2 Background\n"
        L"F3 Dissolve preview: {:.2f}\n"
        L"1 Full UV  2 Quarter UV  3 Flip U\n"
        L"4 Rect  5 Circle  6 Glow\n"
        L"7 Alpha  8 Additive\n"
        L"9 Rotate  0 Reset Rotation\n"
        L"N Point  L Linear  C Clamp  W Wrap",
        gameStateName, frame.fps, scene.GetBulletSystem().GetActiveCount(),
        scene.GetBulletSystem().GetBullets().size(), stats.activeEnemyBullets,
        stats.collisionCandidates, scene.GetGameTimeSeconds(), frame.realTime,
        frame.paused ? L"On" : L"Off", stats.playerHit ? L"YES" : L"no",
        stats.playerGraze ? L"YES" : L"no", stats.grazeCount, samplerName,
        addressName, shapeName, blendName, visuals.dissolvePreviewProgress);
    graphics_.DrawDebugText(overlayText);
  }
}
