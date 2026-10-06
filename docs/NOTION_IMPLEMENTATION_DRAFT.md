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
		
		```cpp
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
		
		```cpp
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
		
		```cpp
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

# 02 · 대량 탄막 렌더링 {color="green"}

<columns>
	<column ratio="50">
		<callout icon="◀️" color="gray_bg">
			<span color="gray">**BEFORE · 개별 렌더링**</span>
			\[영상 · 변경 전\]
		</callout>
	</column>
	<column ratio="50">
		<callout icon="▶️" color="gray_bg">
			**AFTER · 렌더 제출 개선**
			\[영상 · 변경 후\]
		</callout>
	</column>
</columns>

현재 탄환은 후광과 몸체를 각각 그립니다. 이 경로에서는 **한 발당 Draw 2회, 상수 버퍼 갱신 4회**가 발생합니다. \[병목 구간 → 렌더 제출 방식 변경 → 같은 장면의 측정 결과\]

같은 장면에서 **CPU 제출 시간과 GPU 렌더 시간을 따로 비교**합니다.

<table fit-page-width="true" header-row="true">
<tr color="gray_bg"><td>동일 장면 비교</td><td>변경 전</td><td>변경 후</td></tr>
<tr><td>CPU 렌더 제출</td><td>\[ms\]</td><td>\[ms\]</td></tr>
<tr><td>GPU 렌더 시간</td><td>\[ms\]</td><td>\[ms\]</td></tr>
<tr><td>프레임당 Draw</td><td>\[횟수\]</td><td>\[횟수\]</td></tr>
</table>

<columns>
	<column ratio="50">
		*\[사진 · 변경 전 프로파일\]* {color="gray"}
	</column>
	<column ratio="50">
		*\[사진 · 변경 후 프로파일·그래프\]* {color="gray"}
	</column>
</columns>

<details>
<summary>**렌더 경로 · 선택 근거 · 측정 조건**</summary>
	현재는 공유 쿼드를 사용하지만 스프라이트마다 상수 버퍼를 갱신하고 DrawIndexed를 호출합니다. 후광이 많이 겹치는 장면의 GPU 비용도 별도로 확인합니다.
	
	<details>
	<summary>현재 개별 렌더링과 상수 버퍼 갱신</summary>
		`src/Graphics.cpp` · 함수 발췌
		
		```cpp
		void Graphics::DrawSprite(const SpriteDrawData &sprite) {
		  BindSpriteTexture(sprite.texture);
		  BindSpriteSampler(sprite.samplerMode, sprite.addressMode);
		  SetSpriteTransform(sprite.center[0], sprite.center[1], sprite.size[0],
		                     sprite.size[1], sprite.rotationRadians);
		  UpdateSpriteConstants();
		  spriteTintConstants_.tint = sprite.tint;
		  spriteTintConstants_.timeSource = static_cast<float>(sprite.timeSource);
		  spriteTintConstants_.shape = static_cast<float>(sprite.shape);
		  spriteTintConstants_.uvRect = sprite.uvRect;
		  spriteTintConstants_.dissolveProgress = sprite.dissolveProgress;
		  spriteTintConstants_.dissolveNoiseScale = sprite.dissolveNoiseScale;
		  spriteTintConstants_.dissolveEdgeWidth = sprite.dissolveEdgeWidth;
		  spriteTintConstants_.dissolveEdgeStrength = sprite.dissolveEdgeStrength;
		  spriteTintConstants_.dissolveEdgeColor = sprite.dissolveEdgeColor;
		  UpdateSpriteTintConstants();
		  BindBlendState(sprite.blendMode);
		
		  context_->DrawIndexed(6, 0, 0);
		}
		
		void Graphics::UpdateSpriteConstants() {
		  D3D11_MAPPED_SUBRESOURCE mapped{};
		  ThrowIfFailed(context_->Map(spriteConstantBuffer_.Get(), 0,
		                              D3D11_MAP_WRITE_DISCARD, 0, &mapped),
		                "Failed to map the sprite constant buffer.");
		  *static_cast<SpriteConstants *>(mapped.pData) = spriteConstants_;
		  context_->Unmap(spriteConstantBuffer_.Get(), 0);
		}
		
		void Graphics::UpdateSpriteTintConstants() {
		  D3D11_MAPPED_SUBRESOURCE mapped{};
		
		  ThrowIfFailed(context_->Map(spriteTintConstantBuffer_.Get(), 0,
		                              D3D11_MAP_WRITE_DISCARD, 0, &mapped),
		                "Failed to map sprite tint constant buffer.");
		
		  *static_cast<SpriteTintConstants *>(mapped.pData) = spriteTintConstants_;
		
		  context_->Unmap(spriteTintConstantBuffer_.Get(), 0);
		}
		```
	</details>
	
	<details>
	<summary>개선 코드</summary>
		\[채택한 배치/인스턴싱 코드와 HLSL 입력\]
		\[출력 순서·블렌딩·버퍼 용량 처리\]
	</details>
	
	\[기준 커밋·재현 장면·활성 탄환 수·장비·Release 설정·해상도·계측 도구·반복 조건\]
	\[프레임 시간 중앙값/P95·업로드 횟수·동일 출력 확인\]
	\[실측 결과·채택 이유·남은 병목\]
</details>

---

# 03 · 충돌 검사 비교 {color="green"}

<columns>
	<column ratio="50">
		<callout icon="🔍" color="gray_bg">
			**전체 순회**
			\[영상 · 활성 탄환과 후보 수\]
		</callout>
	</column>
	<column ratio="50">
		<callout icon="▦" color="gray_bg">
			**UNIFORM GRID**
			\[영상 · 같은 배치의 후보 수\]
		</callout>
	</column>
</columns>

F5로 전체 순회와 Grid를 전환할 수 있습니다. <span color="green">**후보 선택만 바꾸고 명중·무적·탄환 소비·Graze 처리는 공유**</span>해 판정 차이를 줄였습니다. 성능 비교는 **Grid 구축 비용까지 포함**합니다.

<details>
<summary>**후보 선택 · 공통 판정 · 비교 코드**</summary>
	전체 순회 모드에서는 매 틱 Grid 구축을 생략합니다. Grid 모드는 플레이어 주변 3×3 셀을 조회합니다. 같은 탄환의 Graze 점수는 한 번만 반영합니다.
	
	<details>
	<summary>공통 판정과 후보 선택</summary>
		`src/GameScene.cpp` · 함수 발췌
		
		```cpp
		void GameScene::CheckPlayerEnemyBulletCollisions() {
		  constexpr float kPlayerHitRadius = 5.0f;
		  constexpr float kPlayerGrazeRadius = 24.0f;
		
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
		
		  auto &bullets = bulletSystem_.GetBullets();
		  // 두 경로는 후보 선택만 다르고 명중/무적/Graze 처리는 공유한다.
		  const auto checkBullet = [&](Bullet &bullet) {
		    if (!bullet.active || bullet.owner != BulletOwner::Enemy)
		      return;
		    const CircleHitbox bulletHitbox{bullet.x, bullet.y,
		                                    GetBulletStyle(bullet.type).hitRadius};
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
		      ++grazeCount_;
		      score_ += kGrazeScore;
		    }
		  };
		
		  if (collisionMode_ == CollisionMode::LinearScan) {
		    activeEnemyBulletCount_ = 0;
		    for (auto &bullet : bullets) {
		      if (bullet.active && bullet.owner == BulletOwner::Enemy)
		        ++activeEnemyBulletCount_;
		      checkBullet(bullet);
		    }
		    return;
		  }
		
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
		
		  for (std::size_t cellY = minCellY; cellY <= maxCellY; ++cellY) {
		    for (std::size_t cellX = minCellX; cellX <= maxCellX; ++cellX) {
		      for (const std::size_t bulletIndex :
		           enemyBulletGrid_.GetCell(cellX, cellY)) {
		        checkBullet(bullets[bulletIndex]);
		      }
		    }
		  }
		}
		
		void GameScene::SetCollisionMode(CollisionMode mode) {
		  if (collisionMode_ == mode)
		    return;
		  collisionMode_ = mode;
		  enemyBulletGrid_.Clear();
		  collisionCandidateCount_ = 0;
		}
		```
	</details>
	
	<details>
	<summary>Grid 구축과 전체 순회 분기</summary>
		`src/GameScene.cpp` · 함수 발췌
		
		```cpp
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
	
	\[소수 / 분산 / 밀집 배치 사진\]
	\[배치별 전체 순회 시간과 Grid 구축+조회 시간 비교\]
	
	<details>
	<summary>실전 계측 코드</summary>
		\[재현 배치·구간 측정·집계 코드\]
		\[같은 배치의 HP·점수·Graze 결과 확인\]
	</details>
	
	현재 셀 크기와 판정 반경에 맞춰 조회 범위를 정했습니다. 탄환의 이동 경로를 추적하는 CCD는 구현하지 않았습니다.
</details>

---

# 04 · 디졸브와 사망 연출 {color="green"}

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
		
		```cpp
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
		
		```cpp
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

# 05 · AI 리소스와 애니메이션 {color="green"}

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
		
		```cpp
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
		
		```cpp
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

# 06 · 플레이 화면의 디테일 {color="green"}

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
		
		```cpp
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
		
		```cpp
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
		
		```cpp
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
		
		```cpp
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
		
		```cpp
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
		
		```cpp
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
		
		```cpp
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
		
		```cpp
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
<summary>**탄환 관리 · 파티클 · 아이템과 UI**</summary>
	### 탄환 자료구조
	현재는 8,192개 슬롯을 미리 확보하고 비활성 탄환을 재사용합니다. 빈 슬롯 탐색과 전체 슬롯 순회 비용은 남아 있습니다.
	
	<details>
	<summary>현재 Spawn과 Update</summary>
		`src/BulletSystem.cpp` · 함수 발췌
		
		```cpp
		void BulletSystem::Spawn(float centerX, float centerY, float velocityX,
		                         float velocityY, BulletOwner owner, BulletType type) {
		  ++spawnRequests_;
		  auto target = std::ranges::find(bullets_, false, &Bullet::active);
		  if (target != bullets_.end()) {
		    const auto style = GetBulletStyle(type);
		    *target = Bullet{centerX,      centerY, velocityX, velocityY, style.width,
		                     style.height, owner,   false,     true,      type};
		  } else {
		    ++droppedSpawnRequests_;
		  }
		}
		
		void BulletSystem::Update(double fixedDeltaSeconds) {
		  for (auto &bullet : bullets_) {
		    if (!bullet.active) {
		      continue;
		    }
		    bullet.x += bullet.velocityX * static_cast<float>(fixedDeltaSeconds);
		    bullet.y += bullet.velocityY * static_cast<float>(fixedDeltaSeconds);
		  }
		}
		```
	</details>
	
	\[자료구조 변경 전·후 계측\]
	<details>
	<summary>자료구조 개선 코드</summary>
		\[빈 슬롯·활성 탄환 관리 코드와 제거/재시작 검증\]
		\[실측 비용과 자료구조 선택 이유\]
	</details>
	
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
