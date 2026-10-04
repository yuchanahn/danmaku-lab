#include "Application.h"
#include "BulletSystem.h"
#include "Collision.h"
#include "DanmakuPattern.h"
#include "HealthBar.hpp"
#include <algorithm>
#include <cmath>
#include <format>
#include <numbers>
#include <winuser.h>

Application::Application(HINSTANCE instance, int showCommand)
    : window_(instance, showCommand),
      graphics_(window_.GetHandle(), window_.GetClientSize().width,
                window_.GetClientSize().height) {
  enemyHealthBar_.SetSize({240.0f, 12.0f});
}

int Application::Run() {
  while (true) {
    input_.BeginFrame();

    if (const auto exitCode = window_.ProcessMessages()) {
      return *exitCode;
    }

    const double realDeltaSeconds = timer_.Tick();

    HandlePendingResize();
    HandlePendingInput();
    HandleGameStateControls();
    HandleVisualControls();
    const bool pauseChanged =
        gameState_ == GameState::Playing &&
        input_.GetKeyState('P') == Input::KeyState::Pressed;
    if (pauseChanged) {
      paused_ = !paused_;
      accumulatorSeconds_ = 0.0;
    }
    if (gameState_ == GameState::Playing && !paused_ && !pauseChanged) {
      RunFixedUpdates(std::min(realDeltaSeconds, kMaxGameDeltaSeconds));
    }
    UpdateFps(realDeltaSeconds);
    Render();
  }
}

void Application::HandlePendingInput() {
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
  case GameState::Result:
    allowed = gameState_ == GameState::Playing;
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
    player_.Reset();
    enemy_.Reset();
    battleOutcome_ = BattleOutcome::None;
    bulletSystem_.Reset();
    enemyBulletGrid_.Clear();

    accumulatorSeconds_ = 0.0;
    gameTimeSeconds_ = 0.0;
    enemyShotCooldownSeconds_ = 0.0;
    enemyFanAngleRadians_ = 0.0f;
    paused_ = false;

    playerHit_ = false;
    playerGraze_ = false;
    grazeCount_ = 0;
    score_ = 0;
    activeEnemyBulletCount_ = 0;
    collisionCandidateCount_ = 0;
    shotRequestCount_ = 0;

    audio_.PlayBgm();
  }

  if (state == GameState::Result) {
    audio_.StopBgm();
  }
}

void Application::HandleVisualControls() {
  if (input_.GetKeyState(VK_F1) == Input::KeyState::Pressed) {
    debugOverlayVisible_ = !debugOverlayVisible_;
  }
  if (input_.GetKeyState('1') == Input::KeyState::Pressed) {
    playerUvRect_ = {0.0f, 0.0f, 1.0f, 1.0f};
  }
  if (input_.GetKeyState('2') == Input::KeyState::Pressed) {
    playerUvRect_ = {0.0f, 0.0f, 0.5f, 0.5f};
  }
  if (input_.GetKeyState('3') == Input::KeyState::Pressed) {
    playerUvRect_ = {1.0f, 0.0f, 0.0f, 1.0f};
  }
  if (input_.GetKeyState('4') == Input::KeyState::Pressed) {
    bulletShape_ = SpriteShape::Rectangle;
  }
  if (input_.GetKeyState('5') == Input::KeyState::Pressed) {
    bulletShape_ = SpriteShape::SoftCircle;
  }
  if (input_.GetKeyState('6') == Input::KeyState::Pressed) {
    bulletShape_ = SpriteShape::GlowCircle;
  }
  if (input_.GetKeyState('7') == Input::KeyState::Pressed) {
    bulletBlendMode_ = SpriteBlendMode::Alpha;
  }
  if (input_.GetKeyState('8') == Input::KeyState::Pressed) {
    bulletBlendMode_ = SpriteBlendMode::Additive;
  }
  if (input_.GetKeyState('9') == Input::KeyState::Pressed) {
    playerRotationRadians_ += 0.25f;
  }
  if (input_.GetKeyState('0') == Input::KeyState::Pressed) {
    playerRotationRadians_ = 0.0f;
  }
  if (input_.GetKeyState('N') == Input::KeyState::Pressed) {
    playerSamplerMode_ = SpriteSamplerMode::Point;
  }
  if (input_.GetKeyState('L') == Input::KeyState::Pressed) {
    playerSamplerMode_ = SpriteSamplerMode::Linear;
  }
  if (input_.GetKeyState('C') == Input::KeyState::Pressed) {
    playerAddressMode_ = SpriteAddressMode::Clamp;
    playerUvRect_ = {0.0f, 0.0f, 2.0f, 2.0f};
  }
  if (input_.GetKeyState('W') == Input::KeyState::Pressed) {
    playerAddressMode_ = SpriteAddressMode::Wrap;
    playerUvRect_ = {0.0f, 0.0f, 2.0f, 2.0f};
  }
}

void Application::HandlePendingResize() {
  const auto t = window_.ConsumePendingResize();
  if (t) {
    graphics_.Resize(t->width, t->height);
    const auto columns = static_cast<std::size_t>(
        (static_cast<float>(t->width) + kCollisionGridCellSize - 1.0f) /
        kCollisionGridCellSize);
    const auto rows = static_cast<std::size_t>(
        (static_cast<float>(t->height) + kCollisionGridCellSize - 1.0f) /
        kCollisionGridCellSize);
    enemyBulletGrid_.Resize(columns, rows);
  }
}

void Application::RunFixedUpdates(double deltaSeconds) {
  accumulatorSeconds_ += deltaSeconds;
  while (accumulatorSeconds_ >= kFixedDeltaSeconds) {
    Update(kFixedDeltaSeconds);
    gameTimeSeconds_ += kFixedDeltaSeconds;
    accumulatorSeconds_ -= kFixedDeltaSeconds;
    if (gameState_ != GameState::Playing) {
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
        L"{} | FPS: {:.1f} | Shots: {} | Bullets: {} | Game: {:.1f}s | Real: "
        L"{:.1f}s",
        paused_ ? L"PAUSED (P to resume)" : L"RUNNING (P to pause)",
        currentFps_, shotRequestCount_, bulletSystem_.GetActiveCount(),
        gameTimeSeconds_, timer_.GetElapsedSeconds()));
    elapsedSeconds = 0.0f;
    frameCount = 0;
  }
}

void Application::Update(double fixedDeltaSeconds) {
  float directionX = static_cast<float>(input_.IsDown(VK_RIGHT)) -
                     static_cast<float>(input_.IsDown(VK_LEFT));
  float directionY = static_cast<float>(input_.IsDown(VK_DOWN)) -
                     static_cast<float>(input_.IsDown(VK_UP));

  const float lengthSquared = directionX * directionX + directionY * directionY;
  if (lengthSquared > 0.0f) {
    const float length = std::sqrt(lengthSquared);
    directionX /= length;
    directionY /= length;
  }

  player_.Update(directionX, directionY, fixedDeltaSeconds);
  const auto size = window_.GetClientSize();
  player_.ClampToBounds(static_cast<float>(size.width),
                        static_cast<float>(size.height));

  bulletSystem_.Update(fixedDeltaSeconds);

  UpdateEnemyShooting(fixedDeltaSeconds);

  if (player_.UpdateShooting(input_.IsDown('Z'), fixedDeltaSeconds)) {
    ++shotRequestCount_;
    bulletSystem_.Spawn(player_.GetX(),
                        player_.GetY() - player_.GetHeight() * 0.5f);
    audio_.PlayShot();
  }
  RebuildEnemyBulletGrid();
  CheckEnemyPlayerBulletCollisions();
  CheckPlayerEnemyBulletCollisions();
  CheckBattleOutcome();
  bulletSystem_.RemoveOutside(static_cast<float>(size.width),
                              static_cast<float>(size.height));

  healthBar_.SetValue(player_.GetHp() / 3.f);
  healthBar_.SetPosition(
      {player_.GetX(), player_.GetY() + player_.GetHeight() + 8});
}

void Application::UpdateEnemyShooting(double fixedDeltaSeconds) {
  constexpr float kFanAngularSpeedRadiansPerSecond = 0.7f;
  enemyFanAngleRadians_ +=
      kFanAngularSpeedRadiansPerSecond * static_cast<float>(fixedDeltaSeconds);

  enemyShotCooldownSeconds_ -= fixedDeltaSeconds;
  if (enemy_.GetHp() > enemy_.GetMaxHp() / 2 &&
      enemyShotCooldownSeconds_ <= 0.0) {
    constexpr int kFanBulletCount = 7;
    constexpr float kFanBulletSpeed = 140.0f;
    constexpr float kFanCenterAngle =
        static_cast<float>(std::numbers::pi / 2.0);
    constexpr float kFanSpreadAngle =
        static_cast<float>(std::numbers::pi / 2.0);
    float fanCenterAngle = kFanCenterAngle;
    fanCenterAngle += enemyFanAngleRadians_;
    DanmakuPattern::SpawnFan(bulletSystem_, enemy_.GetX(), enemy_.GetY(),
                             kFanBulletCount, kFanBulletSpeed, fanCenterAngle,
                             kFanSpreadAngle);
    enemyShotCooldownSeconds_ = 1.5;
  } else if (enemy_.GetHp() > 0 && enemyShotCooldownSeconds_ <= 0.0) {
    constexpr int kFanBulletCount = 16;
    constexpr float kFanBulletSpeed = 160.0f;

    DanmakuPattern::SpawnRing(bulletSystem_, enemy_.GetX(), enemy_.GetY(),
                              kFanBulletCount, kFanBulletSpeed,
                              enemyFanAngleRadians_);
    enemyShotCooldownSeconds_ = 1.0;
  }
}

void Application::CheckBattleOutcome() {
  if (player_.GetHp() == 0) {
    battleOutcome_ = BattleOutcome::Failed;
    ChangeGameState(GameState::Result);
  } else if (enemy_.GetHp() == 0) {
    battleOutcome_ = BattleOutcome::Clear;
    ChangeGameState(GameState::Result);
  }
}

void Application::CheckEnemyPlayerBulletCollisions() {

  constexpr float kEnemyHitRadius = 24.0f;
  constexpr float kPlayerBulletHitRadius = 4.0f;
  const CircleHitbox Hitbox{
      enemy_.GetX(),
      enemy_.GetY(),
      kEnemyHitRadius,
  };

  for (auto &bullet : bulletSystem_.GetBullets()) {
    if (!bullet.active || bullet.owner != BulletOwner::Player)
      continue;
    const CircleHitbox bulletHitbox{
        bullet.x,
        bullet.y,
        kPlayerBulletHitRadius,
    };
    const bool bulletHits = Intersects(Hitbox, bulletHitbox);

    if (bulletHits) {
      bullet.active = false;
      if (enemy_.TryTakeDamage()) {
      }
    }
  }
}

void Application::RebuildEnemyBulletGrid() {
  enemyBulletGrid_.Clear();
  activeEnemyBulletCount_ = 0;

  const auto size = window_.GetClientSize();
  const auto &bullets = bulletSystem_.GetBullets();
  for (std::size_t bulletIndex = 0; bulletIndex < bullets.size();
       ++bulletIndex) {
    const auto &bullet = bullets[bulletIndex];
    if (!bullet.active || bullet.owner != BulletOwner::Enemy) {
      continue;
    }
    ++activeEnemyBulletCount_;

    if (bullet.x < 0.0f || bullet.y < 0.0f ||
        bullet.x >= static_cast<float>(size.width) ||
        bullet.y >= static_cast<float>(size.height)) {
      continue;
    }

    auto cellX = static_cast<std::size_t>(bullet.x / kCollisionGridCellSize);
    auto cellY = static_cast<std::size_t>(bullet.y / kCollisionGridCellSize);

    if (cellY >= enemyBulletGrid_.GetRows() ||
        cellX >= enemyBulletGrid_.GetColumns()) {
      continue;
    }

    enemyBulletGrid_.Insert(cellX, cellY, bulletIndex);
  }
}

void Application::CheckPlayerEnemyBulletCollisions() {
  constexpr float kPlayerHitRadius = 5.0f;
  constexpr float kPlayerGrazeRadius = 24.0f;
  constexpr float kEnemyBulletHitRadius = 8.0f;

  playerHit_ = false;
  playerGraze_ = false;
  collisionCandidateCount_ = 0;
  const CircleHitbox playerHitbox{
      player_.GetX(),
      player_.GetY(),
      kPlayerHitRadius,
  };
  const CircleHitbox playerGrazeHitbox{
      player_.GetX(),
      player_.GetY(),
      kPlayerGrazeRadius,
  };

  const std::size_t playerCellX =
      static_cast<std::size_t>(player_.GetX() / kCollisionGridCellSize);
  const std::size_t playerCellY =
      static_cast<std::size_t>(player_.GetY() / kCollisionGridCellSize);

  const std::size_t minCellX = playerCellX > 0 ? playerCellX - 1 : 0;
  const std::size_t minCellY = playerCellY > 0 ? playerCellY - 1 : 0;
  const std::size_t maxCellX =
      std::min(playerCellX + 1, enemyBulletGrid_.GetColumns() - 1);
  const std::size_t maxCellY =
      std::min(playerCellY + 1, enemyBulletGrid_.GetRows() - 1);

  auto &bullets = bulletSystem_.GetBullets();
  for (std::size_t cellY = minCellY; cellY <= maxCellY; ++cellY) {
    for (std::size_t cellX = minCellX; cellX <= maxCellX; ++cellX) {
      for (const std::size_t bulletIndex :
           enemyBulletGrid_.GetCell(cellX, cellY)) {
        auto &bullet = bullets[bulletIndex];
        if (!bullet.active) {
          continue;
        }

        const CircleHitbox bulletHitbox{
            bullet.x,
            bullet.y,
            kEnemyBulletHitRadius,
        };

        ++collisionCandidateCount_;
        const bool bulletHitsPlayer = Intersects(playerHitbox, bulletHitbox);
        if (bulletHitsPlayer) {
          const bool damageApplied = player_.TryTakeDamage();
          playerHit_ = playerHit_ || damageApplied;
          bullet.active = false;
        }

        const bool isGraze =
            Intersects(playerGrazeHitbox, bulletHitbox) && !bulletHitsPlayer;
        playerGraze_ = playerGraze_ || isGraze;

        if (isGraze && !bullet.grazed) {
          bullet.grazed = true;
          grazeCount_++;
          score_ += kGrazeScore;
        }
      }
    }
  }
}

void Application::Render() {
  graphics_.BeginFrame(gameTimeSeconds_, timer_.GetElapsedSeconds());

  RenderCheckerBackground();

  switch (gameState_) {

  case GameState::Title:
    RenderTitle();
    break;
  case GameState::Playing:
    RenderPlaying();
    RenderGameUI();
    break;
  case GameState::Result:
    RenderResult();
    break;
  }

  RenderDebugOverlay();
  RenderHud();
  graphics_.EndFrame();
}

void Application::RenderCheckerBackground() {

  const auto clientSize = window_.GetClientSize();
  constexpr float kCheckerSize = 64.0f;
  constexpr std::array lightCheckerTint{0.78f, 0.79f, 0.81f, 1.0f};
  constexpr std::array darkCheckerTint{0.48f, 0.50f, 0.53f, 1.0f};

  int row = 0;
  for (float y = 0.0f; y < static_cast<float>(clientSize.height);
       y += kCheckerSize, ++row) {
    int column = 0;
    for (float x = 0.0f; x < static_cast<float>(clientSize.width);
         x += kCheckerSize, ++column) {
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

void Application::RenderTitle() {}

void Application::RenderPlaying() {

  float alpha = 1.f;

  if (player_.IsInvulnerable()) {
    alpha = static_cast<float>(sin(player_.GetInvulnerabilitySeconds() * 2.f *
                                   std::numbers::pi * 6.f)) > 0.f
                ? 1.f
                : 0.25f;
  }
  const SpriteDrawData playerSprite{
      {player_.GetX(), player_.GetY()},
      {player_.GetWidth(), player_.GetHeight()},
      SpriteTextureId::Player,
      {1.0f, 1.0f, 1.0f, alpha},
      SpriteTimeSource::Real,
      playerUvRect_,
      SpriteShape::Rectangle,
      SpriteBlendMode::Alpha,
      playerRotationRadians_,
      playerSamplerMode_,
      playerAddressMode_,
  };
  graphics_.DrawSprite(playerSprite);

  const SpriteDrawData enemySprite{
      {enemy_.GetX(), enemy_.GetY()}, {enemy_.GetWidth(), enemy_.GetHeight()},
      SpriteTextureId::White,         {0.55f, 0.15f, 0.85f, 1.0f},
      SpriteTimeSource::Game,         {0.0f, 0.0f, 1.0f, 1.0f},
      SpriteShape::SoftCircle,        SpriteBlendMode::Alpha,
  };
  graphics_.DrawSprite(enemySprite);

  for (const auto &bullet : bulletSystem_.GetBullets()) {
    if (!bullet.active) {
      continue;
    }

    if (bulletShape_ == SpriteShape::GlowCircle) {
      const SpriteDrawData glowSprite{
          {bullet.x, bullet.y},    {bullet.width * 3.0f, bullet.height * 3.0f},
          SpriteTextureId::Yellow, {1.8f, 0.55f, 0.15f, 0.75f},
          SpriteTimeSource::Game,  {0.0f, 0.0f, 1.0f, 1.0f},
          SpriteShape::GlowCircle, bulletBlendMode_,
      };
      graphics_.DrawSprite(glowSprite);

      const SpriteDrawData coreSprite{
          {bullet.x, bullet.y},    {bullet.width, bullet.height},
          SpriteTextureId::Yellow, {1.0f, 0.35f, 0.15f, 1.0f},
          SpriteTimeSource::Game,  {0.0f, 0.0f, 1.0f, 1.0f},
          SpriteShape::SoftCircle, SpriteBlendMode::Alpha,
      };
      graphics_.DrawSprite(coreSprite);
      continue;
    }

    const SpriteDrawData bulletSprite{
        {bullet.x, bullet.y},
        {bullet.width, bullet.height},
        SpriteTextureId::Yellow,
        {1.0f, 0.2f, 0.2f, 0.5f},
        SpriteTimeSource::Game,
        {0.0f, 0.0f, 1.0f, 1.0f},
        bulletShape_,
        bulletBlendMode_,
    };
    graphics_.DrawSprite(bulletSprite);
  }
}

void Application::RenderResult() { RenderPlaying(); }

void Application::RenderHud() {
  const auto size = window_.GetClientSize();
  const float width = static_cast<float>(size.width);
  const float height = static_cast<float>(size.height);

  if (gameState_ == GameState::Playing) {
    const float panelWidth = std::min(300.0f, width);
    const float left = std::max(0.0f, width - panelWidth - 12.0f);
    const auto text = std::format(
        L"HP  {}   INVULN  {:.1f}s\nENEMY HP  {}/{}\nSCORE  {}\nGRAZE  {}\n{}",
        player_.GetHp(), player_.GetInvulnerabilitySeconds(), enemy_.GetHp(),
        enemy_.GetMaxHp(), score_, grazeCount_,
        paused_ ? L"PAUSED  [P: Resume]" : L"Z: Shoot  P: Pause");
    graphics_.DrawUiPanel(text, {left, 12.0f, left + panelWidth, 148.0f});
    return;
  }

  const float panelWidth = std::min(320.0f, width);
  const float panelHeight = std::min(150.0f, height);
  const float left = (width - panelWidth) * 0.5f;
  const float top = (height - panelHeight) * 0.5f;
  const auto text =
      gameState_ == GameState::Title
          ? std::wstring{L"DANMAKU SHOOTER\n\nEnter: Start\nArrows: Move   Z: "
                         L"Shoot"}
          : std::format(L"{}\nSCORE  {}\nGRAZE  {}\n\nEnter: Back to Title",
                        battleOutcome_ == BattleOutcome::Clear    ? L"CLEAR"
                        : battleOutcome_ == BattleOutcome::Failed ? L"FAILED"
                                                                  : L"RESULT",
                        score_, grazeCount_);
  graphics_.DrawUiPanel(text,
                        {left, top, left + panelWidth, top + panelHeight});
}

void Application::RenderGameUI() {
  enemyHealthBar_.SetValue(static_cast<float>(enemy_.GetHp()) /
                           static_cast<float>(enemy_.GetMaxHp()));
  enemyHealthBar_.SetPosition(
      {static_cast<float>(window_.GetClientSize().width) * 0.5f, 32.0f});
  for (const auto &sprite : enemyHealthBar_.GetSprites()) {
    graphics_.DrawSprite(sprite);
  }
  for (const auto &sprite : healthBar_.GetSprites()) {
    graphics_.DrawSprite(sprite);
  }
}

void Application::RenderDebugOverlay() {
  if (debugOverlayVisible_) {
    const wchar_t *gameStateName = gameState_ == GameState::Title ? L"Title"
                                   : gameState_ == GameState::Playing
                                       ? L"Playing"
                                       : L"Result";
    const wchar_t *samplerName =
        playerSamplerMode_ == SpriteSamplerMode::Point ? L"Point" : L"Linear";
    const wchar_t *addressName =
        playerAddressMode_ == SpriteAddressMode::Clamp ? L"Clamp" : L"Wrap";
    const wchar_t *blendName =
        bulletBlendMode_ == SpriteBlendMode::Alpha ? L"Alpha" : L"Additive";
    const wchar_t *shapeName =
        bulletShape_ == SpriteShape::Rectangle    ? L"Rectangle"
        : bulletShape_ == SpriteShape::SoftCircle ? L"SoftCircle"
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
        L"Arrows Move   Z Shoot   P Pause\n"
        L"1 Full UV  2 Quarter UV  3 Flip U\n"
        L"4 Rect  5 Circle  6 Glow\n"
        L"7 Alpha  8 Additive\n"
        L"9 Rotate  0 Reset Rotation\n"
        L"N Point  L Linear  C Clamp  W Wrap",
        gameStateName, currentFps_, bulletSystem_.GetActiveCount(),
        bulletSystem_.GetBullets().size(), activeEnemyBulletCount_,
        collisionCandidateCount_, gameTimeSeconds_, timer_.GetElapsedSeconds(),
        paused_ ? L"On" : L"Off", playerHit_ ? L"YES" : L"no",
        playerGraze_ ? L"YES" : L"no", grazeCount_, samplerName, addressName,
        shapeName, blendName);
    graphics_.DrawDebugText(overlayText);
  }
}
