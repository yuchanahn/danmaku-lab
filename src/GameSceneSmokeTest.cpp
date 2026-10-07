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
  // 생성 슬롯 순서와 풀 고갈/중복 반납/모드 전환을 두 경로에서 비교한다.
  BulletSystem scanPool;
  BulletSystem heapPool;
  scanPool.SetAllocationMode(BulletAllocationMode::LinearScan);
  for (std::size_t i = 0; i < scanPool.GetCapacity(); ++i) {
    scanPool.Spawn(static_cast<float>(i), 100.0f);
    heapPool.Spawn(static_cast<float>(i), 100.0f);
  }
  scanPool.Spawn(-1.0f, 100.0f);
  heapPool.Spawn(-1.0f, 100.0f);
  require(scanPool.GetDroppedSpawnRequests() == 1 &&
              heapPool.GetDroppedSpawnRequests() == 1,
          "Pool exhaustion did not drop exactly one spawn.");
  for (const std::size_t index : {73u, 2u, 6000u}) {
    scanPool.Release(index);
    heapPool.Release(index);
    heapPool.Release(index); // 이미 반납한 슬롯은 목록에 다시 넣지 않는다.
  }
  for (int i = 0; i < 4; ++i) {
    scanPool.Spawn(-100.0f - i, 100.0f);
    heapPool.Spawn(-100.0f - i, 100.0f);
  }
  require(heapPool.GetActiveCount() == heapPool.GetCapacity() &&
              heapPool.GetDroppedSpawnRequests() == 2,
          "Duplicate release corrupted the free-index heap.");
  for (std::size_t i = 0; i < scanPool.GetCapacity(); ++i)
    require(scanPool.GetBullets()[i].x == heapPool.GetBullets()[i].x &&
                scanPool.GetBullets()[i].active == heapPool.GetBullets()[i].active,
            "Allocation modes changed slot order or bullet contents.");
  heapPool.SetAllocationMode(BulletAllocationMode::LinearScan);
  heapPool.Release(9);
  heapPool.SetAllocationMode(BulletAllocationMode::FreeIndexHeap);
  heapPool.Spawn(42.0f, 100.0f);
  require(heapPool.GetBullets()[9].x == 42.0f,
          "Mode switch did not rebuild free indices.");
  heapPool.Clear();
  heapPool.BeginPoolMeasurement();
  heapPool.Spawn(-1000.0f, -1000.0f);
  heapPool.RemoveOutside(720.0f, 960.0f);
  const auto poolTiming = heapPool.GetPoolMeasurement();
  require(poolTiming.spawnCalls == 1 && poolTiming.released == 1 &&
              heapPool.GetActiveCount() == 0,
          "Outside removal or pool measurement omitted lifecycle work.");
  heapPool.EndPoolMeasurement();
  heapPool.Spawn(100.0f, 100.0f);
  require(heapPool.GetPoolMeasurement().spawnCalls == 1,
          "Pool timing continued after it was disabled.");
  heapPool.Reset();
  heapPool.Spawn(100.0f, 100.0f);
  require(heapPool.GetBullets().front().active &&
              heapPool.GetSpawnRequests() == 1 &&
              heapPool.GetDroppedSpawnRequests() == 0,
          "Pool reset did not restore the first free slot and counters.");
  for (const auto mode : {CollisionMode::LinearScan, CollisionMode::UniformGrid}) {
    Reset();
    SetCollisionMode(mode);
    SetGodMode(true);
    const int hpBefore = player_.GetHp();
    bulletSystem_.Spawn(player_.GetX(), player_.GetY(), 0.0f, 0.0f,
                        BulletOwner::Enemy);
    RebuildEnemyBulletGrid();
    CheckPlayerEnemyBulletCollisions();
    require(player_.GetHp() == hpBefore && !playerHit_ &&
                bulletSystem_.GetActiveCount() == 0 && collisionCandidateCount_ > 0,
            "God mode bypassed collision work or applied damage.");
    bulletSystem_.Spawn(player_.GetX(), player_.GetY(), 0.0f, 0.0f,
                        BulletOwner::Enemy);
    BeginCollisionMeasurement();
    (void)UpdateCombat(testCombatInput, kFixedDeltaSeconds);
    const auto collisionTiming = GetCollisionMeasurement();
    require(collisionTiming.ticks == 1 && collisionTiming.candidates > 0 &&
                collisionTiming.buildMilliseconds >= 0 &&
                collisionTiming.queryMilliseconds >= 0,
            "Collision measurement omitted work or counted ticks incorrectly.");
    EndCollisionMeasurement();
    (void)UpdateCombat(testCombatInput, kFixedDeltaSeconds);
    require(GetCollisionMeasurement().ticks == 1,
            "Collision timing continued after measurement was disabled.");
    Reset();
    require(IsGodMode(), "God mode setting did not survive restart.");
    SetGodMode(false);
    bulletSystem_.Spawn(player_.GetX(), player_.GetY(), 0.0f, 0.0f,
                        BulletOwner::Enemy);
    RebuildEnemyBulletGrid();
    CheckPlayerEnemyBulletCollisions();
    require(player_.GetHp() == hpBefore - 1 && playerHit_,
            "Damage was not restored after god mode was disabled.");
  }
  Reset();
  SetCollisionMode(CollisionMode::UniformGrid);
  RunStageSmokeTest(graphics, layout);
  // 셀 경계/화면 가장자리에서도 같은 배치의 판정 결과를 비교한다.
  for (const auto position :
       {std::array{64.0f, 64.0f}, std::array{127.0f, 128.0f},
        std::array{40.0f, 40.0f}, std::array{680.0f, 920.0f}}) {
    Reset();
    SetCollisionMode(CollisionMode::UniformGrid);
    player_.Update((position[0] - player_.GetX()) / 240.0f,
                   (position[1] - player_.GetY()) / 240.0f, 1.0);
    bulletSystem_.Spawn(player_.GetX(), player_.GetY(), 0.0f, 0.0f,
                        BulletOwner::Enemy);
    bulletSystem_.Spawn(player_.GetX() + 11.0f, player_.GetY(), 0.0f, 0.0f,
                        BulletOwner::Enemy, BulletType::Thin);
    bulletSystem_.Spawn(player_.GetX(), player_.GetY() + 20.0f, 0.0f, 0.0f,
                        BulletOwner::Enemy);
    bulletSystem_.Spawn(360.0f, 400.0f, 0.0f, 0.0f, BulletOwner::Enemy);
    bulletSystem_.Spawn(player_.GetX(), player_.GetY(), 0.0f, 0.0f,
                        BulletOwner::Player);
    bulletSystem_.Spawn(player_.GetX(), player_.GetY(), 0.0f, 0.0f,
                        BulletOwner::Enemy);
    bulletSystem_.Release(5);
    const auto initialPlayer = player_;
    const auto initialBullets = bulletSystem_.GetBullets();
    RebuildEnemyBulletGrid();
    CheckPlayerEnemyBulletCollisions();
    const auto gridStats = GetStatistics();
    const auto gridPlayer = player_;
    const auto gridBullets = bulletSystem_.GetBullets();
    CheckPlayerEnemyBulletCollisions();
    require(score_ == gridStats.score && grazeCount_ == gridStats.grazeCount,
            "Grid counted the same graze more than once.");

    player_ = initialPlayer;
    // 테스트 스냅샷을 복원할 때 파생된 빈 슬롯 목록도 다시 만든다.
    bulletSystem_.bullets_ = initialBullets;
    bulletSystem_.RebuildFreeIndices();
    score_ = grazeCount_ = 0;
    SetCollisionMode(CollisionMode::LinearScan);
    RebuildEnemyBulletGrid();
    for (std::size_t y = 0; y < enemyBulletGrid_.GetRows(); ++y)
      for (std::size_t x = 0; x < enemyBulletGrid_.GetColumns(); ++x)
        require(enemyBulletGrid_.GetCell(x, y).empty(),
                "Linear scan rebuilt the grid.");
    CheckPlayerEnemyBulletCollisions();
    const auto linearStats = GetStatistics();
    require(player_.GetHp() == gridPlayer.GetHp() &&
                player_.GetInvulnerabilitySeconds() ==
                    gridPlayer.GetInvulnerabilitySeconds() &&
                linearStats.score == gridStats.score &&
                linearStats.grazeCount == gridStats.grazeCount &&
                linearStats.playerHit == gridStats.playerHit &&
                linearStats.playerGraze == gridStats.playerGraze &&
                linearStats.activeEnemyBullets ==
                    gridStats.activeEnemyBullets &&
                linearStats.collisionCandidates == 4 &&
                gridStats.collisionCandidates < linearStats.collisionCandidates,
            "Linear and grid collision results differed.");
    for (std::size_t i = 0; i < gridBullets.size(); ++i)
      require(bulletSystem_.GetBullets()[i].active == gridBullets[i].active &&
                  bulletSystem_.GetBullets()[i].grazed == gridBullets[i].grazed,
              "Collision modes consumed or grazed different bullets.");
    CheckPlayerEnemyBulletCollisions();
    require(score_ == linearStats.score &&
                grazeCount_ == linearStats.grazeCount,
            "Linear scan counted the same graze more than once.");
    SetCollisionMode(CollisionMode::UniformGrid);
    RebuildEnemyBulletGrid();
    CheckPlayerEnemyBulletCollisions();
    require(score_ == linearStats.score &&
                player_.GetHp() == gridPlayer.GetHp(),
            "Switching collision modes changed existing bullet state.");
    SetCollisionMode(CollisionMode::LinearScan);
    Reset();
    require(GetCollisionMode() == CollisionMode::LinearScan,
            "Restart discarded the collision mode selection.");
  }
  SetCollisionMode(CollisionMode::UniformGrid);
  Reset();
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
  testCombatInput.Reset();
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
  require(!IsFinished(), "Result discarded uncollected rewards.");
  for (int i = 0; i < 600 && !IsFinished(); ++i)
    (void)Update(testCombatInput, kFixedDeltaSeconds);
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
