#include "GameScene.h"

#include <stdexcept>

void GameScene::PrepareStageBenchmark(int stageNumber) {
  if (stageNumber < 1 || stageNumber > 3)
    throw std::invalid_argument("Benchmark stage must be 1, 2 or 3.");
  Reset();
  SetGodMode(true);
  visuals_ = SceneVisualSettings{};
  SetCollisionMode(CollisionMode::UniformGrid);
  for (int stage = 1; stage < stageNumber; ++stage)
    stage_.Advance();
  StartStage();
  // 최종보스는 대량 탄막이 나오는 후반 HP 페이즈를 재현한다.
  if (stageNumber == 3) {
    while (enemy_.GetHp() > enemy_.GetMaxHp() / 2)
      (void)enemy_.TryTakeDamage();
  }
  Input noInput;
  for (int tick = 0; tick < 15 * 60; ++tick)
    (void)Update(noInput, 1.0 / 60.0);
}
