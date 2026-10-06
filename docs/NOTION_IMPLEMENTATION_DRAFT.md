<callout icon="🔮" color="green_bg">
	**마법 숲에서 펼쳐지는 3스테이지 탄막 슈팅**
	C++ · DirectX 11로 창 생성부터 렌더링, 전투와 스테이지 진행까지 구현했습니다.
</callout>
<columns>
	<column ratio="65">
		*\[대표 GIF · 최종보스 탄막과 적 소환\]* {color="gray"}
	</column>
	<column ratio="35">
		**PROJECT INFO** {color="green"}
		개인 프로젝트 · Windows x64
		C++23 · Win32 · DirectX 11 · HLSL
		\[개발 기간\]
		[↗ 소스 코드](https://github.com/yuchanahn/danmaku-lab)
		\[전체 플레이 영상 링크\]
		\[실행 파일 링크\]
	</column>
</columns>
게임 루프·렌더링·전투를 다루는 학습 프로젝트로, 코드 리뷰와 일부 구현에 AI 보조를 사용했습니다. {color="gray"}
---
# 01 · 스테이지와 보스전 {color="green"}
*\[영상 · 일반 적 → 중간보스 → 최종보스\]* {color="gray"}
**일반 적 10마리 → 중간보스 → 최종보스.** 최종보스는 일반 적을 소환하고, HP가 절반 이하가 되면 탄환 수와 발사 빈도가 증가합니다. 적이 없는 순간에도 예정된 스폰이 남아 있으면 다음 스테이지로 넘어가지 않습니다.
<columns>
	<column ratio="34">
		<callout icon="🌱" color="gray_bg">
			**일반 적 웨이브**
			\[사진 · 적 1과 적 2\]
			총 10마리 처치
		</callout>
	</column>
	<column ratio="33">
		<callout icon="🧙" color="gray_bg">
			**중간보스전**
			\[사진 · 중간보스\]
			처치 후 최종보스 진입
		</callout>
	</column>
	<column ratio="33">
		<callout icon="👑" color="gray_bg">
			**최종보스전**
			\[사진 · 보스와 소환된 적\]
			HP 절반 이하에서 패턴 강화
		</callout>
	</column>
</columns>
*\[GIF · 최종보스 HP 절반 전 / 후\]* {color="gray"}
<details>
<summary>**스폰 조건 · 패턴 계산 · 주요 코드**</summary>
	StageDirector가 시간표와 전환 조건을 관리하고 GameScene이 실제 적을 생성합니다. 보스 사망은 HP 0과 연출 종료를 구분해 처리합니다.
	원형·부채꼴 패턴은 각도를 속도 벡터로 바꿉니다. 최종보스는 반대 방향의 두 원형 패턴과 조준 부채꼴을 조합합니다.
	<details>
	<summary>스폰 소비와 전환 조건</summary>
		`src/StageDirector.cpp` · 함수 발췌
		```c++
std::optional<EnemySpawnEvent> StageDirector::ConsumeSpawn() {
  if (stage_ != 1 || phase_ != StagePhase::Running || AllSpawnsIssued() ||
      kStageOneSpawns[nextSpawn_].time > elapsed_ + 1e-9)
    return std::nullopt;
  return kStageOneSpawns[nextSpawn_++];
}

bool StageDirector::CanAdvance(bool enemiesRemoved,
                               bool bossRemoved) const noexcept {
  if (stage_ == 1)
    return AllSpawnsIssued() && enemiesRemoved;
  return stage_ == 2 && phase_ == StagePhase::Transitioning && bossRemoved &&
         transitionSeconds_ + 1e-9 >= 0.8;
}
		```
	</details>
	<details>
	<summary>원형·부채꼴 속도 벡터</summary>
		`src/DanmakuPattern.cpp` · 함수 발췌
		```c++
void SpawnRing(BulletSystem &bulletSystem, float centerX, float centerY,
               int bulletCount, float speedPixelsPerSecond,
               float startAngleRadians, BulletType type) {
  if (bulletCount <= 0 || speedPixelsPerSecond <= 0.0f) {
    return;
  }
  for (int i = 0; i < bulletCount; i++) {
    float r = static_cast<float>(std::numbers::pi * 2.0) *
              (static_cast<float>(i) + 1.f) / bulletCount;

    bulletSystem.Spawn(centerX, centerY,
                       std::cos(r + startAngleRadians) * speedPixelsPerSecond,
                       std::sin(r + startAngleRadians) * speedPixelsPerSecond,
                       BulletOwner::Enemy, type);
  }
}

void SpawnFan(BulletSystem &bulletSystem, float centerX, float centerY,
              int bulletCount, float speedPixelsPerSecond,
              float centerAngleRadians, float spreadAngleRadians,
              BulletType type) {
  if (bulletCount <= 0 || speedPixelsPerSecond <= 0.0f ||
      spreadAngleRadians < 0.0f) {
    return;
  }

  if (bulletCount == 1) {
    bulletSystem.Spawn(centerX, centerY,
                       std::cos(centerAngleRadians) * speedPixelsPerSecond,
                       std::sin(centerAngleRadians) * speedPixelsPerSecond,
                       BulletOwner::Enemy, type);
    return;
  }

  for (int i = 0; i < bulletCount; i++) {
    float t = static_cast<float>(i) / static_cast<float>(bulletCount - 1);
    float r = (centerAngleRadians - (spreadAngleRadians / 2)) +
              spreadAngleRadians * t;

    bulletSystem.Spawn(centerX, centerY, std::cos(r) * speedPixelsPerSecond,
                       std::sin(r) * speedPixelsPerSecond, BulletOwner::Enemy,
                       type);
  }
}
		```
	</details>
	<details>
	<summary>HP 강화와 적 소환</summary>
		`src/GameSceneStages.cpp` · 함수 발췌
		```c++
void GameScene::UpdateEnemyShooting(double fixedDeltaSeconds) {
  if (!bossPresent_ || enemy_.GetLifeState() != EnemyLifeState::Alive)
    return;
  enemy_.Update(fixedDeltaSeconds);
  const float age = static_cast<float>(enemy_.GetAnimationTimeSeconds());
  const float aimedAngle = std::atan2(player_.GetY() - enemy_.GetY(),
                                      player_.GetX() - enemy_.GetX());
  const bool enraged = enemy_.GetHp() <= enemy_.GetMaxHp() / 2;
  if (enemy_.GetKind() == EnemyKind::MidBoss) {
    if (!enemy_.ConsumeShot(enraged ? 0.65 : 1.0))
      return;
    if (enraged) {
      DanmakuPattern::SpawnRing(bulletSystem_, enemy_.GetX(), enemy_.GetY(), 24,
                                155.0f, age * 0.7f);
      DanmakuPattern::SpawnFan(bulletSystem_, enemy_.GetX(), enemy_.GetY(), 5,
                               190.0f, aimedAngle, 0.65f, BulletType::Thin);
    } else {
      DanmakuPattern::SpawnFan(bulletSystem_, enemy_.GetX(), enemy_.GetY(), 9,
                               150.0f, aimedAngle, 1.4f, BulletType::Thin);
    }
    return;
  }

  summonCooldownSeconds_ -= fixedDeltaSeconds;
  if (summonCooldownSeconds_ <= 0.0) {
    // Count both living and dissolving minions toward the summon limit.
    if (minions_.size() <= 6) {
      minions_.emplace_back(EnemyKind::Enemy1,
                            std::clamp(enemy_.GetX() - 140.0f, 110.0f, 610.0f),
                            enemy_.GetY() + 85.0f);
      minions_.emplace_back(EnemyKind::Enemy2,
                            std::clamp(enemy_.GetX() + 140.0f, 110.0f, 610.0f),
                            enemy_.GetY() + 85.0f);
    }
    summonCooldownSeconds_ = enraged ? 4.0 : 5.5;
  }
  if (!enemy_.ConsumeShot(enraged ? 0.20 : 0.30))
    return;
  ++bossVolleyIndex_;
  const int count = enraged ? 96 : 48;
  const float phase = age * 0.65f;
  const float halfStep = static_cast<float>(std::numbers::pi) / count;
  DanmakuPattern::SpawnRing(bulletSystem_, enemy_.GetX(), enemy_.GetY(), count,
                            110.0f, phase, BulletType::Normal);
  DanmakuPattern::SpawnRing(bulletSystem_, enemy_.GetX(), enemy_.GetY(), count,
                            145.0f, -phase + halfStep, BulletType::Thin);
  if (bossVolleyIndex_ % 3 == 0) {
    DanmakuPattern::SpawnFan(bulletSystem_, enemy_.GetX(), enemy_.GetY(),
                             enraged ? 11 : 7, 200.0f, aimedAngle, 0.8f,
                             BulletType::Thin);
  }
}
		```
	</details>
</details>
---
# 02 · 충돌 검사와 공간분할 {color="green"}
*\[비교 영상 · 같은 탄막에서 F5 전체 순회 / Grid 전환\]* {color="gray"}
최종보스 HP가 절반 이하가 되면 약 **3,000발**의 탄환이 유지됩니다. 플레이어 주변 3×3 셀의 탄환만 검사하도록 Uniform Grid를 적용하고, 전체 순회와 비용을 비교했습니다.
<table fit-page-width="true" header-row="true">
<tr>
<td>고정 업데이트 1틱당</td>
<td>전체 순회</td>
<td>Uniform Grid</td>
</tr>
<tr>
<td>검사 후보</td>
<td>3,105개</td>
<td>66개</td>
</tr>
<tr>
<td>조회·충돌 판정</td>
<td>20.64µs</td>
<td>1.34µs</td>
</tr>
<tr>
<td>Grid 구축</td>
<td>0.05µs</td>
<td>20.68µs</td>
</tr>
<tr color="orange_bg">
<td>**구축 + 조회·판정**</td>
<td>**20.69µs**</td>
<td>**21.84µs**</td>
</tr>
</table>
후보는 **97.9% 감소**했지만 총비용은 **5.6% 증가**했습니다. 플레이어 한 명을 조회하는 동안 모든 탄환을 셀에 다시 등록해야 했기 때문입니다. 이 장면에서는 가벼운 원 충돌 판정을 직접 반복하는 편이 더 저렴했습니다.
<details>
<summary>**Grid 구축 코드**</summary>
	F5는 후보 선택 방식만 바꿉니다. 명중·무적·탄환 제거·Graze 처리는 두 방식이 공유하며, Grid는 매 고정 업데이트에서 재구축합니다.
	`src/GameScene.cpp` · 실제 구현
	```c++
void GameScene::RebuildEnemyBulletGrid() {
  activeEnemyBulletCount_ = 0;
  if (collisionMode_ == CollisionMode::LinearScan)
    return;
  enemyBulletGrid_.Clear();

  const auto &bullets = bulletSystem_.GetBullets();
  for (std::size_t bulletIndex = 0; bulletIndex < bullets.size();
       ++bulletIndex) {
    const auto &bullet = bullets[bulletIndex];
    if (!bullet.active || bullet.owner != BulletOwner::Enemy) {
      continue;
    }
    ++activeEnemyBulletCount_;

    if (bullet.x < 0.0f || bullet.y < 0.0f || bullet.x >= kPlayfieldWidth ||
        bullet.y >= kPlayfieldHeight) {
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
	```
</details>
<details>
<summary>**충돌 비교 · 측정 조건과 CSV**</summary>
	최종보스 HP50% / 활성 탄환2,976\~3,216개 / 개별 렌더링 고정. Release / RTX3080 Laptop GPU / client1920×1080 / Present(0,0).
	무적·입력 없음, HUD·배경 ON, BGM·디버그 OFF. 15초 준비 → 2초warmup → 5초수집, 각3회·순서반전. 표는 틱당 평균 시간의3회 중앙값이며 충돌 업데이트는60Hz입니다.
	steady_clock으로 Grid 구축과 플레이어 대 적 탄환의 조회·판정을 측정했습니다. 플레이어 탄환 대 적 충돌은 별도 경로입니다.
	재현: `./benchmark-stage-fps.ps1 -Grid`
	<file src="notion-file-block://7bc2714c-b466-49be-9db9-324751a8c062/81e4deff-62cd-47cb-af9f-b116fd99f1b2?space_id=f5f1cb4e-7b06-4063-b0e8-b02a88514e38&name=grid-comparison-2026-10-07.csv">충돌 비교 · 원본 CSV 6구간</file>
</details>
---
# 03 · 대량 탄막 렌더링 {color="green"}
<columns>
	<column ratio="50">
		*\[영상 · 개별 Draw / 최종보스 HP50%\]* {color="gray"}
	</column>
	<column ratio="50">
		*\[영상 · 인스턴싱 / 같은 탄막·후광\]* {color="gray"}
	</column>
</columns>
충돌 검사와 함께 탄환 렌더링 경로를 살펴봤습니다. 기존에는 **후광 → 몸체**를 각각 그리며, 한 발마다 Draw 2회와 Map 4회를 호출했습니다.
WPR로 호출 스택을 수집하고 WPA에서 게임 프로세스를 확인했습니다. `DrawSprite` 아래 **Map과 드라이버 경로가 CPU 샘플 가중치의 68.6%**를 차지했습니다.
![WPA · DrawSprite의 Map 호출 경로](notion-file-block://634eaf66-1cfa-4ec3-8ff3-4edc026e1171/cb8de7f6-73ea-4861-bacf-f8b2c496c0d4?space_id=f5f1cb4e-7b06-4063-b0e8-b02a88514e38&name=image.png)
위 사진의 `DrawSprite` 샘플 가중치는 약2,493ms, 그 아래 Map 경로는 약1,709ms였습니다. 반복 버퍼 갱신을 줄이기 위해 탄환 데이터를 배열로 모아 제출했습니다. [Microsoft · DX11 CPU 제출 비용](https://microsoft.github.io/DirectX-Specs/d3d/CPUEfficiency.html#problems)
공유 쿼드에 개체별 위치·크기·색·회전 기저·형태를 전달하고, **후광 한 번 → 몸체 한 번**씩 인스턴싱합니다. 배열 저장 공간도 재사용합니다.
<table fit-page-width="true" header-row="true">
<tr>
<td>탄환 N발의 렌더 호출</td>
<td>개별 Draw</td>
<td>인스턴싱</td>
</tr>
<tr>
<td>Draw</td>
<td>2N회</td>
<td>2회</td>
</tr>
<tr>
<td>Map</td>
<td>4N회</td>
<td>2회</td>
</tr>
</table>
<table fit-page-width="true" header-row="true">
<tr>
<td>최종보스 HP50% · 2,976\~3,216발</td>
<td>개별 Draw</td>
<td>인스턴싱</td>
</tr>
<tr color="blue_bg">
<td>**FPS · 3회 중앙값**</td>
<td>**650**</td>
<td>**1,789**</td>
</tr>
<tr>
<td>평균 프레임 시간</td>
<td>1.540ms</td>
<td>0.559ms</td>
</tr>
</table>
**프레임 시간이 63.7% 줄었습니다.** 후광·색·크기·블렌드·출력 순서를 유지한 채 기존 개별 경로와 비교했습니다.
<details>
<summary>**인스턴스 배열 업로드와 Draw 코드**</summary>
	한 번 순회해 후광과 몸체 배열을 함께 만들고, 화면 좌표로 변환한 데이터를 span으로 전달합니다. 인스턴스 버퍼는 공유 정점 버퍼와 별도 입력 슬롯에 연결합니다.
	`src/GraphicsBulletInstances.cpp` · 실제 구현
	```c++
void Graphics::DrawBulletInstances(
    std::span<const BulletSpriteInstanceData> instances,
    SpriteBlendMode blendMode) {
  if (instances.empty())
    return;

  UploadBulletInstances(instances);
  BindBulletInstancePipeline(blendMode);

  context_->DrawIndexedInstanced(6, static_cast<UINT>(instances.size()), 0, 0,
                                 0);

  BindSpritePipeline();
}

void Graphics::UploadBulletInstances(
    std::span<const BulletSpriteInstanceData> instances) {
  if (instances.empty())
    return;
  if (instances.size() > kBulletInstanceCapacity) {
    throw std::length_error("Bullet instance batch exceeds buffer capacity.");
  }

  D3D11_MAPPED_SUBRESOURCE mapped{};
  ThrowIfFailed(context_->Map(bulletInstanceBuffer_.Get(), 0,
                              D3D11_MAP_WRITE_DISCARD, 0, &mapped),
                "Failed to map the bullet instance buffer.");

  std::memcpy(mapped.pData, instances.data(),
              sizeof(BulletSpriteInstanceData) * instances.size());

  context_->Unmap(bulletInstanceBuffer_.Get(), 0);
}
	```
	HLSL에서는 두 회전 기저의 선형 결합으로 로컬 좌표를 회전시킵니다.
	```c++
float2 RotateInstanceLocalPosition(float2 localPosition,
                                   float2 rotationXAxis, float2 rotationYAxis)
{
    return localPosition.x * rotationXAxis + localPosition.y * rotationYAxis;
}
	```
</details>
<details>
<summary>**렌더링 비교 · 측정 조건과 CSV**</summary>
	Release / RTX3080 Laptop GPU / client1920×1080 / Present(0,0). 두 방식 모두 Grid·무적 ON, 입력 없음, HUD·배경 ON, BGM·디버그 OFF.
	각 스테이지15초 준비 → 2초warmup → 5초수집, 각3회·순서반전. 같은 EXE에서 개별 렌더링과 인스턴싱을 전환했습니다. 각 짝의 탄수 범위가 같고 생성 누락은0입니다.
	QPC로 게임 루프의 프레임 수와 경과 시간을 수집했습니다. FPS는 앱 처리량, 프레임 시간은 측정 시간/프레임 수입니다. Draw·Map 횟수는 탄환 후광/몸체 코드 기준입니다.
	<table fit-page-width="true" header-row="true">
<tr>
<td>스테이지</td>
<td>활성 탄환</td>
<td>개별 FPS</td>
<td>인스턴스 FPS</td>
</tr>
<tr>
<td>일반 적</td>
<td>56\~67</td>
<td>3,291</td>
<td>3,601</td>
</tr>
<tr>
<td>중간보스</td>
<td>40\~50</td>
<td>3,138</td>
<td>3,043</td>
</tr>
<tr>
<td>최종보스</td>
<td>2,976\~3,216</td>
<td>650</td>
<td>1,789</td>
</tr>
	</table>
	최종보스의 반복별 배율은2.48/2.76/2.51배였습니다.
	재현: `./benchmark-stage-fps.ps1 -Compare`
	<file src="notion-file-block://c4cb17b0-5baa-44ef-b9ac-954744f4d95a/5f767068-a68c-42df-8704-211ae15cf24a?space_id=f5f1cb4e-7b06-4063-b0e8-b02a88514e38&name=stage-comparison-2026-10-07.csv">렌더링 전후 비교 · 원본 CSV 18구간</file>
</details>
---
# 04 · 탄환 풀의 빈 슬롯 관리 {color="green"}
탄환 8,192개를 미리 확보해 재사용했지만, 기존 `Spawn`은 발사할 때마다 첫 빈 슬롯을 찾았습니다. 5초 동안 4,550발을 생성하며 활성 플래그를 **약 788만 번** 확인했습니다.
빈 인덱스를 **최소 힙**에 보관해 생성 탐색을 O(N)에서 O(log N)으로 바꿨습니다. 가장 작은 인덱스를 선택해 기존 슬롯 배치와 렌더 순서를 유지하고, 충돌·화면 밖 제거는 `Release`로 모아 중복 반납을 막았습니다.
<table fit-page-width="true" header-row="true">
<tr>
<td>5초 구간 시간 합계 · 3회 중앙값</td>
<td>전체 순회</td>
<td>최소 힙</td>
</tr>
<tr>
<td>생성</td>
<td>2.408ms</td>
<td>0.380ms</td>
</tr>
<tr>
<td>반납</td>
<td>0.083ms</td>
<td>0.310ms</td>
</tr>
<tr color="blue_bg">
<td>**생성 + 반납**</td>
<td>**2.492ms**</td>
<td>**0.690ms**</td>
</tr>
</table>
생성·반납 비용은 **72.3% 감소**했습니다. 절감한 시간은 5초 전체에서 약 1.80ms였습니다. 전체 FPS는 세 번의 비교 중 한 번 상승하고 두 번 하락했습니다.
<details>
<summary>**빈 슬롯 선택과 반납 코드**</summary>
	`src/BulletSystem.cpp` · 슬롯 선택과 Release 발췌
	```c++
  auto target = bullets_.end();
  if (allocationMode_ == BulletAllocationMode::LinearScan) {
    target = std::ranges::find(bullets_, false, &Bullet::active);
    if (measuring_)
      measurement_.inspectedSlots += static_cast<std::size_t>(target - bullets_.begin()) +
                                    (target != bullets_.end() ? 1 : 0);
  } else if (!freeIndices_.empty()) {
    std::ranges::pop_heap(freeIndices_, std::greater<>{});
    target = bullets_.begin() + freeIndices_.back();
    freeIndices_.pop_back();
  }

void BulletSystem::Release(std::size_t index) noexcept {
  if (index >= bullets_.size() || !bullets_[index].active)
    return;
  const auto start = measuring_ ? Clock::now() : Clock::time_point{};
  bullets_[index].active = false;
  if (allocationMode_ == BulletAllocationMode::FreeIndexHeap) {
    freeIndices_.push_back(index);
    std::ranges::push_heap(freeIndices_, std::greater<>{});
  }
  if (measuring_) {
    measurement_.releaseMilliseconds += ElapsedMilliseconds(start);
    ++measurement_.released;
  }
}
	```
	빈 목록 저장 공간은 처음에 풀 용량만큼 확보합니다. Clear/Reset/방식 전환 때 빈 목록을 재구성하고, 이동·렌더의 전체 슬롯 순회는 유지했습니다.
	Debug·Release 자동 검사로 풀 고갈·중복 반납·선택 순서·재시작·충돌 동작을 확인했습니다.
</details>
<details>
<summary>**풀 비교 · 측정 조건과 CSV**</summary>
	최종보스 HP50% / 활성 탄환2,976\~3,216개 / 인스턴싱·Grid 고정. 장비와 실행 설정은 위 렌더링 비교와 같습니다.
	각 방식3회,15초 준비 → 2초warmup → 5초수집·순서반전. FPS6구간은 생성/반납 타이머OFF, 별도 시간6구간은 타이머ON입니다. 모든 시간 구간 생성4,550회/누락0입니다.
	생성 시간은 슬롯 선택·초기화, 반납 시간은 유효 반납의 상태 변경·힙 갱신을 steady_clock으로 측정했습니다.
	FPS는 전체 순회/힙 순으로1회2313/2276,2회2121/2182,3회1922/1895였습니다. 실험은 각각 별도 실행으로 수집했습니다.
	재현: `./benchmark-stage-fps.ps1 -Pool`
	<file src="notion-file-block://eea4c3e7-3365-40a1-97b9-ff9e27da731b/b070039f-c30d-4ff3-beb5-f90d8ffafa86?space_id=f5f1cb4e-7b06-4063-b0e8-b02a88514e38&name=pool-comparison-2026-10-07.csv">탄환 풀 비교 · 원본 CSV 12구간</file>
	<page url="https://app.notion.com/p/3f1d058c022e809aad5fd4b072492e01">많은 탄환 처리 프레임 떨어짐 현상.</page>
</details>
---
# 05 · 디졸브와 사망 연출 {color="green"}
<columns>
	<column ratio="50">
		*\[GIF · 적 처치 디졸브\]* {color="gray"}
	</column>
	<column ratio="50">
		*\[GIF · 플레이어 사망 디졸브\]* {color="gray"}
	</column>
</columns>
HP가 0이 되는 순간 삭제하면 사망 효과도 사라집니다. 적을 **Alive → Dying → Removed**로 나눠, 이동과 발사를 멈춘 뒤 0.8초 동안 디졸브를 재생합니다. 셰이더는 노이즈와 진행도를 비교해 픽셀을 제거하고 남은 경계를 밝게 표시합니다.
<details>
<summary>**디졸브 셰이더 · 객체 수명 코드**</summary>
	\[진행도 0 / 0.35 / 0.7 / 1 비교 사진\]
	<details>
	<summary>픽셀 제거와 경계 밝기</summary>
		`shaders/Sprite.hlsl` · 함수 발췌
		```c++
void ApplyDissolve(float noiseValue, float progress)
{
    if (progress <= 0.0f)
        return;
    if (progress >= 1.0f)
    {
        clip(-1.0f);
        return;
    }
    if(noiseValue < progress)
    {
        clip(-1.0f);
        return;
    }
    return;
}

float ComputeDissolveEdgeIntensity(float noiseValue, float progress, float edgeWidth)
{
    if (edgeWidth <= 0.0f)
        return 0.0f;

    float t = noiseValue - progress;

    if(t >= 0 && t <= edgeWidth) {
        return saturate(1 - t / edgeWidth);
    }

    return 0.0f;
}
		```
	</details>
	<details>
	<summary>피해 처리와 사망 수명</summary>
		`src/Enemy.cpp` · 함수 발췌
		```c++
void Enemy::Update(double fixedDeltaSeconds) {
  if (lifeState_ == EnemyLifeState::Removed)
    return;
  if (lifeState_ == EnemyLifeState::Dying) {
    deathSeconds_ =
        std::min(kDissolveDurationSeconds, deathSeconds_ + fixedDeltaSeconds);
    if (deathSeconds_ + 1e-9 >= kDissolveDurationSeconds) {
      deathSeconds_ = kDissolveDurationSeconds;
      lifeState_ = EnemyLifeState::Removed;
    }
    return;
  }
  aliveSeconds_ += fixedDeltaSeconds;
  shotCooldownSeconds_ -= fixedDeltaSeconds;
  y_ = std::min(spawnY_ + 80.0f,
                y_ + movementSpeed_ * static_cast<float>(fixedDeltaSeconds));
  if (horizontalAmplitude_ > 0.0f) {
    x_ = spawnX_ + ComputeHorizontalOffset(aliveSeconds_, horizontalAmplitude_,
                                           horizontalPeriodSeconds_);
  }
}

bool Enemy::TryTakeDamage(int damage) noexcept {
  if (lifeState_ != EnemyLifeState::Alive || damage <= 0) {
    return false;
  }
  hp_ = std::max(0, hp_ - damage);
  if (hp_ == 0) {
    lifeState_ = EnemyLifeState::Dying;
    deathSeconds_ = 0.0;
  }
  return true;
}
		```
	</details>
</details>
---
# 06 · AI 리소스와 애니메이션 {color="green"}
<columns>
	<column ratio="33">
		<callout icon="🖼️" color="gray_bg">
			**이미지 생성**
			\[ComfyUI 원본 이미지\]
		</callout>
	</column>
	<column ratio="33">
		<callout icon="🎞️" color="gray_bg">
			**영상 생성**
			\[생성 영상 장면\]
		</callout>
	</column>
	<column ratio="34">
		<callout icon="🧩" color="gray_bg">
			**시트 변환**
			\[추출 프레임·최종 시트\]
		</callout>
	</column>
</columns>
*\[영상 · 원본 영상 / 게임 속 애니메이션\]* {color="gray"}
캐릭터 리소스는 제가 **ComfyUI로 이미지 생성 → 영상 생성 → 영상에서 프레임 추출 → 스프라이트 시트 변환** 과정을 거쳐 제작했습니다. 게임에서는 PNG 시트의 프레임별 UV를 계산해 재생하고, 왼쪽 이동 시트를 수평 반전해 오른쪽 이동에도 사용합니다.
<details>
<summary>**프레임 UV · 반전 코드 · 제작 설정**</summary>
	플레이어는 8열 2행의 대기/왼쪽 이동 시트, 적과 보스는 각각 8프레임 시트를 사용합니다. 열·행·시작 프레임·재생 속도를 데이터로 전달하고 개체별 시간을 기준으로 재생합니다.
	<details>
	<summary>재생 시간에서 프레임 UV 계산</summary>
		출처: `src/SpriteAnimation.cpp` · 실제 UV 계산 함수.
		```c++
std::array<float, 4>
CalculateSpriteAnimationUv(const SpriteAnimationData &animation,
                           double elapsedSeconds) {
  if (animation.columns == 0 || animation.rows == 0 ||
      animation.columns > std::numeric_limits<std::size_t>::max() /
                              animation.rows ||
      animation.frameCount == 0 ||
      !std::isfinite(animation.framesPerSecond) ||
      animation.framesPerSecond <= 0.0 || !std::isfinite(elapsedSeconds)) {
    throw std::invalid_argument("Invalid sprite animation dimensions or timing.");
  }
  const auto cellCount = animation.columns * animation.rows;
  if (animation.firstFrame >= cellCount ||
      animation.frameCount > cellCount - animation.firstFrame) {
    throw std::invalid_argument("Sprite animation frames exceed the sheet.");
  }

  const double elapsedFrames =
      std::floor(std::max(0.0, elapsedSeconds) * animation.framesPerSecond);
  if (!std::isfinite(elapsedFrames)) {
    throw std::invalid_argument("Sprite animation time is too large.");
  }
  const auto offset = static_cast<std::size_t>(
      animation.loop
          ? std::fmod(elapsedFrames, static_cast<double>(animation.frameCount))
          : std::min(elapsedFrames,
                     static_cast<double>(animation.frameCount - 1)));
  const auto frame = animation.firstFrame + offset;
  const auto column = frame % animation.columns;
  const auto row = frame / animation.columns;
  const float cellWidth = 1.0f / static_cast<float>(animation.columns);
  const float cellHeight = 1.0f / static_cast<float>(animation.rows);
  return {static_cast<float>(column) * cellWidth,
          static_cast<float>(row) * cellHeight,
          static_cast<float>(column + 1) * cellWidth,
          static_cast<float>(row + 1) * cellHeight};
}
		```
	</details>
	<details>
	<summary>프레임 안에서 UV 반전 적용</summary>
		`src/GameSpriteRenderer.cpp` · 함수 발췌
		```c++
void GameSpriteRenderer::DrawGameSprite(const SpriteDrawData &sprite,
                                        const SpriteAnimationData &animation,
                                        double time) const {
  const auto uv = CalculateSpriteAnimationUv(animation, time);
  const float width = uv[2] - uv[0];
  const float height = uv[3] - uv[1];
  auto frame = sprite;
  frame.uvRect = {
      uv[0] + sprite.uvRect[0] * width, uv[1] + sprite.uvRect[1] * height,
      uv[0] + sprite.uvRect[2] * width, uv[1] + sprite.uvRect[3] * height};
  DrawGameSprite(frame);
}
		```
	</details>
	\[실제 ComfyUI 워크플로·모델/버전·프레임 추출 및 시트 변환 설정\]
</details>
---
# 07 · 플레이 화면의 디테일 {color="green"}
<columns>
	<column ratio="34">
		<callout icon="💫" color="gray_bg">
			**탄환 형태와 후광**
			\[사진 · 보통탄 / 얇은 탄\]
			후광과 몸체를 분리해 표시
		</callout>
	</column>
	<column ratio="33">
		<callout icon="🌫️" color="gray_bg">
			**숲과 청록 안개**
			\[GIF · 배경 스크롤\]
			서로 다른 속도의 2레이어
		</callout>
	</column>
	<column ratio="33">
		<callout icon="❤️" color="gray_bg">
			**캐릭터 체력바**
			\[GIF · 이동과 HP 감소\]
			현재 위치 추종·왼쪽 끝 유지
		</callout>
	</column>
</columns>
<details>
<summary>**후광 · 배경 · HP바 · 오디오 코드**</summary>
	<details>
	<summary>탄환 후광의 거리 감쇠</summary>
		`shaders/Sprite.hlsl` · 함수 발췌
		```c++
float ComputeGlowIntensity(float2 localUV)
{
    float distance = length(localUV - float2(0.5, 0.5));
    float normalizedDistance = distance / kGlowRadius;

    return pow(saturate(1 - normalizedDistance), kGlowFalloffExponent);
}

float ComputeBulletCoreIntensity(float normalizedDistance)
{
    return saturate(1.0f - normalizedDistance);
}
		```
	</details>
	<details>
	<summary>2레이어 배경 반복</summary>
		`src/ScrollingBackground.cpp` · 함수 발췌
		```c++
float CalculateBackgroundScrollOffset(double elapsedSeconds, float speed,
                                      float tileHeight) {
  if (speed <= 0.0f || tileHeight <= 0.0f)
    return 0.0f;
  return static_cast<float>(std::fmod(std::max(0.0, elapsedSeconds) * speed,
                                      static_cast<double>(tileHeight)));
}

void ScrollingBackground::Render(const GameSpriteRenderer &renderer,
                                 double gameTimeSeconds) const {
  // 현재 bg0/bg1 원본의 실제 크기. 가로폭을 맞추고 원래 비율을 보존한다.
  constexpr float kSourceWidth = 941.0f;
  constexpr float kSourceHeight = 1672.0f;
  constexpr float kTileWidth = PlayfieldLayout::kWidth;
  constexpr float kTileHeight = kTileWidth * kSourceHeight / kSourceWidth;
  static_assert(kTileHeight >= PlayfieldLayout::kHeight);
  const auto drawLayer = [&](SpriteTextureId texture, float speed,
                             std::array<float, 4> tint) {
    const float offset =
        CalculateBackgroundScrollOffset(gameTimeSeconds, speed, kTileHeight);
    SpriteDrawData sprite{{kTileWidth * 0.5f, offset + kTileHeight * 0.5f},
                          {kTileWidth, kTileHeight},
                          texture,
                          tint};
    sprite.samplerMode = SpriteSamplerMode::Linear;
    renderer.DrawGameSprite(sprite);
    sprite.center[1] -= kTileHeight;
    renderer.DrawGameSprite(sprite);
  };
  drawLayer(SpriteTextureId::ForestBackground, 40.0f, {1.0f, 1.0f, 1.0f, 1.0f});
  drawLayer(SpriteTextureId::FogBackground, 15.0f, {1.0f, 1.0f, 1.0f, 0.16f});
}
		```
	</details>
	<details>
	<summary>체력바의 위치와 폭 보정</summary>
		`src/HealthBar.cpp` · 함수 발췌
		```c++
void HealthBar::SetPosition(std::array<float, 2> position) {
  barBackgroundSprite_.center = position;
  barSprite_.center = position;
  UpdateSprites();
}

void HealthBar::UpdateSprites() {
  barSprite_.size[0] = value_ * barBackgroundSprite_.size[0];
  barSprite_.center[0] =
      barBackgroundSprite_.center[0] -
      (barBackgroundSprite_.size[0] - barSprite_.size[0]) / 2.f;
}
		```
	</details>
	발사음은 PCM을 공유하고 SourceVoice별 재생 상태를 관리합니다. BGM은 한 Voice에서 반복합니다.
	<details>
	<summary>발사음과 BGM 재생</summary>
		`src/AudioSystem.cpp` · 함수 발췌
		```c++
void AudioSystem::PlayShot() {
  for (auto *voice : shotVoices_) {
    XAUDIO2_VOICE_STATE state{};
    voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
    if (state.BuffersQueued != 0) {
      continue;
    }

    XAUDIO2_BUFFER buffer{};
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = static_cast<UINT32>(shotClip_.pcm.size());
    buffer.pAudioData = shotClip_.pcm.data();
    ThrowIfFailed(voice->SubmitSourceBuffer(&buffer),
                  "Failed to submit the shot audio buffer.");
    return;
  }
}

void AudioSystem::PlayBgm() {
  if (bgmPlaying_) {
    return;
  }

  XAUDIO2_BUFFER buffer{};
  buffer.Flags = XAUDIO2_END_OF_STREAM;
  buffer.AudioBytes = static_cast<UINT32>(bgmClip_.pcm.size());
  buffer.pAudioData = bgmClip_.pcm.data();
  buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

  ThrowIfFailed(bgmVoice_->SubmitSourceBuffer(&buffer),
                "Failed to submit the BGM audio buffer.");
  ThrowIfFailed(bgmVoice_->Start(), "Failed to start the BGM source voice.");
  bgmPlaying_ = true;
}
		```
	</details>
</details>
<details>
<summary>**입력 오류 수정 · 리소스 소유권 · 자동 검증**</summary>
	이동 키를 누른 채 최소화한 뒤 복귀하면 키 상태가 남는 문제가 있었습니다. 포커스 상실 시 현재·이전 키 상태를 초기화했습니다. 지속 입력과 한 번의 입력을 구분하고 게임 업데이트는 60Hz 고정 간격으로 실행합니다.
	\[최소화·복귀 후 입력 확인 영상\]
	<details>
	<summary>입력 초기화와 고정 업데이트</summary>
		`src/Application.cpp` · 함수 발췌
		```c++
void Application::HandlePendingInput() {
  if (window_.ConsumeFocusLost()) {
    input_.Reset();
  }
  for (const auto &event : window_.ConsumeKeyEvents()) {
    input_.SetKeyDown(event.virtualKey, event.isDown);
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
		```
	</details>
	게임 객체는 표시 데이터를 전달하고 TextureCache가 SRV를 소유합니다. 전투 좌표는 720×960으로 유지하고 화면에 출력할 때 창 크기에 맞춥니다.
	\[데이터 전달과 GPU 리소스 소유권 도식\]
	<details>
	<summary>지연 로딩과 SRV 재사용</summary>
		`src/TextureCache.cpp` · 함수 발췌
		```c++
ID3D11ShaderResourceView *TextureCache::Get(SpriteTextureId id) {
  const auto index = static_cast<std::size_t>(id);
  if (index >= textures_.size()) {
    throw std::invalid_argument("Invalid sprite texture ID.");
  }

  if (!textures_[index]) {
    textures_[index] = LoadTextureFromFile(GetTexturePath(id));
  }

  return textures_[index].Get();
}
		```
	</details>
	<details>
	<summary>논리 전투 좌표의 화면 변환</summary>
		출처: `src/PlayfieldLayout.h`
		```c++
#include "SpriteDrawData.h"
#include <algorithm>
#include <array>

// 전투 영역의 논리 크기와 화면 배치. GPU 리소스나 게임 상태는 소유하지 않는다.
struct PlayfieldLayout {
  static constexpr float kWidth = 720.0f;
  static constexpr float kHeight = 960.0f;
  float left = 0.0f;
  float top = 0.0f;
  float scale = 1.0f;

  [[nodiscard]] static PlayfieldLayout FromClientSize(float width,
                                                      float height) {
    const float scale = std::min(width / 1920.0f, height / 1080.0f);
    return {(width - kWidth * scale) * 0.5f, (height - kHeight * scale) * 0.5f,
            scale};
  }
  [[nodiscard]] float Right() const { return left + kWidth * scale; }
  [[nodiscard]] float Bottom() const { return top + kHeight * scale; }
  [[nodiscard]] SpriteDrawData ToScreen(const SpriteDrawData &sprite) const {
    auto result = sprite;
    result.center = {left + sprite.center[0] * scale,
                     top + sprite.center[1] * scale};
    result.size = {sprite.size[0] * scale, sprite.size[1] * scale};
    return result;
  }
};
		```
	</details>
	직전 Debug·Release 자동 검사를 통과했습니다. 스폰 간격의 빈 구간, 사망 연출과 전환, 중복 점수, 재시작, 충돌 경로의 판정 동등성을 확인합니다. 가독성·밸런스·사운드 청취는 실제 플레이로 따로 확인합니다.
	<details>
	<summary>예정 스폰이 남은 구간의 검사</summary>
		출처: `src/GameSceneStageSmokeTest.cpp` · 테스트 함수 일부.
		```c++
while (minions_.front().GetHp() > 0)
  (void)minions_.front().TryTakeDamage();
UpdateMinions(0.8);
UpdateStageProgress();
require(stage_.GetStageNumber() == 1 && minions_.empty(),
        "An empty gap advanced before the scheduled spawns finished.");
		```
	</details>
	\[최종 커밋의 빌드·검사 결과\]
	\[최종 전체 플레이 영상\]
</details>
<details>
<summary>**파티클 · 아이템과 UI**</summary>
	### 파티클
	\[피격·소환·이동 효과 전·후 GIF\]
	\[섬광·입자·잔상의 적용 이유와 탄막 가독성·렌더 비용 비교\]
	<details>
	<summary>파티클 코드</summary>
		\[생성 이벤트·수명·Update/Render 코드\]
		\[최대 활성 수·비용·종료와 재시작 확인\]
	</details>
	### 강화 아이템과 UI 마감
	\[드롭 → 습득 → 강화 발사 GIF\]
	\[타이틀 / 전투 HUD / 결과 화면\]
	<details>
	<summary>아이템·UI 코드</summary>
		\[드롭·습득·발사 강화와 초기화 코드\]
		\[화면별 레이아웃·입력·상태 전환 코드\]
	</details>
</details>
<details>
<summary><span color="gray">작업 범위와 리소스 출처</span></summary>
	개인 학습 프로젝트로 핵심 과제는 직접 구현하고 AI의 설명·리뷰·구현 보조를 사용했습니다. 반복 설정과 구조 분리, 최근 3스테이지 연결과 F5 비교 경로는 AI 보조 구현에 포함됩니다.
	\[최종 기능별 직접 구현 / 보조 구현 / 직접 분석·수정·검증 범위\]
	캐릭터 이미지·영상·시트는 제가 ComfyUI로 제작했습니다. BGM은 [Fairy Battles · MintoDog · CC0](https://opengameart.org/content/fairy-battles)를 PCM WAV로 변환해 사용했습니다.
	\[나머지 리소스 출처·이용 조건\]
	\[원고·소스·실행 파일·영상의 최종 기준 커밋\]
</details>
