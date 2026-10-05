#include "GameScene.h"
#include "GameSpriteRenderer.h"
#include "Graphics.h"
#include "HealthBar.hpp"
#include <cmath>
#include <stdexcept>

void GameScene::RunSmokeTest(Graphics &graphics,
                             const PlayfieldLayout &layout) {
  constexpr double kFixedDeltaSeconds = 1.0 / 60.0;
  Input testCombatInput;
  const auto render = [&] {
    graphics.BeginFrame(gameTimeSeconds_, 0.0);
    const GameSpriteRenderer renderer(graphics, layout, gameTimeSeconds_, 0.0);
    Render(renderer);
    graphics.EndFrame();
  };
  const auto require = [](bool condition, const char *message) {
    if (!condition) {
      throw std::runtime_error(message);
    }
  };

  const auto closeEnough = [](float a, float b) {
    return std::abs(a - b) < 0.00001f;
  };
  RunStageSmokeTest(graphics, layout);
  const auto prepareFinalBoss = [&] {
    stage_.Reset();
    stage_.Advance();
    stage_.Advance();
    StartStage();
  };
  require(
      closeEnough(CalculateBackgroundScrollOffset(0.0, 40.0f, 960.0f), 0.0f) &&
          closeEnough(CalculateBackgroundScrollOffset(23.5, 40.0f, 960.0f),
                      940.0f) &&
          closeEnough(CalculateBackgroundScrollOffset(24.0, 40.0f, 960.0f),
                      0.0f) &&
          closeEnough(CalculateBackgroundScrollOffset(24.5, 40.0f, 960.0f),
                      20.0f),
      "Background scroll did not wrap at a full tile height.");
  Enemy testEnemyA(EnemyKind::Enemy1, 200.0f, 180.0f);
  Enemy testEnemyB(EnemyKind::Enemy2, 520.0f, 180.0f);
  testEnemyA.Update(0.6);
  testEnemyB.Update(0.0);
  testEnemyA.Update(0.4);
  testEnemyB.Update(0.4);
  require(testEnemyA.ConsumeShot() && !testEnemyB.ConsumeShot(),
          "Enemies shared a firing cooldown.");
  require(testEnemyA.GetY() > 180.0f,
          "Enemy straight movement did not advance.");
  while (testEnemyA.GetHp() > 0)
    (void)testEnemyA.TryTakeDamage();
  const float deadEnemyY = testEnemyA.GetY();
  testEnemyA.Update(0.4);
  require(testEnemyA.GetLifeState() == EnemyLifeState::Dying &&
              closeEnough(testEnemyA.GetDissolveProgress(), 0.5f) &&
              testEnemyA.GetY() == deadEnemyY && !testEnemyA.ConsumeShot() &&
              !testEnemyA.TryTakeDamage(),
          "Dying enemy moved, fired or accepted damage.");
  testEnemyA.Update(0.4);
  require(testEnemyA.GetLifeState() == EnemyLifeState::Removed,
          "Enemy was not removable after its dissolve.");
  Reset();
  require(minions_.size() == 1 && !HasBoss(),
          "Stage one initial spawn failed.");
  while (minions_.front().GetHp() > 0)
    (void)minions_.front().TryTakeDamage();
  CheckBattleOutcome();
  require(battleOutcome_ == BattleOutcome::None,
          "An ordinary enemy death ended the battle.");
  UpdateMinions(0.8);
  require(minions_.empty(), "Dissolved ordinary enemy was not erased.");
  Reset();
  bulletSystem_.Spawn(player_.GetX() + 11.0f, player_.GetY(), 0.0f, 0.0f,
                      BulletOwner::Enemy, BulletType::Thin);
  RebuildEnemyBulletGrid();
  CheckPlayerEnemyBulletCollisions();
  require(player_.GetHp() == player_.GetMaxHp() && playerGraze_,
          "Thin bullet should graze without hitting at this distance.");
  const auto &thin = bulletSystem_.GetBullets().front();
  require(thin.type == BulletType::Thin && thin.width == 28.0f &&
              thin.height == 8.0f,
          "Thin bullet style was not applied on spawn.");
  render();
  Reset();
  bulletSystem_.Spawn(player_.GetX() + 11.0f, player_.GetY(), 0.0f, 0.0f,
                      BulletOwner::Enemy, BulletType::Normal);
  RebuildEnemyBulletGrid();
  CheckPlayerEnemyBulletCollisions();
  require(player_.GetHp() == player_.GetMaxHp() - 1,
          "Normal bullet should hit at the same distance.");
  Reset();
  SpriteAnimationData testAnimation{6, 3, 6, 6, 12.0, true};
  const auto firstUv = CalculateSpriteAnimationUv(testAnimation, 0.0);
  const auto lastUv = CalculateSpriteAnimationUv(testAnimation, 5.0 / 12.0);
  const auto wrappedUv = CalculateSpriteAnimationUv(testAnimation, 0.5);
  require(closeEnough(firstUv[0], 0.0f) &&
              closeEnough(firstUv[1], 1.0f / 3.0f) &&
              closeEnough(firstUv[2], 1.0f / 6.0f) &&
              closeEnough(lastUv[0], 5.0f / 6.0f) &&
              closeEnough(lastUv[2], 1.0f) && wrappedUv == firstUv,
          "Sprite animation row selection, timing or looping failed.");
  testAnimation.loop = false;
  require(CalculateSpriteAnimationUv(testAnimation, 10.0) == lastUv,
          "Non-looping sprite animation did not hold its final frame.");
  testAnimation.firstFrame = 5;
  testAnimation.frameCount = 2;
  const auto nextRowUv = CalculateSpriteAnimationUv(testAnimation, 1.0 / 12.0);
  require(closeEnough(nextRowUv[0], 0.0f) &&
              closeEnough(nextRowUv[1], 1.0f / 3.0f),
          "Sprite animation could not continue across sheet rows.");
  testAnimation.columns = 0;
  bool rejectedInvalidAnimation = false;
  try {
    (void)CalculateSpriteAnimationUv(testAnimation, 0.0);
  } catch (const std::invalid_argument &) {
    rejectedInvalidAnimation = true;
  }
  require(rejectedInvalidAnimation, "Invalid animation was not rejected.");

  Input testInput;
  testInput.SetKeyDown('Z', true);
  require(testInput.GetKeyState('Z') == Input::KeyState::Pressed,
          "Input press transition failed.");
  testInput.BeginFrame();
  require(testInput.GetKeyState('Z') == Input::KeyState::Held,
          "Input hold transition failed.");
  testInput.Reset();
  require(!testInput.IsDown('Z') &&
              testInput.GetKeyState('Z') == Input::KeyState::Up,
          "Input reset left a stale key state.");

  HealthBar testBar;
  testBar.SetValue(0.5f);
  testBar.SetPosition({100.0f, 100.0f});
  testBar.SetSize({240.0f, 12.0f});
  const auto firstSprites = testBar.GetSprites();
  require(firstSprites[1].size[0] == 120.0f &&
              firstSprites[1].center[0] == 40.0f,
          "HealthBar failed to preserve ratio and left edge.");
  testBar.SetValue(0.5f);
  require(testBar.GetSprites()[1].center == firstSprites[1].center,
          "HealthBar position correction accumulated.");

  render();
  Reset();
  render(); // Load and draw the player image, shaders and game UI.
  gameTimeSeconds_ = 0.25;
  playerMotion_ = PlayerMotion::Left;
  render();
  playerMotion_ = PlayerMotion::Right;
  render();
  gameTimeSeconds_ = 0.0;
  playerMotion_ = PlayerMotion::Idle;
  visuals_.dissolvePreviewProgress = 0.5f;
  render();
  visuals_.dissolvePreviewProgress = 1.0f;
  render();
  visuals_.dissolvePreviewProgress = 0.0f;
  require(player_.GetHp() == player_.GetMaxHp() &&
              enemy_.GetHp() == enemy_.GetMaxHp() && score_ == 0,
          "New game reset failed.");

  require(player_.GetX() == 360.0f && player_.GetY() == 800.0f &&
              enemy_.GetX() == 360.0f,
          "Actors did not start inside the vertical playfield.");
  player_.Update(1.0f, 1.0f, 10.0);
  player_.ClampToBounds(kPlayfieldWidth, kPlayfieldHeight);
  require(player_.GetX() == kPlayfieldWidth - player_.GetWidth() * 0.5f &&
              player_.GetY() == kPlayfieldHeight - player_.GetHeight() * 0.5f,
          "Player escaped the logical playfield.");
  player_.Reset();
  bulletSystem_.Spawn(800.0f, 200.0f, 0.0f, 0.0f, BulletOwner::Enemy);
  bulletSystem_.RemoveOutside(kPlayfieldWidth, kPlayfieldHeight);
  require(bulletSystem_.GetActiveCount() == 0,
          "Bullet survived outside the playfield in a side panel.");

  prepareFinalBoss();
  bulletSystem_.Spawn(enemy_.GetX(), enemy_.GetY(), 0.0f, 0.0f,
                      BulletOwner::Player);
  CheckEnemyPlayerBulletCollisions();
  CheckEnemyPlayerBulletCollisions();
  require(enemy_.GetHp() == enemy_.GetMaxHp() - 1 &&
              bulletSystem_.GetActiveCount() == 0,
          "A consumed player bullet applied repeated damage.");

  bulletSystem_.Spawn(player_.GetX(), player_.GetY(), 0.0f, 0.0f,
                      BulletOwner::Enemy);
  RebuildEnemyBulletGrid();
  CheckPlayerEnemyBulletCollisions();
  const int damagedHp = player_.GetHp();
  bulletSystem_.Spawn(player_.GetX(), player_.GetY(), 0.0f, 0.0f,
                      BulletOwner::Enemy);
  RebuildEnemyBulletGrid();
  CheckPlayerEnemyBulletCollisions();
  require(damagedHp == player_.GetMaxHp() - 1 && player_.GetHp() == damagedHp &&
              bulletSystem_.GetActiveCount() == 0,
          "Invulnerability or enemy bullet consumption failed.");

  bulletSystem_.Spawn(player_.GetX(), player_.GetY() - 40.0f);
  visuals_.bulletShape = SpriteShape::GlowCircle;
  visuals_.enhancedBullets = false;
  render();
  testCombatInput.SetKeyDown('B', true);
  HandleVisualControls(testCombatInput, true);
  require(visuals_.enhancedBullets, "Enhanced bullet rendering toggle failed.");
  testCombatInput.Reset();
  visuals_.bulletBlendMode = SpriteBlendMode::Alpha;
  render();
  visuals_.bulletBlendMode = SpriteBlendMode::Additive;
  render();
  while (enemy_.GetHp() > 0) {
    (void)enemy_.TryTakeDamage();
  }
  CheckBattleOutcome();
  require(IsEnding() && battleOutcome_ == BattleOutcome::Clear,
          "Clear did not enter its death effect phase.");
  const float survivingPlayerX = player_.GetX();
  const float frozenBulletY = bulletSystem_.GetBullets()[0].y;
  const auto frozenBulletCount = bulletSystem_.GetActiveCount();
  const double frozenGameTime = gameTimeSeconds_;
  testCombatInput.SetKeyDown(VK_RIGHT, true);
  testCombatInput.SetKeyDown('Z', true);
  for (int i = 0; i < 24; ++i) {
    (void)Update(testCombatInput, kFixedDeltaSeconds);
  }
  require(IsEnding() && closeEnough(GetDeathDissolveProgress(), 0.5f) &&
              player_.GetX() == survivingPlayerX &&
              bulletSystem_.GetBullets()[0].y == frozenBulletY &&
              bulletSystem_.GetActiveCount() == frozenBulletCount &&
              gameTimeSeconds_ == frozenGameTime,
          "Death effect failed to advance separately from frozen combat.");
  render();
  testCombatInput.Reset();
  for (int i = 0; i < 24; ++i) {
    (void)Update(testCombatInput, kFixedDeltaSeconds);
  }
  require(IsFinished() && GetDeathDissolveProgress() == 1.0f,
          "Result started before the complete death effect.");
  render();

  Reset();
  require(player_.GetHp() == player_.GetMaxHp() &&
              enemy_.GetHp() == enemy_.GetMaxHp() &&
              bulletSystem_.GetActiveCount() == 0 &&
              battleOutcome_ == BattleOutcome::None &&
              gameTimeSeconds_ == 0.0 && battleEndingSeconds_ == 0.0 &&
              visuals_.dissolvePreviewProgress == 0.0f,
          "Restart retained previous battle state.");
  prepareFinalBoss();
  while (enemy_.GetHp() > 0) {
    (void)enemy_.TryTakeDamage();
  }
  while (player_.GetHp() > 0) {
    (void)player_.TryTakeDamage();
    player_.Update(0.0f, 0.0f, 1.0);
  }
  CheckBattleOutcome();
  require(IsEnding() && battleOutcome_ == BattleOutcome::Failed,
          "Simultaneous defeat did not prefer failure.");
  render();
  for (int i = 0; i < 48; ++i) {
    (void)Update(testCombatInput, kFixedDeltaSeconds);
  }
  require(IsFinished() && battleOutcome_ == BattleOutcome::Failed,
          "Simultaneous death effects did not finish.");
  render();

  Reset();
  while (player_.GetHp() > 0) {
    (void)player_.TryTakeDamage();
    player_.Update(0.0f, 0.0f, 1.0);
  }
  CheckBattleOutcome();
  require(IsEnding() && enemy_.GetHp() == enemy_.GetMaxHp() &&
              battleOutcome_ == BattleOutcome::Failed,
          "Player-only death did not preserve the surviving enemy.");
  for (int i = 0; i < 24; ++i) {
    (void)Update(testCombatInput, kFixedDeltaSeconds);
  }
  render();
  for (int i = 0; i < 24; ++i) {
    (void)Update(testCombatInput, kFixedDeltaSeconds);
  }
  require(IsFinished(), "Player-only death effect did not finish.");
  render();
  // Leave an ending scene for Application's pause/screen-transition integration
  // check.
  Reset();
  while (player_.GetHp() > 0) {
    (void)player_.TryTakeDamage();
    player_.Update(0.0f, 0.0f, 1.0);
  }
  CheckBattleOutcome();
}
