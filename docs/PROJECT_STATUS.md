# Project Status

> 이 파일이 현재 진행 상황의 단일 진실 원본이다. 새 세션/새 에이전트는 가장 먼저 이 파일을 읽는다.

## 현재 요약

- 프로젝트: Danmaku Lab (실행 파일/빌드 대상: DanmakuShooter)
- 목적: 게임회사 클라이언트/게임 프로그래머 취업용 포트폴리오
- 현재 단계: **구조 정리 — GameScene / GameHud 책임 분리**
- 상태: **사용자 요청으로 코드 이동 완료 / Debug 빌드 및 자동 검사 통과**
- 다음 단계: 구조 변경 후 기존 조작·애니메이션·사망 연출을 화면 확인하고17E 탄막 효과를 보강한다. 다중 적/스테이지/패턴/UI 및 렌더 성능 개선 범위는 유지한다.

## 최신 진행 — 2026-10-05

- 사용자 요청으로 Application의 전투 및 표시 책임을 분리했다. Application은 창·루프·입력 전달·일시정지·화면 전환·오디오를 소유한다. GameScene은 Player/Enemy/탄환/Grid와 전투 시간·승패·사망 연출을 소유하고, 발사 여부와 전투 결과를 Application에 전달한다.
- GameHud는 체력바와 화면 UI를 구성하며 const GameScene에서 표시할 값을 읽는다. GameSpriteRenderer는 Graphics를 빌려 시트 UV 계산과 그리기를 연결하고, PlayfieldLayout은720×960 논리 전투 좌표를 화면 좌표로 변환한다. GPU 리소스 수명은 기존 Graphics/캐시에 유지한다. 별도 Scene 상속 체계는 추가하지 않았다.
- 사용자 플레이어 시트는8열×2행, Idle 첫 행/Left 둘째 행/Right 둘째 행 UV 반전으로 유지했다. 행 번호를 동작 상태처럼 쓰던 부분을 PlayerMotion enum으로 표현하고 렌더 시 행과 반전을 선택한다. 입력 동작과 재생 시간 정책은 확장하지 않았다.
- 현재 핵심: 파일 수보다 데이터의 소유권과 변경 책임이 중요하다. 전투 판정은 GameScene, 화면 전환과 소리는 Application, 표시용 읽기는 GameHud가 담당한다.
- 현재 과제: 새 코딩 과제 없음(기계적 이동은 사용자 위임). 실행해 좌우 이동·8프레임 애니메이션·오른쪽 반전·HP바·사망 디졸브·P 일시정지·재시작을 확인한다.
- 완료 조건: 구조 변경 전의 조작과 전투/표시 동작을 유지하며 Application에 전투 데이터와 충돌 계산이 남아 있지 않음.
- 마지막 검증: MSVC Debug 빌드 성공. 최종 --smoke-test 종료0. 전투 영역 축소/좌표 변환, 시트 프레임, 충돌·무적, 단독/동시 사망, Ending 시간/정지, 화면 전환, 재시작, 입력 전달과 발사를 검사했다. git diff --check 통과. 실제 화면의 최종 육안 확인은 사용자 대기.

- 17F-2 사용자 구현 소스 리뷰 통과: t>=0 및 t<=edgeWidth 구간에서 saturate(1-t/edgeWidth), 나머지0으로 경계 발광 계산 정상. 사용자 다음 단계 승인 후17F-3 사망 연결을 에이전트가 완성했다.
- 상태 흐름은 Title→Playing→Ending→Result→Title. HP0에서 승패를 고정하고 Ending 진입, 즉시 BGM 정지. Ending에서는 이동/발사/탄환/충돌/게임시간과 시트 포즈를 정지하고 사망 연출 시간만 fixed update로 증가한다.
- 죽은 Player/Enemy는 Ending 동안 렌더하며 dissolveProgress=clamp(연출경과/0.8,0,1). 생존 캐릭터는 디졸브0. 플레이어 사망은 피격 무적 깜빡임을 적용하지 않는다. HP0 체력바는 숨긴다. 0.8초 후Result에서 죽은 캐릭터를 더 그리지 않는다. 동시 사망은 둘 다 디졸브하고 기존 실패 우선.
- P는Ending에서도 일시정지/재개 가능. 최소화는 기존 Run 대기로 연출도 정지한다. Enter는Ending을 건너뛰지 않는다. F3는Playing에서만 미리보기를 바꾸며 실제 사망 진행도를 덮어쓰지 않는다. 새 Playing에서Ending 시간과F3값을0으로 리셋.
- 현재 과제: 코딩 과제 없음(이미 설명한 상태 전환/시간 연결을 사용자 요청으로 위임). 적 사망→0.8초 소멸→CLEAR, 플레이어 사망→소멸→FAILED, P 일시정지/재개 및 재시작 초기화를 실행 확인한다. 다음은 확인 뒤 탄막 이펙트 보강. 다중 적 일반 사망은 전체 Ending 상태와 별도 객체 연출 수명이 필요하므로17B에서 확장한다.
- 마지막 검증: MSVC Debug 빌드 성공, --smoke-test 종료0. 적 단독/플레이어 단독/동시 사망,0.4초 진행도0.5, 살아남은 캐릭터/탄환/게임시간 정지,Ending pause,0.8초 뒤Result,재시작 연출 초기화를 검사했다. 중간/종료 렌더 및 HLSL 컴파일도 수행. 실제 화면의 연출 체감은 사용자 확인 대기.

- 17F-1 사용자 ApplyDissolve 구현 소스 확인: progress0 유지/1 전체clip/중간noiseValue<progress clip 정상. 사용자 F3 진행도별 소멸 확인 보고로 해당 Step 완료.
- 17F-2 주변 코드 완료: SpriteDrawData의 dissolveEdgeWidth0.08/Strength2/Color청록, Graphics b1 CPU/HLSL80바이트 일치 및 전송. 제거 후 남은 픽셀에서 같은 noiseValue를 재사용하고 ComputeDissolveEdgeIntensity 반환값으로 RGB에 발광 색을 더한다. 원본 alpha/PNG 투명도는 변경하지 않는다.
- 현재 핵심: 경계까지의 차이는 noiseValue-progress라는 노이즈 값 공간에서의 차이이며 화면 픽셀 거리가 아니다. 살아남은 쪽의 좁은 구간에만 빛을 주고, 경계에서1→edgeWidth만큼 떨어진 곳에서0이 되도록 감쇠한다. 주변 공간으로 번지는 Bloom은 아직 구현하지 않는다.
- 현재 과제: shaders/Sprite.hlsl::ComputeDissolveEdgeIntensity TODO에0~1 빛 세기를 반환한다. 진행도0.5/폭0.08에서 노이즈0.50은1,0.54는중간,0.58이상은0이 목표. 이미 학습한 saturate 또는 smoothstep으로 구현 가능. 반환값은float이며 실제 RGB 합성은 에이전트가 연결했다.
- 완료 조건: F3의 중간 진행도에서 소멸 경계만 청록으로 밝아지고 남은 내부는 원색, 진행도0은 원래 이미지/1은 완전숨김, 투명 셀 배경에 사각형 빛이 생기지 않음. 현재return0 placeholder이므로 사용자 구현 전에는 경계 빛이 보이지 않는다.
- 마지막 검증: MSVC Debug 빌드 성공, --smoke-test 종료0. HLSL 런타임 컴파일 및 디졸브 진행도0.5/1 렌더 경로/기존 전투 검사 통과. 빛 세기 과제 미구현이며 시각적 발광 확인은 사용자 구현 후다. 다음은 경계 발광 리뷰 후 사망 연출 수명 연결.

- 사용자 요청: 적합한 리소스 제작이 어려워 아트 제작을 잠시 두고 디졸브와 탄막 시각 효과부터 진행한다. 한 번에 한 Step 원칙에 따라17F-1 디졸브 픽셀 제거부터 시작했다. 기존 플레이어4×3/적4×2 시트 및 사용자 입력 연결 수정은 유지했다.
- 주변 코드 완료: SpriteDrawData의 dissolveProgress(기본0)/dissolveNoiseScale(기본14), Graphics b1 상수 버퍼64바이트 CPU/HLSL 일치, Draw마다 전달. 로컬UV 기반 절차적 부드러운 노이즈를 완성해 새 이미지 제작 없이 실습할 수 있다. 실제 GPU 노이즈 비용은 후속 성능 측정 대상이다.
- F3 미리보기는 플레이어와 적에 진행도0/.25/.5/.75/1을 순환 적용한다. HP/충돌/사망/승패에 영향을 주지 않으며 새 Playing에서0으로 초기화한다. 노이즈는 시간이나 시트 프레임UV를 사용하지 않아 패턴이 재생 중 바뀌지 않는다.
- 현재 과제: shaders/Sprite.hlsl::ApplyDissolve TODO에서 중간 진행도의 노이즈 기준 픽셀 제거 구현. clip(x)는 음수이면 해당 픽셀을 버린다는 API 설명 제공. 진행도0 전체 유지/1 전체 제거 처리는 주변 코드가 완료했다.
- 완료 조건: F3의 중간 진행도에서 점점 더 많은 불규칙 영역이 사라지고, 진행도1은 완전히 숨김/다음0은 복귀, 원래 PNG 투명 배경 유지. 사용자 구현 전에는 중간 진행도가 무동작이고1만 숨겨지는 것이 예상 동작이다. 과제 정답을 주석에 제공하지 않았다.
- 다음 Step: 결과 리뷰 후 디졸브 경계 발광. 실제 사망에서는 HP0 판정과 Sprite 연출 수명을 분리해야 하며 아직 연결하지 않았다. 탄막 코어/후광/꼬리/명중 파티클은 이후 한 Step씩 다룬다.
- 마지막 검증: MSVC Debug 빌드 성공, --smoke-test 종료0로 HLSL 런타임 컴파일과 기존 전투 검사 통과. TODO 핵심 기능은 미구현 상태이며 픽셀 결과 육안 검증은 하지 않았다.

- 사용자 Enemy 텍스처 추가 후 종료 원인: SpriteTextureId::Enemy=4인데 TextureCache 배열은4개(유효0~3)여서 Invalid sprite texture ID 예외. 수정 전 --smoke-test 종료1로 재현했다. enum 마지막 Count를 추가하고 캐시 배열 크기를 Count에서 계산하도록 수정했다.
- 현재 Enemy 렌더는 애니메이션 데이터를 생성하지만 DrawGameSprite(enemySprite)만 호출해 재생하지 않는다. firstFrame에 플레이어 행을 사용해 오른쪽 입력에서는8부터 시작하므로4×2 시트 범위를 넘는다. 애니메이션 연결 시 별도 수정 필요. 이번에는 종료 원인인 캐시만 고쳤으며 Enemy 표시/입력 동작은 변경하지 않았다.
- 사용자 지적 반영: 이전 플레이어 입력별 행 선택은 에이전트가 요청 범위를 넘어 추가한 것이므로 이후 입력/상태 연동을 임의로 확장하지 않는다. 기존 코드 제거 요청은 아직 없어 현재 연동은 유지했다.
- 마지막 검증: MSVC Debug 빌드 성공、수정 후 --smoke-test 종료0. Enemy 파일 로딩/렌더 및 기존 전투 검사 통과. 현재 Step17A 및 다음 적 시트 적용/화면 확인 유지.

- 사용자 명시 구현 요청으로 SpriteDrawData.h 옆에 SpriteAnimationData 구조체를 추가했다(columns/rows/firstFrame/frameCount/framesPerSecond/loop). 재생 상태와 GPU 리소스를 소유하지 않는다. SpriteAnimation.cpp의 순수 계산 함수가 지정한 시간에서 UV를 계산한다.
- DrawGameSprite(sprite, animation)는 SpriteTimeSource에 맞는 게임/실제 시간을 선택한다. DrawGameSprite(sprite, animation, animationTimeSeconds)는 개별 재생 경과 시간을 직접 받을 수 있다. 선택한 셀 안에 기존 sprite.uvRect를 적용한 뒤 기존 좌표 변환/Draw 경로를 재사용한다.
- 사용자 player.png의6열×3행에 적용: 기본/왼쪽/오른쪽 행, 각6프레임,12fps 반복(0.5초 주기). fixed Update에서 수평 입력으로 행을 선택하며 새 게임에서 기본 행으로 리셋. 게임 시간 사용으로 pause/Result에서 시간과 선택 행이 멈춘다. 방향 전환 시 프레임은 공통 게임 시간 기준이며0번부터 재시작하지 않는다.
- 이번 과제: 새 API 코딩 과제 없음(사용자 구현 위임). 기본/좌/우 애니메이션, P 정지/재개, 재시작 첫 프레임 표시를 실행 확인한다. 이미지 자체의 셀별 중심 흔들림은 데이터 계산과 별도로 제작 정렬을 확인한다. 다음 Step은 이 적용 확인 후 선택한다.
- 마지막 검증: MSVC Debug 빌드 성공, --smoke-test 종료0. 행/프레임 선택,0.5초 반복, 비반복 마지막 프레임 유지, 행 경계 진행, 잘못된0열 거부 및 실제 세 행 렌더 호출/기존 전투 검사 통과. 게임 실행 파일 잠금으로 한 차례 링크 실패해 해당 프로젝트 프로세스를 종료 후 빌드했다. 사용자 화면의 애니메이션 육안 확인은 대기.

- 사용자 새 플레이어 이미지 적용 후 종료 문제 해결: TextureCache는 assets/player.png를 찾도록 변경됐으나 CMake CopyAssets는 기존 player_test.png/WAV만 복사해 실행 파일 옆에 player.png가 없었다. 수정 전 --smoke-test 종료1 및 WIC 파일 열기 오류 재현.
- CopyAssets를 assets 디렉터리 전체 복사로 변경해 새 파일도 빌드마다 포함한다. 사용자 player.png와 로더 경로를 유지했다. 원본/실행 폴더 이미지 SHA256 동일 확인.
- 마지막 검증: MSVC Debug 빌드 성공, 수정 후 --smoke-test 종료0로 새 PNG 로딩/렌더 및 기존 전투 검사 통과. 시트 프레임 선택/애니메이션은 아직 구현하지 않아 이 검사는 전체 텍스처 로딩 확인이다. 현재 Step17A/다음 첫 시트 적용 유지.

- 세로 전투 영역 설정 완료: 전체 창 기본1920×1080, 논리 전투 영역720×960(3:4), 기본 위치(600,60). 창 크기 변경 시 가운데 배치와 동일 배율 확대/축소를 적용한다. 스테이지 배경 규격도3:4로 정정했다.
- Application::DrawGameSprite에서 전투 좌표를 화면 좌표로 변환한다. 이동 제한/탄환 반환/충돌/Grid는720×960 기준으로 유지하고, 플레이어(360,800)/보스(360,140)로 배치했다. Grid는12×15이며 창 Resize로 게임 영역을 바꾸지 않는다.
- 주변4개 불투명 패널로 경계 밖 탄환/Glow를 가리고 HUD는 오른쪽 공간에 배치했다. 캐릭터/탄환 표시 크기는 기존값이며 사용자 스프라이트 적용 때 조정한다.
- 마지막 검증: MSVC Debug 빌드/링크 성공, --smoke-test 종료0. 기존 충돌/무적/재시작/승패/렌더 검사 및 추가 플레이어 전투 영역 제한/측면 탄환 반환 검사 통과. 실제 창 확대/축소 화면의 육안 검증은 사용자 확인 대기다.
- 현재 Step17A 유지. 현재 과제: 화면의 세로 전투 영역과 이동 제한/창 축소 시 비율 유지 확인 및 사용자 이미지 제작. 다음은 첫 스프라이트 시트 적용이다.

- 사용자 요청으로 Window 기본 client 크기를1280×720에서1920×1080(Full HD)으로 변경했다. 제목 표시줄/테두리는 기존 AdjustWindowRect로 추가된다. ASSET_PLAN의 전체 화면 배경 규격과 D047도 갱신했다.
- 마지막 검증: MSVC Debug 빌드/링크 성공. 실제 창 크기/화면 배치는 이번 턴에서 실행 확인하지 않았다. 현재 Step17A 및 이미지 제작 과제는 유지한다.

- 사용자 수정: 이미지는 사용자가 직접 제작하며 타일 형태의 스프라이트 시트 애니메이션을 지원하도록 준비한다. 에이전트는 추가 이미지를 생성하지 않는다. 이전 시안 검토 과제는 취소하고 필요한 자산 목록/규격 산정으로 전환했다.
- 현재 핵심: 해상도와 캐릭터 표시 크기를 함께 높일 수 있으며 기존 48×48은 최종 제약이 아니다. 스프라이트 시트는 기존 uvRect로 한 프레임을 선택할 수 있지만 프레임 시간/상태 전환은 후속 구현이 필요하다.
- 현재 계획 초안: 플레이어1, 일반 적3종, 보스2종, 스테이지2개를 가정한 21개 PNG/애니메이션 프레임 및 정지 그림145개. 확정 콘텐츠 수가 아닌 제작량 추정이며 세부 목록은 docs/ASSET_PLAN.md 참조.
- 현재 과제: 코딩 과제 없음. 사용자가 목록을 바탕으로 제작 범위를 정하고 첫 플레이어 시트를 준비한다. 완료 조건: 셀 크기/배열/프레임 순서/중심 기준이 명확한 첫 시트 확보. 다음: 시트 재생 개념 설명 및 주변 코드 준비 후 핵심 프레임 선택 과제. 현재는 이미지 제작 목록만 정리한다.
- 마지막 검증: SpriteDrawData의 기존 uvRect 및 Player 표시 크기 소스 확인. 문서만 변경했으며 새 빌드/실행 검증은 수행하지 않았다. 아래 이전 시안 과제/진행은 이력이다.

- 사용자 명시 범위 확장: 대량 렌더링 전에 정교한 플레이어/적 텍스처, 실제 게임의 스테이지 진행, 여러 적/패턴/보스, 정교한 탄환 및 파티클 이펙트, 플레이어/Enemy 디졸브, 타이틀/최종 화면 UI 디자인 강화를 완료한다. 이후 렌더링 및 성능 개선. 이는 이전 작은 게임 뒤 바로 최적화/3D로 가던 일정에 우선한다.
- 현재 단일 보스 패키지는 첫 완성본으로 보존한다. 최종 확장 프로젝트 완성이라고 표기하지 않는다. ROADMAP에17A~17H를 추가했고 다음 Step은 아트 방향 및 텍스처 적용 준비다.
- 사용자 아트 방향 확정: 캐릭터 중심 판타지. built-in imagegen으로 플레이어 마법사 투명 PNG 첫 시안을 생성해 assets/concepts/player_mage_v1.png에 보관했다. 기존 플레이어 텍스처는 아직 교체하지 않았다. 상세 프롬프트와 시안 한계는 docs/ART_DIRECTION.md 참조.
- 현재 핵심: 원본 이미지 해상도와 SpriteDrawData의 화면 표시 크기는 별개다. 큰 원화도 48px 표시에서는 세부가 읽히지 않을 수 있고, 세로로 긴 인물을 정사각형으로 표시하면 체형이 눌린다. 충돌 반경은 그림 크기와 별도로 유지한다.
- 현재 과제: 시안의 캐릭터 분위기가 원하는 방향인지 보고, 유지할 점/바꿀 점을 짧게 말한다. 완료 조건: 첫 플레이어의 스타일 기준 합의. 다음: 작은 표시 크기에 맞는 실루엣/비율을 보완한 뒤 기존 TextureCache 경로로 적용하고 화면에서 가독성을 확인한다. 새 API 코딩 과제는 아직 없다.
- 이번 검증: 생성 이미지 육안 검토 및 프로젝트 내부 복사 완료. 옷 장식이 많고 체형이 길어 현 48×48에서 적합성 확인 필요. 게임 코드/셰이더 변경이 없어 새 빌드나 실행 검증은 하지 않았다.
- 이번 진행은 범위/순서 문서 갱신이며 게임 코드는 변경하지 않았다. 마지막 빌드/패키지 검증은 단일 보스 첫 완성본의 Debug/Release 및 ZIP 자동 검사 성공 기록을 유지한다.
- 사용자 "끝"을 학습 종료로 해석한 것은 정정. 사용자는 게임 마무리를 끝까지 진행하라는 뜻이었고, 코드 정리/Release 검증/패키징을 이어서 완료했다.
- 코드 정리: 빈 TryTakeDamage 분기를 반환값을 명시적으로 버리는 호출로 변경, std::sin 사용, HealthBar 비율0~1/크기음수 방어, WM_SYSKEYDOWN/UP을 DefWindowProcW에도 전달해 Alt+F4 같은 시스템 동작 유지, AudioSystem 초기화 중 예외에서도 생성된 Voice를 정리, Unicode W 오류 표시 적용.
- ApplicationSmokeTest.cpp에 검사 로직을 분리하고 --smoke-test 실행 모드 추가. 입력 Pressed/Held/Reset, 체력바 비율/왼쪽 고정/반복 호출, 비활성 탄환 중복 피해 방지, 무적과 탄환 소비, CLEAR, 재시작, 동시 사망 FAILED를 검사하며 Title/Playing/Glow/Result 렌더를 실행한다. 실제 Win32 마우스·키 입력 자동 조작이나 픽셀 비교는 수행하지 않는다.
- package.ps1 추가: Release 구성/빌드→실행파일·assets·shaders·README.txt를 새 타임스탬프 폴더에 복사→프로젝트 밖 작업 경로에서 숨김 검사→성공 시 ZIP. 기존 패키지를 덮어쓰거나 삭제하지 않는다.
- 마지막 검증: Debug/Release 빌드 성공, 두 설정 자동 검사 종료0, ZIP 압축 해제본을 TEMP 작업 경로로 실행한 검사 종료0, assets 누락 복사본은 예상 오류/종료1. dumpbin으로 MSVCP140/VCRUNTIME140 의존 확인 및 런타임 요구사항 안내. 다른 PC/고DPI/드라이버 전체 호환성은 미검증.
- 배포 ZIP: out/packages/DanmakuLab-20261005-143507-351.zip. README 및 docs/RELEASE_VERIFICATION.md에 빌드/패키징과 검사 범위를 기록했다. 이후 소스 변경은 아직 추가 커밋하지 않았으며 첫 커밋1a76c6e 이후 작업 트리 변경으로 남는다.
- 사용자 기본 외형 변경에 대해 "오케이 좋다. 끝"으로 승인하고 이번 진행 종료. 추가 구현은 진행하지 않는다. 개별 F2/Glow 실행 결과는 별도 상세 보고가 없으므로 추가 검증을 수행했다고 기록하지 않는다. 다음 학습은 코드 정리/Release 배포 확인부터 재개한다.
- 사용자 HP120 전투가 좋은 것 같다고 확인하여 현재 값 유지. 기본 외형 정리: 플레이어 PNG는 기존 우주선 이미지이고 debugOverlayVisible_도 이미 false여서 유지했다. 기본 배경을 어두운 남색의 미세한 격자로 변경하고 F2로 기존 밝은 체크 격자를 전환하게 했다. 플레이어/탄환/보스 표시 원리는 변경하지 않는다.
- 창 초기 제목 및 FPS 갱신 제목에 Danmaku Lab을 반영하고 Playing HUD에 F1 디버그 안내, 디버그 키 목록에 F2 배경 안내 추가.
- 현재 확인 과제: 기본 배경에서 플레이어와 두 소유자 탄환 가독성, F2 밝은 배경 복귀, 6 Glow 표현 확인. 이번 변경은 기존 분기/Tint 반복 연결이라 에이전트가 처리했고 새 API 과제는 두지 않았다.
- 마지막 검증: 외형 정리 포함 MSVC Debug 빌드/링크 성공. 게임 화면 실행 확인 대기.
- 사용자 최소 창 크기 및 최대화/복원 확인 보고 수신. 해당 창 크기 단계 실행 확인 완료.
- 밸런스 첫 조정: Enemy 최대 HP30→120. 플레이어 발사 간격0.15초/피해1에서 지속 전탄 명중 기준 약4.5초→18초 분량의 공격이 필요하다(첫 발사 시점/탄 이동 시간 제외한 근사). 두 패턴을 경험할 시간을 늘리는 시험값이며 최종 확정은 플레이 후 결정한다.
- 마지막 검증: 시험 HP120 적용 후 MSVC Debug 빌드/링크 성공. 플레이 체감 확인 대기.
- GetMaxHp 기반 패턴 경계와 체력바는 자동으로 새 최대 HP를 사용해 별도 변경하지 않았다. 현재 과제: 한 판 플레이해 패턴 전환 전 종료처럼 짧은 문제 개선 여부, 원형 회피 난이도, 전투가 길어져 반복적이지 않은지 확인하고 체감을 설명한다. HP를 늘리는 것만으로 패턴 다양성이 개선된다고 간주하지 않는다.
- 창 축소 처리: Window 최소 client 크기 상수960×540 추가. WM_GETMINMAXINFO에서 AdjustWindowRect로 테두리/제목 표시줄 포함 크기를 계산한 후 MINMAXINFO::ptMinTrackSize만 지정한다. 최대화 관련 필드는 유지하고 기존 WM_SIZE 기반 DX11/Grid Resize는 그대로 사용한다.
- 선택 이유: 작은 게임 범위에서 고정 보스 좌표와 HUD를 보호하기 위해 최소 창 크기를 둔다. 가상 해상도/레터박스 렌더링은 별도 확장으로 남긴다. 현재 WM_GETMINMAXINFO는 테두리 드래그 최소 크기를 제한하며 고DPI별 전용 대응은 구현하지 않는다.
- 현재 과제: 창을 끝까지 줄여도 보스/적 체력바/우상단 HUD가 남는지, 최대화 후 복원해도 렌더와 이동/충돌이 정상인지 확인. 새 API 설명은 WM_GETMINMAXINFO, lParam의 MINMAXINFO 포인터, ptMinTrackSize x/y 및 AdjustWindowRect의 client→window 크기 변환을 포함한다.
- 마지막 검증: 최소 창 크기 처리 포함 MSVC Debug 빌드/링크 성공. 실제 창 크기 및 표시 확인은 사용자 대기. 이전 포커스 수정 재현 확인도 아직 명시 보고는 없으므로 완료로 단정하지 않는다.
- 사용자 최소화 중 키를 놓으면 복귀 후 Held가 남는 문제 보고. WM_KILLFOCUS에서 포커스 손실 알림을 기록하고 이전 pendingKeyEvents를 비운다. Application이 알림을 소비하면 Input::Reset으로 current_/previous_ 모두 false로 초기화하고 이후 입력 이벤트를 적용한다.
- HandlePendingInput 호출을 메시지 처리 직후/최소화 대기 분기 앞으로 이동해 최소화 직후에도 입력 초기화가 수행된다. 창 포커스 손실에 적용하므로 Alt+Tab에도 대응한다. P pause나 HP는 변경하지 않는다.
- 마지막 검증: 포커스 입력 수정 포함 MSVC Debug 빌드/링크 성공. 사용자 재현 테스트(방향키/Z 누른 채 최소화 또는 Alt+Tab→다른 창에서 키 해제→복귀→새 입력 정상) 대기. 실제 UI 재현 확인은 아직 수행하지 않았다.
- 최소화 과제 최신 구현 리뷰 통과: WaitMessage 후 Tick 반환값 버림/accumulator0/continue 순서로 수정되고 일반 delta Tick은 분기 뒤로 이동. 사용자 핵심 구현을 유지하면서 에이전트가 WaitMessage 실패 시 runtime_error 처리 및 명시 stdexcept include를 보완해 최소화 상태 렌더링으로 내려가지 않게 했다.
- 마지막 검증: 최소화 구현 및 오류 처리 포함 MSVC Debug 빌드/링크 성공. 최소화5초/복귀/기존 P pause 유지 실행 확인은 사용자 대기.
- 최소화 과제 첫 리뷰: WaitMessage 후 continue가 타이머/누적시간 처리보다 먼저 있어 뒤 두 줄이 실행되지 않는다. timer_.Reset은 실제 누적 시간까지 0으로 만들므로 대기 후 Tick 반환값을 버리는 방식으로 보완하도록 안내. 일반 프레임 realDeltaSeconds 산출은 최소화 분기 뒤 배치 권장. 소스 리뷰만 수행, 수정 대기.
- Window WM_SIZE는 최소화 크기 변경을 무시하지만 Run은 계속 Update/Render하는 문제 확인. 주변 코드로 minimized_ 및 IsMinimized() 조회를 추가하고 WM_SIZE의 SIZE_MINIMIZED 여부로 갱신했다. Application::Run 메시지 처리 직후에 최소화 처리 TODO를 준비.
- 현재 과제: 해당 TODO에서 최소화 중 게임 Update/Render를 건너뛰고 WaitMessage로 새 메시지를 기다린다. 대기 후 timer_.Tick 반환값은 게임에 적용하지 않고 기준 시각/실제 시간만 갱신하며 accumulatorSeconds_를 비운다. 메시지 처리는 계속 가능해야 하고 기존 P pause 상태를 변경하지 않는다. 새 API WaitMessage와 WM_SIZE의 wParam 의미를 설명한 뒤 사용자에게 맡긴다.
- 완료 조건: 최소화 5초 동안 게임 시간/탄환/무적 진행 없음, 복귀 시 순간 이동이나 몰아서 업데이트 없음, 최소화 전 P pause가 복귀 후 유지됨. BGM은 기존 정책대로 재생 유지. 현재 TODO는 아직 무동작이므로 사용자 구현 전에는 기존처럼 진행된다.
- 마지막 검증: Window 최소화 조회 및 Run TODO 주변 코드 MSVC Debug 빌드/링크 성공. 사용자 핵심 구현과 실행 검증 대기.
- 사용자 탄환 소유자 색상/형태 전환 확인 완료. 다음 마무리로 Player GetMaxHp와 HealthBar GetHeight 조회 추가, 플레이어 체력바를 위쪽 8px 간격으로 배치(화면 상단에서는 최소 중심 Y를 반높이로 제한)했다.
- 플레이어 체력바 데이터 갱신을 fixed Update에서 RenderGameUI로 옮겼다. 이는 HP를 변경하는 로직이 아니라 현재 HP/위치의 표시 데이터 구성으로, 새 Playing 첫 렌더 전에 fixed update가 없어도 즉시 올바른 값/위치가 나온다.
- HP0 Player/Enemy는 Sprite를 출력하지 않는다. Result에서 기존 전투 정지 장면과 CLEAR/FAILED 안내는 유지한다. Title 표시명을 DANMAKU LAB으로 갱신.
- 마지막 검증: 위 마무리 코드 포함 MSVC Debug 빌드/링크 성공. 사용자 확인 과제: 위쪽 체력바 이동 추적/상단 가시성, 재시작 직후 HP 가득 표시, 클리어 보스 숨김 및 실패 플레이어 숨김. 이번 단계는 기존 원리의 반복 연결이어서 에이전트가 처리했다.
- 게임 마무리: Player 탄환은 청록 Tint(0.15,0.75,1), Enemy 탄환은 붉은 Tint(1,0.18,0.12)로 구분. 기존 Yellow 대신 White 텍스처를 공유하여 곱셈 Tint로 원하는 색을 표현한다. Glow 주변광은 같은 RGB를 1.8배/alpha0.65로, 코어는 같은 기본 Tint로 출력. Rect/SoftCircle/Glow 모두 소유자 색을 사용하고 충돌/피해 규칙은 변경하지 않았다.
- 사용자 기존 선호(반복 분기 과제 생략)에 맞춰 색상 연결은 에이전트가 완료했다. 이번 과제는 실행 확인: Z 플레이어 탄환 청록/적 탄환 붉은 표시, 4/5/6 형태 전환 시 구분 유지 확인. 새로운 API는 사용하지 않았다.
- 마지막 검증: 색상 변경 포함 MSVC Debug 빌드/링크 성공. 실제 화면 색상 확인은 사용자 대기.
- 사용자 요청으로 수직동기화 설정을 Present(1,0)에서 Present(0,0)으로 변경. 고정 game update 60Hz는 유지한다. tearing 지원 swapchain/Present flags 변경은 이번 범위에 포함하지 않는다.
- 마지막 검증: 수직동기화 변경 후 게임/CollisionBenchmark MSVC Debug 빌드·링크 성공. 표시/FPS 변화는 이번 턴에서 실행 확인하지 않았다.
- 사용자가 게임 완성을 먼저 하자고 지적하여 진행 순서를 정정했다. 작은 보스 전투의 흐름 확인과 프로젝트 전체 완성을 구분하며, 렌더 스트레스/인스턴싱/파티클 추가는 마무리 뒤 진행한다.
- 현재 마무리 후보: 플레이어 체력바 위 배치, 탄환 소유자별 시각 구분, 클리어/실패 결과의 표시와 피드백, 난이도/테스트 자산 정리, 새 판 첫 렌더 시 표시 데이터 동기화와 창 크기 변경 등 경계 상황 검증. 새 스테이지/보스 추가나 파티클 시스템은 필수 완료 조건으로 확대하지 않는다.
- Step 18-1 문답: 사용자가 Grid 구성 오버헤드를 지적해 후보 감소와 실행 시간 차이의 이유를 이해했다. 이후 작은 합성 CPU 수치보다 실제 게임에 큰 렌더 부하를 만들어 병목 확인/최적화하기를 제안했다.
- 코드 확인: 현재 DrawSprite는 스프라이트마다 상태 바인딩, Transform/Tint 상수 버퍼 갱신 및 DrawIndexed를 호출한다. Glow 탄환은 주변광/코어 두 Sprite로 그린다. Present(1,0)을 사용하므로 FPS만으로 렌더 비용을 판단하지 않는다. 아직 CPU/GPU 병목은 측정으로 확인하지 않았다.
- 제안 방향: 먼저 같은 텍스처의 대량 스프라이트를 1천/5천/1만 수준으로 조절하는 렌더 스트레스 모드에서 비용을 측정하고, 확인된 CPU 제출 병목에 배칭/인스턴싱을 적용한다. 이후 Glow 크기/겹침을 늘려 GPU 픽셀 부하를 별도 실험한다. 파티클 시스템은 후속 확장 선택지이며 이번 메시지에서는 구현을 시작하지 않는다.
- 사용자 Step 17-2 전투 흐름 실행 검증이 잘 된다고 확인. 패턴/승패/재시작/일시정지 확인 완료 보고를 수신하고 Step18로 진행.
- Step 18-1 측정 인프라 완료: 별도 CollisionBenchmark 대상 및 tools/collision_benchmark.cpp 추가. 실제 Collision/UniformGrid를 재사용하며 고정 256-slot 합성 배치에서 활성32/128/256을 비교한다. 두 경로의 Hit/Graze 개수를 확인하고 Release에서 워밍업 후 반복 측정. Grid 시간에 Clear/Insert/주변 셀 검사를 모두 포함하며 게임 상태는 변경하지 않는다.
- 마지막 검증: 게임 및 CollisionBenchmark MSVC Release 빌드/링크 성공. 벤치마크 종료0, 모든 배치에서 Hit/Graze 개수 일치. 첫 결과는 docs/COLLISION_BENCHMARK.md에 기록(32개: 순회0.228µs/Grid0.398µs, 128개:0.595/0.598, 256개:1.083/0.926). 실제 게임 FPS 측정 결과로 해석하지 않는다.
- 현재 과제: 32개 및 256개 조건에서 비용만 보면 어떤 방식을 선택할지, 128개에서 후보 수가 128→22로 줄었는데 실행 시간이 거의 같은 이유를 설명한다. 새로운 chrono API의 steady_clock와 duration 단위 변환 설명을 제공하고 코드 작성 과제는 두지 않는다.
- Step 17-1 최종 소스 확인: 원형 발사 후 간격이 1.0초로 수정되어 패턴 선택 과제 코드 기준 완료. 이전 README의 미수정 간격 항목 제거. 사용자 선택한 표시명 Danmaku Lab을 README 제목에 적용했고 CMake 대상/실행 파일명은 유지한다.
- 현재 과제(Step 17-2): 한 판 클리어와 한 판 실패를 각각 실행하고 Result→Title→Playing 재시작 시 Player HP3/Enemy HP30/점수0/탄환 초기화/첫 패턴 복귀를 확인. P 일시정지 중 이동·탄환·무적 타이머·깜빡임 정지 확인. 두 패턴 경계 실행 확인도 포함한다. 코딩 과제는 없으며 문제를 발견하면 재현 상황을 보고한다.
- 마지막 검증: 패턴 간격 수정은 소스 확인 완료. 이번 단계 실행 검증은 사용자 확인 대기.
- 사용자 요청으로 Git 업로드 준비: 기존 초기 README를 현재 기능/빌드/조작키/구조/제한/학습 기록 안내로 갱신. 최종 포트폴리오 문서가 아닌 저장소 사용 안내이며 현재 학습 Step은 유지한다. 소스와 테스트 자산을 포함하는 첫 로컬 커밋을 준비했고 원격 업로드는 사용자가 수행한다.
- 최신 소스 확인: 패턴 경계는 GetMaxHp()/2로 수정되고 원형 분기 미사용 각도 상수도 제거됨. 원형 발사 간격은 여전히 1.5초이므로 README에도 현재값과 목표 1.0초를 구분했다. 플레이어 체력바 아래쪽 배치도 제한으로 명시.
- 마지막 검증: README 갱신 시점의 현재 소스 MSVC Debug 빌드/링크 성공. 결과/재시작 및 패턴 전환 전체 실행 확인은 대기.
- Step 17-1 첫 리뷰: HP>15 부채꼴 / else if HP>0 원형 및 쿨다운 유지 방식은 현재 최대 HP30에서 맞음. 원형 발사 뒤 간격이 아직 1.5초라 요구 1.0초로 수정 필요. 경계15는 GetMaxHp 기반으로 일반화 권장, 원형 분기의 미사용 각도 상수 두 개 제거 안내. 소스 리뷰만 수행, 수정 대기.
- Step 16-7 사용자 구현 리뷰 통과: Player HP 0 Failed, else if Enemy HP 0 Clear로 동시 사망 실패 우선 및 단일 Result 전환 성립. 빌드 성공, 승패/재시작 실제 실행 확인은 아직 명시 미보고.
- Step 17-1 주변 코드: 기존 발사/회전/쿨다운 블록을 UpdateEnemyShooting 함수로 추출하고 Update 호출 연결. 기본 부채꼴 동작 유지 및 HP 기반 선택 TODO만 추가. MSVC Debug 빌드/링크 성공.
- 현재 과제: HP 절반 초과는 기존 7발/140px·s/1.5초 부채꼴, 절반 이하는 SpawnRing 16발/160px·s/1.0초. 원형 시작 각도는 기존 enemyFanAngleRadians_를 재사용. HP 0에서는 발사 없음, 기존 탄환은 유지, 패턴 변경 시 이미 진행 중인 쿨다운은 유지하고 다음 발사부터 새 간격 적용. HP로부터 구간을 계산하므로 별도 phase 상태 멤버는 필요 없다.
- 완료 조건: HP 16과 15 경계(최대 HP30)에서 다음 발사 패턴이 달라짐, 두 패턴이 한 발사에서 동시에 나오지 않음, frame마다 쿨다운을 초기화하지 않음, 새 판은 첫 패턴으로 시작. 사용자 구현/실행 확인 대기. 아래 Step16 과제 기록은 이력이다.
- 사용자가 Step 16-6 적 체력바 표시/감소가 잘 동작한다고 확인. 크기 일반화 과제 완료.
- Step 16-7 주변 코드 완료: Application::BattleOutcome(None/Clear/Failed), 새 Playing에서 None 초기화, 결과 HUD CLEAR/FAILED 표시, 두 충돌 처리 이후 CheckBattleOutcome 호출. 기존 Player HP 0 실패 처리는 새 함수로 이동해 유지.
- 현재 과제: CheckBattleOutcome에 적 HP 0 클리어 조건 추가. 두 HP가 같은 fixed update에서 0이 되면 실패 우선이라는 정책을 적용해 결과를 덮어쓰지 않는다. 승패 확정 시 battleOutcome_ 저장 및 Result 전환. 새 API 없이 enum/분기/기존 상태 전환 사용.
- 마지막 검증: 클리어 TODO 및 기존 실패 처리 포함 MSVC Debug 빌드/링크 성공. 사용자 클리어 판정 구현 및 실행 확인 대기.
- Step 16-6 최신 리뷰 통과: UpdateSprites에서 value_×배경 너비로 채움 너비를 계산한 뒤 중심을 보정한다. 모든 setter가 이를 호출하여 정상 범위 입력에서 크기/비율/위치 설정 순서와 무관하게 동일한 결과를 만든다. SetSize에서 높이도 함께 갱신됨. MSVC Debug 빌드/링크 성공. 적 체력바 표시 및 명중 시 감소는 사용자 실행 확인 대기.
- Step 16-6 두 번째 리뷰: value_ 초기화/저장과 SetSize에서 비율 적용 및 중심 갱신은 수정됨. 그러나 SetValue에서 너비 계산이 제거됐고 UpdateSprites도 중심만 계산하여 비율만 변경할 때 너비가 갱신되지 않음. 너비 계산을 공통 UpdateSprites의 중심 계산보다 앞에 모으도록 힌트 제공. 소스 리뷰만 수행, 수정 대기.
- Step 16-6 첫 구현 리뷰: 고정 너비 30을 배경 너비로 교체한 것은 맞음. 그러나 SetSize가 배경/채움 모두 전체 크기로 대입하여 기존 비율을 잃고, UpdateSprites도 호출하지 않아 중심 보정이 이전 크기 기준으로 남음. 비율 0.5를 설정한 뒤 240×12로 변경하면 채움 너비가 기대 120 대신 240이 되는 예제로 설명. 채움 비율을 멤버로 보관하고 크기 변경 시 이를 적용하여 중심까지 갱신하도록 힌트 제공. 소스 리뷰만 수행, 수정 대기.
- Step 16-5 최신 코드 확인 통과: inactive 및 비Player 필터, 명중 시 피해 요청과 탄환 소비, break 제거 완료. 사용자 실행 확인은 명시 미보고. 이번 주변 코드 빌드와 함께 컴파일/링크 확인됨.
- Step 16-6 주변 코드 완료: Application에 enemyHealthBar_ 추가, 생성 시 SetSize({240,12}) 호출, RenderGameUI에서 현재 적 HP 비율과 화면 가로 중앙/Y32 위치 설정 및 Sprite 출력. 기존 HealthBar를 두 인스턴스가 재사용한다. 표시 상태만 Render에서 갱신하며 게임 HP는 변경하지 않는다.
- 현재 과제: HealthBar::SetSize의 크기 설정과 기존 고정 30 기반 계산을 일반화. 현재 채움 비율을 기억해 크기 변경 후에도 유지하며 크기/비율/위치 setter 순서에 무관한 결과를 만든다. 기본 크기는 30×10 유지. 함수 선언 및 무동작 TODO는 에이전트가 준비했으며 핵심은 사용자 구현.
- 완료 조건: 적 체력바 240×12 표시, HP 비율대로 감소/왼쪽 끝 고정, 플레이어 체력바 기존 크기 유지, 비율 0.5에서 전체 너비 30→240 변경 시 채움 15→120. 현재 SetSize는 무동작이라 구현 전 적 체력바도 기본 크기로 보인다.
- 마지막 검증: Step 16-6 TODO 포함 MSVC Debug 빌드/링크 성공. 크기 변경 사용자 구현 및 화면 확인 대기. 아래 이전 과제 기록은 이력이다.
- Step 16-5 두 번째 리뷰: Enemy 소유 제외, break 제거, 명중 시 탄환 소비 분리는 수정됨. active 필터가 여전히 빠져 소비한 플레이어 탄환이 같은 위치에서 다음 fixed update에도 피해를 줄 수 있음. 루프 시작 조건에 !bullet.active를 추가하도록 안내. 빈 TryTakeDamage if 본문은 실제 호출이 일어나므로 기능 오류는 아님. 소스 리뷰만 수행, 수정 대기.
- Step 16-5 첫 사용자 구현 리뷰: 원 충돌 판정과 피해 요청 연결은 맞으나 active/owner 필터 누락으로 비활성 탄환 및 적 자신의 탄환도 피해를 줄 수 있다. 명중 후 break로 한 fixed update에 한 발만 처리하며, 피해 성공 여부에 탄환 소비가 묶여 HP 0에서는 명중 탄환이 남는다. 세 항목을 설명하고 수정 과제로 유지. 이번 검증은 소스 리뷰만 수행.
- 사용자가 깜빡임 동작이 맞는 것 같다고 보고하고 다음 진행 요청. Step 16-4 실행 확인 보고 수신.
- Step 16-5 주변 코드 완료: Enemy HP 30, GetHp/GetMaxHp, 피해 1 및 HP 0 방어 TryTakeDamage, 새 Playing 진입 Reset, HUD 적 HP 표시, HP 0에서 추가 발사 중단. 적 무적 시간은 두지 않는다. HP 0에서 클리어 전환/적 숨김은 후속 단계이며 현재는 화면에 남는다.
- 현재 과제는 Application::CheckEnemyPlayerBulletCollisions의 명중 처리 하나. 함수 호출/선언/TODO 준비 완료. 활성 Player 소유 탄환만 검사, CircleHitbox와 Intersects 재사용(적 반경 24px / 플레이어 탄환 반경 4px), 명중 시 피해 요청 및 탄환 소비. 적은 하나라 전체 풀 순회로 시작한다.
- 완료 조건: 적 탄환으로 적 HP가 감소하지 않음, 명중한 플레이어 탄환 1개당 HP 1 감소 및 비활성화, HP 음수 없음, 새 판 HP 30 복귀. TODO 포함 MSVC Debug 빌드/링크 성공. 사용자 명중 구현 및 실행 검증 대기.
- 이전 체력바 위 배치 완료 기록 정정: 현재 Application은 playerY + playerHeight + 8을 사용하므로 아래쪽 배치다. 사용자에게 발견 사실을 알렸고 이번 단계에서 HealthBar 관련 코드는 변경하지 않았다.
- Step 16-4 최신 소스 리뷰 통과: IsInvulnerable 조건 아래 남은 무적 시간×2π×6의 sin 부호로 alpha 1.0/0.25를 선택하고 플레이어 Tint에 전달한다. 비무적 시 alpha 1.0, pause 중 남은 무적 시간 정지, 체력바에는 효과 미적용. 이번 리뷰는 소스 확인만 수행했으며 빌드 및 실제 깜빡임 실행 확인은 대기.
- Step 16-4 첫 리뷰: 무적 남은 시간을 sin 입력으로 쓰는 선택은 유효하며 pause 중에도 정지한다. 현재 max(0,sin(...))은 양의 반주기에 부드러운 알파 변화, 음의 반주기에 완전 투명으로 동작하여 지정한 1.0/0.25 교대와 다르다. 사인 부호로 두 알파를 선택하도록 힌트 제공. GetInvulnerabilitySeconds의 bool 변환은 현재 비음수 계약에서 동작하나 IsInvulnerable이 의도를 더 명확히 표현한다. 소스 리뷰만 수행, 수정 대기.
- 사용자가 체력바 배치/동작을 확인했다고 보고하고 시각 효과 진행 요청. Step 16-4는 기존 Player::IsInvulnerable(), gameTimeSeconds_, SpriteDrawData tint alpha로 피격 무적 중 깜빡임을 구현한다. 새 Shader/API/리소스는 필요 없다.
- Step 16-4 현재 과제: RenderPlaying에서 무적 중에만 초당 6주기로 alpha를 1.0/0.25 사이에서 교대시키고 그 외에는 1.0으로 표시. sin의 부호와 2π×주파수의 관계를 설명하며 정답 코드는 미제공. 게임 시간을 사용해 pause 중 깜빡임도 멈춘다. Render는 HP/무적 시간을 변경하지 않는다.
- Step 16-4 완료 조건: 피해 직후 깜빡임, 무적 종료 시 완전 불투명, P pause 중 깜빡임 정지, 체력바에는 깜빡임 미적용. 코드/실행 검증 대기. 아래 Step 16-3 기록은 이전 과제 이력이다.
- 최신 리뷰 통과: UpdateSprites가 배경 center.x에서 너비 차이의 절반을 빼서 채움 center.x를 대입하도록 수정됨. 반복 호출 시 보정이 누적되지 않고 두 setter 호출 순서가 같은 결과를 만든다. Y는 SetPosition에서 두 Sprite에 같이 대입하므로 현재 구조에서 유지된다. 범위 0~1 입력 기준 왼쪽 끝 고정 성립. 소스 리뷰만 수행했으며 화면 실행 검증은 대기.
- 보완 과제 리뷰: 두 setter가 UpdateSprites를 호출하도록 변경했으나, UpdateSprites의 center[0] -= 보정이 호출마다 누적된다. 반복 SetValue(0.5)로 중심이 계속 왼쪽으로 이동하는 문제를 설명. 배경 center를 기준 위치로 사용해 채움 중심을 매번 기준에서 재계산하는 힌트 제공. 별도 position_/value_ 멤버 추가는 필수가 아님. 수정/검증 대기.
- 소스 확인: 두 swap이 대입으로 수정됐고 불필요한 1.5 위치 보정 및 미사용 value_가 제거됐다. 배경/채움 초기 크기는 모두 30×10이다.
- 현재 핵심: 중심 좌표와 사각형의 위/아래 끝을 이용해 다른 객체 위에 UI를 배치한다. 화면 Y는 아래로 증가한다. HealthBar는 플레이어를 알지 않고 Application이 배치 위치를 전달한다.
- 현재 과제: Application의 HealthBar 위치 연결을 바꿔 플레이어 위쪽 끝과 체력바 아래쪽 끝 사이에 8px 간격을 만든다. 현재 Player 높이는 GetHeight(), 체력바 높이는 10px이며 새 API는 필요 없다.
- 완료 조건: 플레이어 이동 시 간격 유지, HP 3→2→1에서 채움 왼쪽 끝 고정, 무적 중 연속 피해 없음, 재시작 시 가득 찬 체력바 확인. 재시작 직후 첫 fixed update 이전에는 이전 표시 데이터가 남을 가능성도 후속 리뷰에서 확인한다.
- 마지막 검증: 소스 리뷰만 수행. 이번 변경 후 빌드/화면 동작은 미검증. 사용자 핵심 코드는 수정하지 않음.

## 최신 진행 — 2026-10-04

- HealthBar 추적 지연 원인 확인: `SetPosition`에서 배경 center와 입력 position을 `swap`하여 입력이 이전 배경 좌표로 바뀌고, 채움이 그 이전 좌표를 사용한다. 사용자에게 swap과 대입의 차이를 설명하고 두 swap을 대입으로 수정하도록 안내. 구현은 직접 수정하지 않았으며 수정 후 실행 확인 대기.
- 사용자 요청으로 CMake 소스 목록을 `file(GLOB ... CONFIGURE_DEPENDS)` 방식으로 변경했다. `src` 바로 아래의 `.cpp`, `.h`, `.hpp` 파일을 자동 수집하며 하위 폴더는 포함하지 않는다.
- `HealthBar.cpp`가 빌드 대상에 포함되어 `HealthBar::HealthBar()` LNK2019 오류가 해결됐다. HealthBar 구현은 수정하지 않았다. 현재 과제는 HealthBar 구현/렌더 연결을 완료한 뒤 리뷰받는 것이다.
- 마지막 검증: 자동 소스 수집 적용 후 MSVC Debug 빌드/링크 성공. 체력바 화면 동작은 아직 미검증.

- 사용자 명시 요청: HealthBar는 파일과 빈 클래스 틀만 에이전트가 생성하고, 메서드/상태 설계부터 CMake 등록, Application 연결, 실제 표시까지 사용자가 전부 구현한다. 완료 후 에이전트는 코드 평가를 수행하며 먼저 정답 구현을 작성하지 않는다.
- 생성 파일: `src/HealthBar.hpp` 빈 HealthBar 클래스, `src/HealthBar.cpp` 헤더 include만 추가. 게임 코드와 CMake는 변경하지 않았다. 현재 단계에서 빌드 재검증은 하지 않음.
- 요구사항: Player 위를 따라가는 체력바, 배경과 채움 표시, HP 비율에 따라 왼쪽 끝 고정/오른쪽 끝 감소, HP 0에서 채움 없음. 체력바는 HP를 변경하지 않고 GPU 리소스를 별도 소유하지 않는다.
- 필요한 선행 설명: SpriteDrawData 계약, 중심 기준 사각형 좌표, 정수 나눗셈/분모 0, 배경-채움 그리기 순서, D3D Sprite를 D2D 패널보다 먼저 출력해야 하는 현재 렌더 구조, Player 최대 HP 접근 설계, CMake 소스 등록.
- 기존 `TryTakeDamage()`에 HP 0/무적 거부 및 HP 감소/무적 설정 구현이 들어 있음. 이번에는 사용자 요청에 따라 HealthBar 완료 전 별도 리뷰로 진행을 끊지 않는다.

- 현재 과제: `Player::TryTakeDamage()` 구현. HP 0 또는 무적 중이면 상태 변경 없이 false, 피해 허용 시 HP 1 감소 및 1초 무적 설정 후 true. 현재 placeholder는 항상 false이므로 과제 전에는 피해를 받지 않는다.
- Player가 HP(기본 3), 남은 무적 시간(기본 0), 무적 지속시간 상수(1초)를 소유한다. Reset / fixed update 타이머 감소 / HUD 표시 / 충돌에서 피해 요청 / HP 0 Result 전환은 에이전트가 연결했다.
- 몸에 닿은 적탄은 피해 허용 여부와 무관하게 비활성화한다. playerHit_는 이번 fixed update에서 실제 피해가 적용됐는지를 나타낸다. 무적 중 몸에 닿은 탄환은 Graze 점수를 주지 않는다.
- Result 전환이 발생하면 같은 외부 프레임에 남은 fixed update도 중단하도록 보완했다.
- 완료 조건: 연속 피해 요청에 첫 요청만 성공, 무적 종료 후 다시 피해 가능, HP 0 미만 방지, P 일시정지 중 무적 타이머 유지, 새 판 HP 3/무적 0 초기화.
- 마지막 검증: Step 16-2 주변 코드 및 TryTakeDamage placeholder 포함 MSVC Debug 빌드/링크 성공. 실제 피해 코드는 사용자 구현 이후 검증.

- 사용자 피드백: 신규 Direct2D UI API의 설명이 누락되었음. HP 과제 진행을 잠시 미루고 DrawUiPanel, Direct2D/DirectWrite, D3D11과 공유 BackBuffer 관계를 먼저 설명한다. 단순 배선 생략 요청은 새로운 API 설명 생략을 의미하지 않는다.

- Step 16-1: Graphics에 문자열/영역을 받는 `DrawUiPanel` 추가. Application이 게임 데이터로 HUD 문자열을 구성하고 Graphics는 그리기만 담당한다.
- Playing에는 우상단 Score/Graze/Pause HUD, Title에는 시작 안내, Result에는 최종 Score/Graze 및 복귀 안내를 표시한다. F1 디버그 패널을 숨겨도 게임 UI는 유지된다. 모든 Sprite 출력 뒤 UI를 그린다.
- 새로운 탄환의 첫 Graze 이벤트에서만 100점을 가산한다. `score_`는 Application이 소유하고 새 Playing에서 0으로 초기화하며 Render에서는 변경하지 않는다.
- 현재 과제(개념): HP=3 상태에서 같은 탄환이 3 fixed update 동안 겹치면 매 충돌마다 HP를 감소시키는 방식의 문제가 무엇인지 설명한다. 다음 구현은 피해 이벤트와 무적 시간이다.
- 마지막 검증: HUD 및 Score 코드 MSVC Debug 빌드/링크 성공. 화면 레이아웃 실행 검증은 아직 미수행.

- 사용자 요청에 따라 단순 함수 호출, 이미 배운 패턴의 반복 연결은 에이전트가 즉시 처리한다. 과제는 새로운 원리 적용, 설계 판단, 의미 있는 디버깅에만 남긴다.
- Playing 진입에서 `audio_.PlayBgm()`, Result 진입에서 `audio_.StopBgm()` 연결 완료. 현재 과제 없음.
- BGM은 게임 fixed update와 별도로 재생되므로 P로 게임을 일시정지해도 계속 재생된다. 현재 정책은 Playing에서 반복, Result에서 중지, 새 판에서 처음부터 재생.
- 실제 BGM 청취/상태 전환 실행 확인은 아직 미확인. 마지막 빌드 결과는 아래 최신 검증 기록을 따른다.
- 마지막 검증(2026-10-04): BGM 상태 연결 포함 MSVC Debug 빌드/링크 성공.

## 최신 인수인계 — 2026-10-02

아래 Step 8C 상태가 현재 진행 상황이다. 이후 기록은 과거 이력이다.

- 현재 Step: **8C — Sprite Shader 실습**, Tint/Alpha·시간·UV 변환 구현 통과, 원형 알파 마스크 과제 진행 중. Step 0~8B 학습/구현 완료.
- 8B 코드 리뷰: PSSetShaderResources(0,1,선택 SRV.GetAddressOf()) 구현 정확. static_cast<int>로 0/1 ID를 인덱스화했으며 DrawSprite의 사전 ID 검증 아래 안전하게 사용. 사용자 다음 진행 요청. 구체적 실행 색상/교체 결과는 미보고.
- 사용자 질문: 슬롯/개수를 왜 리터럴로 지정하는지, config/동적 변수 필요 여부. 설명: 현재 슬롯은 HLSL↔C++ 인터페이스 약속이라 명명 상수로 표현하고, 설정 파일에 독립 변경 가능하게 노출하지 않음. 여러 입력 텍스처/다른 Shader 배치에서 슬롯·개수가 달라질 수 있음. 현재 NumViews=1은 한 SRV 선택을 뜻함.
- 준비 완료: SpriteDrawData.tint(RGBA float4, 기본 모두1), SpriteTintConstants 16바이트 GPU 버퍼, DrawSprite마다 tint 복사/Map/Unmap, PS b1 연결. 기존 미사용 Triangle 색상 버퍼를 Tint용으로 정리.
- 명명 상수: kSpriteTextureSlot=0(t0), kSpriteSamplerSlot=0(s0), kSpriteTransformSlot=0(VS b0), kSpriteTintSlot=1(PS b1). b1이 색상 전용 특별 슬롯은 아니며 이번 셰이더의 배치 약속.
- 테스트 값: Player tint=(1,1,1,1), Yellow Bullet tint=(1,0.2,0.2,0.5). 과제 완료 시 Player 원색 유지, Bullet 빨강/주황 계열 + 반투명.
- 현재 핵심: Sample의 float4 RGBA와 tint를 성분별로 곱해 출력. RGB는 곱셈 색조, Alpha는 기존 Alpha Blend 입력으로 전달. 검정/0 성분은 곱셈만으로 다른 색으로 바꿀 수 없음. PS 색상 계산과 OM Alpha Blend는 서로 다른 단계.
- Tint/Alpha 구현 통과 — 2026-10-02: 사용자가 PSMain에서 Sample 결과에 spriteTint를 곱해 반환. 파일에서 구현 확인, fxc PSMain 컴파일 성공, MSVC Debug 빌드 성공. 완료된 TODO 주석만 제거. 출력 색/알파의 구체적인 실행 보고는 아직 없음.
- 사용자 관심: 시간에 따른 sin/cos 플레이어 색 변화, 탄환 글로우, 글로우와 포스트 프로세스의 관계. HLSL sin은 시간 값을 전달받아 계산 가능, Shader가 CPU 게임 시간을 자동으로 알지는 않는다는 설명 제공. Quad 내부의 UV 기반 빛무리/가산 혼합과 별도 render texture 기반 Bloom 후처리 구분.
- 시간 정책 개념 — 2026-10-02: 사용자 Unreal Time 노드 경험과, pause 중 UI/Shader 효과를 위해 게임 시간과 별도 시간을 쓰는 설계 제안. 게임 시뮬레이션 시간(pause/timeScale 반영)과 일시정지 영향을 받지 않는 표시용 시간의 차이를 설명. Shader 자체에 단일 시간 정책이 있는 것이 아니라 CPU가 효과별로 원하는 시간을 전달. Unreal Time의 Ignore Pause 설정을 공식 문서로 확인.
- 다음 시간 실습 준비 시 주의: 일시정지 중 게임 fixed Update는 중단하되 렌더링/표시용 시간 갱신은 유지. 현재 GameTimer delta는 장시간 프레임을 clamp하므로 누적값을 엄밀한 실제 경과 시간이라고 부르지 않음. 별도 실제 경과 시간과 게임/표시 시간 정책을 명확히 정한 후 코드 준비.
- 시간 주변 코드 준비 완료: GameTimer::Tick은 실제 경과 dt를 반환하고 GetElapsedSeconds로 비clamp 누적 시간 제공. 기존 0.25초 clamp는 Application의 게임 fixed Update 입력으로 이동. gameTimeSeconds는 실제 수행한 fixed step만큼 증가.
- Pause: P키 Pressed로 토글. pause/토글 프레임에서는 fixed Update 미수행, accumulator는 토글 시 초기화해 중지 시간 catch-up 방지. 메시지/입력/resize/FPS/Render는 계속 수행. 이동/발사/탄환 삭제/게임 시간이 멈춤.
- GPU 시간 연결: FrameConstants(16바이트)에 gameTimeSeconds/realTimeSeconds를 담아 프레임마다 PS b2에 전송. SpriteTintConstants는 32바이트로 확장해 timeSource와 padding 포함. SpriteDrawData.timeSource는 Game(0)/Real(1), Player=Real, Bullet=Game. Shader에서 선택한 시간을 ComputePulse 인자로 전달.
- 맥동 코드 리뷰(2026-10-02): 사용자 sin(2πt/T), -1~1→0~1, 0.25~1 구간 매핑 구현 정확. fxc PS/VS 컴파일과 MSVC Debug 빌드 성공. pause/맥동 결과의 구체적 실행 보고는 미확인.
- 사용자 과제 난이도 선호: 맥동 과제는 공식을 모두 제공해 너무 정답을 알려준 느낌이라고 피드백. 필요한 개념/API를 설명하되 완성 변환식/구현 순서를 선행 제공하지 않고 원하는 결과와 입력 계약으로 과제를 제시.
- UV 주변 코드 준비 완료: SpriteDrawData.uvRect=(startU,startV,endU,endV), 기본(0,0,1,1). SpriteTintConstants를48바이트로 확장해 uvRect를 PS b1에 전송. PSMain에서 TransformSpriteUV 결과만 Sample에 사용. 원본 input.uv는 로컬0~1 좌표로 유지해 후속 형태/빛무리 계산과 구분.
- 테스트 제어: 숫자1=전체영역(0,0,1,1), 2=좌상영역(0,0,0.5,0.5), 3=좌우반전(1,0,0,1). Application의 표시 제어는 pause 중에도 가능. Player만 영역 변경, Bullet은 기본 전체영역 유지.
- UV 코드 리뷰(2026-10-02): 사용자가 u/v 각각의 시작·끝 성분을 lerp하고 float2로 반환. 전체/부분/역방향 범위를 모두 처리하며 정확. chained swizzle도 HLSL 컴파일 성공. 구체적 실행1/2/3 결과는 미보고.
- 원형 주변 코드 준비: SpriteShape Rectangle(0)/SoftCircle(1) 선택을 표시 데이터/PS b1로 전달. 기존 padding 일부를 shape에 사용해 SpriteTintConstants48바이트 유지. PSMain은 shape 활성일 때 ComputeShapeAlpha(input.uv) 결과를 기존 color.a에 곱함. 로컬UV를 사용해 아틀라스 선택과 분리.
- 탄환 생성 크기20×20으로 변경(기존12×20). 정사각형 Quad이므로 로컬UV의 원이 화면에서도 원으로 표시. Rectangle 모드와 비교용 키4/5 추가. Player는 Rectangle 기본값 유지.
- 원형 알파 마스크 구현 통과(2026-10-02): `length(localUV - float2(0.5,0.5))`로 중심 거리를 scalar float로 계산하고 `1 - smoothstep(inner, outer, distance)`로 안쪽1/바깥쪽0/중간 부드러운 감소를 구현. fxc PSMain 컴파일 경고 없이 성공.
- Glow/Blend 주변 코드 준비(2026-10-02): SpriteShape에 GlowCircle, SpriteDrawData에 SpriteBlendMode(Alpha/Additive) 추가. Graphics가 일반 Alpha Blend와 Additive Blend State를 모두 소유하고 각 Sprite Draw 직전에 선택. Additive RGB는 `src.rgb * src.a + dst.rgb`. 키6=GlowCircle, 7=Alpha, 8=Additive 비교용으로 연결.
- 현재 사용자 과제: `Triangle.hlsl`의 `ComputeGlowIntensity(float2 localUV)` 구현. 중심에서 1, Glow 반경 밖에서 0이 되며 바깥으로 부드럽게 감소하는 0~1 값을 만들 것. 과제에 필요한 `length`, `saturate`, `pow`의 의미를 설명한 뒤 사용자가 식을 구성. TODO에는 정답식/순서를 적지 않음.
- Glow 완료 조건: 키6 상태에서 탄환이 중심이 가장 밝고 바깥으로 약해지는 분포를 가지며, 같은 Shader 출력에서 키7 Alpha와 키8 Additive가 서로 다른 framebuffer 합성 결과를 보임. Additive에서 배경에 빛이 더해져 더 밝게 보이는 이유를 설명할 수 있음.
- Glow 함수 통과(2026-10-02): 사용자가 중심 거리 / Glow 반경 정규화 → `1 - normalizedDistance` → `saturate` → `pow(..., falloffExponent)`로 0~1 감쇠를 정확히 구현.
- 실행 피드백: 기존 20x20 단일 Quad에서는 Glow가 Quad 밖으로 퍼질 수 없어 작은/투명한 탄환처럼 보였고, 어두운 clear color + alpha 0.5 + `SRC_ALPHA/ONE` Additive 조합에서는 Alpha/Additive 차이도 작게 보임. 실습 표시를 보완해 GlowCircle일 때 3배 크기의 Glow Sprite를 먼저 그리고 원래 크기의 SoftCircle Core를 Alpha Blend로 한 번 더 그리도록 변경. Glow pass에만 키7/8 Blend Mode를 적용.
- Blend 비교 시인성 보완(2026-10-02): 사용자가 의도한 배경은 선 격자가 아니라 밝은 회색/짙은 회색 체크무늬였음. 테스트용 White texture로 64x64 타일을 번갈아 그리는 체크 배경으로 수정. 게임 로직에는 영향 없음. 키6은 Glow 모양만 선택하고 기본 blendMode가 이미 Alpha라서 6→7은 화면 변화가 없는 것이 정상이며, 8에서 Additive로 바뀔 때 차이가 나타남.
- Step 8C 완료(2026-10-02): Glow 함수 구현 통과 및 Alpha/Additive 차이 실행 확인. 사용자 다음 진행 요청으로 Step 8D 시작.
- Step 8D-1 회전 주변 코드 준비: SpriteDrawData에 `rotationRadians`, VS b0 SpriteConstants에 회전값 추가. Graphics가 Draw마다 값을 전송. VS에 `RotateLocalPosition` TODO를 추가하고 회전된 로컬 좌표가 기존 size/center/NDC 경로로 이어지도록 주변 코드 연결. 키9는 Player 회전각 +0.25 rad, 키0은 0으로 초기화.
- 현재 사용자 과제: HLSL `RotateLocalPosition(float2 localPosition, float angleRadians)` 구현. 입력은 Sprite 중심 기준 로컬 좌표이며 출력도 회전 후 로컬 좌표. `sin`/`cos`를 사용하며 외부 API/새 DX 호출은 필요 없음.
- 회전 완료 조건: 9를 누를 때 Player가 자신의 중심을 유지한 채 회전하고, 0에서 원래 방향으로 복귀. 회전을 center를 더한 뒤 적용하면 왜 화면 원점 주변 공전이 되는지 설명할 수 있어야 함.
- 마지막 검증(2026-10-02): Step 8D-1 회전 전송/VS 주변 코드 포함 MSVC Debug 빌드 성공. Windows SDK fxc로 VSMain/PSMain 컴파일 성공. `RotateLocalPosition`은 사용자 과제를 위해 현재 입력을 그대로 반환하는 placeholder 상태.
- Step 8D-1 회전 완료(2026-10-02): 사용자가 회전된 X/Y 기저벡터에 local x/y 스칼라를 곱해 합산하는 구현을 완성. `(x,y)=x(1,0)+y(0,1)`에서 회전행렬이 만들어지는 원리를 이해 확인 후 다음 진행 요청.
- Step 8D-2 Sampler Filter 주변 코드 준비: `SpriteSamplerMode::Point/Linear` 추가. Graphics가 Clamp AddressMode를 공유하는 Point/Linear Sampler State 두 개를 생성하고 Draw마다 선택하는 구조로 변경. Player는 N=Point, L=Linear 선택. 현재 `BindSpriteSampler`는 Point를 기본 바인딩하며 Sampler 선택만 사용자 TODO로 남김.
- 현재 사용자 과제: `Graphics::BindSpriteSampler(SpriteSamplerMode samplerMode)`에서 `samplerMode`에 따라 `pointSampler_` 또는 `linearSampler_`를 선택해 기존 `PSSetSamplers(kSpriteSamplerSlot, 1, &samplerState)`에 전달. 새 DX 호출은 없음.
- Sampler Filter 완료 조건: Player의 2x2 Checker Texture를 크게 표시했을 때 N(Point)은 texel 경계가 딱 끊기고, L(Linear)은 이웃 texel 색이 보간되어 경계가 부드럽게 섞임. Texture/SRV를 바꾸지 않았는데 결과가 달라지는 이유를 설명할 수 있어야 함.
- 마지막 검증(2026-10-02): Step 8D-2 Sampler 주변 코드 포함 MSVC Debug 빌드 성공. TODO 상태에서는 N/L 모두 Point Sampler가 바인딩되어 화면 차이가 없는 것이 정상.
- Step 8D-2 Sampler Filter 완료(2026-10-02): 사용자가 Point/Linear 선택 switch를 구현하고 N/L 실행 차이를 확인. 최초에는 Point case의 `break` 누락으로 항상 Linear까지 fallthrough 되었으나 원인을 직접 찾아 수정.
- Step 8D-3 AddressMode 준비: `SpriteAddressMode::Clamp/Wrap` 추가. Graphics가 Point/Linear × Clamp/Wrap 네 Sampler State를 생성하고 두 선택값에 따라 바인딩. C=Clamp, W=Wrap이며 두 키 모두 Player UV를 (0,0)~(2,2)로 설정해 0~1 밖 샘플링을 바로 비교할 수 있게 함.
- 현재 과제: 실행 전에 `UV=(1.25, 0.25)` 같은 범위 밖 좌표를 읽을 때 Clamp와 Wrap이 각각 어느 위치를 읽을지 자기 말로 예측한 뒤 C/W 결과와 비교. 새 API 구현 과제는 없음.
- AddressMode 완료 조건: C에서는 0~1 밖 UV가 가장자리로 고정되어 Texture 가장자리 색이 늘어나 보이고, W에서는 UV가 주기적으로 되감겨 2x2 Checker가 반복되어 보임. Filter와 AddressMode 역할 차이를 설명할 수 있어야 함.
- 마지막 검증(2026-10-02): Step 8D-3 Clamp/Wrap Sampler State 네 조합과 테스트 입력 연결 후 MSVC Debug 빌드 성공.
- Step 8D 완료(2026-10-02): 사용자가 범위 밖 UV `(1.25, 0.25)`에 대해 처음에는 Clamp/Wrap 결과를 반대로 답했으나, Clamp=`(1.0,0.25)`, Wrap=`(0.25,0.25)`임을 확인하고 다음 진행 요청. 회전, Blend, Point/Linear Filter, Clamp/Wrap까지 8D 범위 종료.
- Step 9 Object Pool 주변 코드 준비: 기존 `std::vector<Bullet>`을 유지하면서 생성 시 256개 슬롯으로 고정. Bullet에 `active` 추가. Update/Render는 inactive를 건너뛰고, 화면 밖 탄환은 erase하지 않고 `active=false`로 반환. 제목의 Bullets 수는 `GetActiveCount()` 사용.
- 현재 사용자 과제: `BulletSystem::Spawn(centerX, centerY)`에서 비활성 슬롯 하나를 찾아 기존 발사 탄환 값 `(centerX, centerY, 0, -480, 20, 20)`으로 다시 초기화하고 활성화. 풀 크기를 늘리거나 `push_back`하지 않는다.
- Object Pool 1차 완료 조건: 발사 시 inactive 슬롯이 재사용되고, 화면 밖 탄환 슬롯이 다시 다음 발사에 사용됨. `bullets_.size()`는 실행 중 256으로 유지되며 활성 탄환 수만 증감. 왜 이것이 기존 push_back/erase 방식과 다른지 설명할 수 있어야 함.
- Spawn 최종 리뷰(2026-10-02): `std::ranges::find(bullets_, false, &Bullet::active)`로 첫 inactive 슬롯을 찾고 기존 발사 기본값으로 덮어쓰도록 수정 완료. inactive 슬롯이 없으면 별도 `push_back`/예외 없이 그대로 반환되어 풀 크기 256 고정 정책을 만족.
- Step 9 완료(2026-10-02): 사용자가 현재 Spawn의 선형 inactive 탐색 비용을 설명하고, 개선안으로 free index stack(Free List)과 active/inactive 영역을 swap하여 유지하는 방식을 스스로 제시. Object Pool이 항상 더 빠른 것이 아니라 할당/삭제 비용과 탐색/순회 비용의 trade-off임을 이해 확인.
- Step 10-1 준비(2026-10-02): 정지 Enemy 1기를 추가하고, BulletSystem에 임의 velocityX/velocityY를 받는 Spawn overload를 추가. Enemy가 1.5초마다 `DanmakuPattern::SpawnRing`을 호출하도록 연결했으며 패턴 함수의 방향 계산/Spawn만 사용자 과제로 남김.
- 현재 사용자 과제: `DanmakuPattern::SpawnRing`에서 `bulletCount`개의 탄환이 원 둘레에 균등한 방향으로 같은 속력 `speedPixelsPerSecond`를 갖도록 각 탄환의 velocity를 만들고 `bulletSystem.Spawn`을 호출한다.
- Step 10-1 완료 조건: 12발이 적 중심에서 전 방향으로 균등하게 퍼지고, 각 탄환의 속력은 같으며, 왜 각도 간격을 균등하게 나누면 원형 패턴이 되는지 설명할 수 있어야 함.
- Step 10-1 구현 리뷰(2026-10-02): `bulletCount`와 `speedPixelsPerSecond` 인자를 사용하도록 수정했고, 각도별 벡터를 정규화한 뒤 지정 속력을 곱해 원형 발사 조건을 만족. 수학적으로는 맞지만 `sin+cos`, `-sin+cos` 후 정규화는 불필요하게 복잡하며 직접 단위 원의 sin/cos 성분을 쓰는 형태가 더 단순함.
- Step 10-1 이해 확인(2026-10-02): 사용자가 `(cos(r), sin(r))`가 `cos^2+sin^2=1` 때문에 별도 정규화 없이 길이 1인 단위 방향 벡터가 된다는 점을 이해했다고 확인.
- Step 10-2 주변 코드 준비: `SpawnRing`에 `startAngleRadians` 입력을 추가하고 Application이 게임 시간 기준으로 `enemyRingAngleRadians_`를 초당 0.8 rad 증가시켜 매 발사에 전달하도록 연결. 현재 패턴 함수에서는 오프셋을 아직 각도 계산에 반영하지 않아 기존 원형 탄막과 동일하게 발사됨.
- 현재 사용자 과제: `DanmakuPattern::SpawnRing`에서 모든 탄환의 균등한 각도 간격은 유지하면서 `startAngleRadians`만큼 원형 패턴 전체 방향을 회전시킨다.
- Step 10-2 완료 조건: 매 1.5초마다 새 원형 탄막의 12개 간격은 그대로 유지되지만 이전 원형과 시작 방향이 조금씩 돌아가며, 개별 탄환의 속력은 여전히 `speedPixelsPerSecond`로 동일해야 함.
- Step 10-2 완료(2026-10-02): 사용자가 각 탄환의 기본 각도 `r`에 `startAngleRadians`를 더한 뒤 `(cos, sin) * speed`로 전달하도록 구현. 균등 간격과 속력은 유지하면서 원형 패턴 전체 방향만 회전하는 구조가 됨.
- Step 10-3 주변 코드 준비: `DanmakuPattern::SpawnFan` 추가. Application은 적이 1.5초마다 아래쪽(`pi/2`)을 중심으로 총 90도(`pi/2`) 범위에 7발을 발사하도록 연결. 현재 `SpawnFan`의 다발 계산은 사용자 TODO 상태라 `bulletCount > 1`에서는 발사하지 않음.
- 현재 사용자 과제: `SpawnFan`에서 `centerAngleRadians`를 중심으로 `spreadAngleRadians` 범위 안에 `bulletCount`개의 각도를 균등하게 배치하고, 각 각도를 기존 `(cos, sin) * speedPixelsPerSecond` 속도 벡터로 바꿔 Spawn한다.
- Step 10-3 완료 조건: 적 기준 아래쪽을 중심으로 90도 범위에 7발이 좌우 대칭이며 균등한 각도 간격으로 퍼지고, 첫 발과 마지막 발이 부채꼴 양 끝 방향을 사용함을 설명할 수 있어야 함.
- Step 10-3 완료(2026-10-02): 사용자가 `t=i/(bulletCount-1)`로 0~1 구간을 만들고 `center-spread/2`에서 `center+spread/2`까지 균등하게 보간해 각 탄환을 `(cos, sin) * speed`로 Spawn하도록 구현. 첫/마지막 탄환이 양 끝 각도를 포함하고 N발 사이에 N-1개의 간격이 생기는 이유까지 확인.
- Step 10-4 주변 코드 준비(2026-10-02): Application이 게임 fixed update에서 `enemyFanAngleRadians_`를 초당 0.7 rad씩 증가시키도록 연결. `SpawnFan` 자체는 그대로 유지하고, 발사 시 사용할 `fanCenterAngle` 계산만 사용자 과제로 남김.
- 현재 사용자 과제: 기본 부채꼴 중심 방향과 `enemyFanAngleRadians_`를 이용해 이번 발사의 `fanCenterAngle`을 정한다. 폭(`spreadAngleRadians`)과 7발의 내부 간격은 바꾸지 않는다.
- Step 10-4 완료 조건: 매 1.5초마다 새 부채꼴 전체 방향이 조금씩 회전하지만 90도 폭과 7발의 균등 간격은 유지되며, 왜 중심 각도 하나만 바꾸면 부채꼴 전체가 회전하는지 설명할 수 있어야 함.
- Step 10-4 완료(2026-10-02): 사용자가 `fanCenterAngle`에 `enemyFanAngleRadians_`를 더하는 구조를 정확히 설명했고 해당 답을 코드에 반영. 기본 아래쪽 방향을 유지하면서 시간 오프셋만 더해 부채꼴 전체가 회전함.
- Step 10 완료(2026-10-02): Enemy, 임의 속도 Bullet Spawn, 원형 탄막, 회전 원형 탄막, 부채꼴 탄막, 회전 부채꼴 탄막까지 완료.
- Step 11-1 주변 코드 준비(2026-10-02): 렌더 Sprite 크기와 별개로 충돌용 `CircleHitbox(centerX, centerY, radius)`와 `Intersects` 함수를 추가. CMake 등록 완료. 실제 Player/Bullet 연결은 원형 충돌 수학을 먼저 이해한 뒤 다음 하위 단계에서 진행.
- 현재 사용자 과제: `Collision.cpp`의 `Intersects`에서 두 원형 히트박스가 겹치는지 판정한다. 새로운 API는 필요 없고 지금까지 사용한 덧셈/뺄셈/곱셈/비교만으로 구현 가능.
- Step 11-1 완료 조건: 두 중심 사이 거리와 두 반지름의 합을 기준으로 겹침 여부를 판정하며, 제곱근을 구하지 않고 제곱값끼리 비교해도 결과가 같은 이유를 설명할 수 있어야 함.
- Step 11-1 완료(2026-10-02): 사용자가 중심 차이와 반지름 합의 제곱 비교를 구현했으나 비교 방향을 `>`로 작성해 비충돌을 true로 반환하는 오류가 있었음. 리뷰 후 에이전트가 `<=`로 수정하여 접하는 경우까지 충돌로 판정하도록 완료.
- Step 11-2 주변 코드 준비(2026-10-02): 같은 Object Pool에 Player/Enemy 탄환이 섞이는 구조이므로 `BulletOwner::Player/Enemy`를 추가하고 Spawn 시 소유자를 명시하도록 정리. Application에 Player Hitbox 반경 5px, Enemy Bullet Hitbox 반경 8px, 적 탄환 필터링 루프와 Debug Overlay의 Player Hit 표시를 연결함.
- 현재 사용자 과제: `Application::CheckPlayerEnemyBulletCollisions()`의 준비된 반복문 안에서 기존 `Intersects(playerHitbox, bulletHitbox)` 결과를 `playerHit_`에 반영한다. 새 API나 새 수학은 필요 없음.
- Step 11-2 완료 조건: Player 탄환은 검사에서 제외되고 Enemy 탄환만 Player와 충돌 판정하며, Sprite 크기와 Hitbox 반경을 별도로 두는 이유 및 BulletOwner 필터가 필요한 이유를 설명할 수 있어야 함.
- Step 11-2 완료(2026-10-02): 사용자가 Enemy 탄환 반복문에서 `Intersects(playerHitbox, bulletHitbox)` 결과를 `playerHit_`에 반영하도록 구현. 기존 true 상태를 false로 되돌리지 않도록 조건부 갱신했으며 논리적으로 정상.
- Step 11-3 주변 코드 준비(2026-10-02): Player 중심에 피격 반경 5px와 별도로 Graze 반경 24px 원을 추가하고 Debug Overlay에 현재 Graze 상태를 표시하도록 연결. 같은 `Intersects`를 재사용하며 실제 Graze 조건만 사용자 과제로 남김.
- 현재 사용자 과제: 현재 Enemy 탄환 하나에 대해 '피격은 아니지만 Graze 원에는 닿는 경우'를 `playerGraze_`에 반영한다. 이미 계산한 `bulletHitsPlayer`, `playerGrazeHitbox`, `bulletHitbox`, `Intersects`만 사용하면 된다.
- Step 11-3 완료 조건: Hit 영역은 Graze로 중복 처리하지 않고, Hit 반경 바깥부터 Graze 반경까지의 영역에서만 Graze가 true가 되며 두 원의 역할 차이를 설명할 수 있어야 함.
- Step 11-3 완료(2026-10-02): 사용자가 `Intersects(playerGrazeHitbox, bulletHitbox) && !bulletHitsPlayer`로 Graze 영역을 정확히 구현. 프레임 내 여러 탄환 결과는 OR 누적으로 유지.
- Step 11-4 주변 코드 준비(2026-10-02): `Bullet`에 수명 동안 유지되는 `grazed` 상태를 추가하고 Object Pool 슬롯 재사용 시 Spawn에서 false로 초기화. Application에 누적 `grazeCount_`와 Debug Overlay 표시를 추가하고 충돌 루프에서 Bullet 상태를 갱신할 수 있도록 연결.
- 현재 사용자 과제: 현재 계산된 `isGraze`와 `bullet.grazed`, `grazeCount_`를 사용하여 같은 탄환은 활성 수명 동안 딱 한 번만 Graze 횟수를 증가시키도록 구현한다. 슬롯이 재사용되면 `grazed=false`로 새 탄환 상태가 시작된다.
- Step 11-4 완료 조건: 한 탄환이 Graze 영역에 여러 프레임 머물러도 카운트는 1만 증가하고, 다른 탄환은 각각 별도로 1회씩 증가하며, 왜 이 상태가 Application의 프레임 상태가 아니라 Bullet의 수명 상태여야 하는지 설명할 수 있어야 함.
- Step 11-4 완료(2026-10-03): 사용자가 `isGraze && !bullet.grazed`에서만 `bullet.grazed=true`와 `grazeCount_++`를 수행하도록 구현. 같은 탄환은 활성 수명 동안 1회만 Graze 처리되고 Object Pool 재사용 시 새 탄환으로 초기화됨.
- Step 11 완료(2026-10-03): 원형 Hitbox, Player/Enemy 탄환 구분, 실제 피격, Graze 영역, 탄환당 1회 Graze 상태까지 구현 완료.
- Step 12-1 준비(2026-10-03): 현재 Player 대 Enemy Bullet 충돌 루프에서 실제 원형 narrow-phase 검사 횟수를 `collisionCheckCount_`로 계측하고 Debug Overlay에 표시. Uniform Grid를 구현하기 전에 현재 비용과 후보 감소 필요성을 관찰하는 기준선으로 사용.
- 현재 사용자 과제: 실행 중 `Bullets`와 `Collision Checks` 값을 비교한 뒤, (1) 왜 현재 구조에서는 대체로 활성 Enemy Bullet 수만큼 충돌 검사를 하는지, (2) Player 한 명과 최대 256발 정도인 현재 규모에서 Uniform Grid가 반드시 더 빠르다고 말할 수 없는 이유를 설명한다. 코딩 과제 없음.
- Step 12-1 완료 조건: Broad Phase는 비싼 정확 충돌 검사 전에 공간적으로 가능성 없는 후보를 제거하는 단계이며, Grid 구축/초기화/셀 삽입/조회 자체에도 비용이 있으므로 객체 수와 질의 수가 적으면 단순 순회가 더 나을 수 있음을 설명할 수 있어야 함.
- Step 12-1 완료(2026-10-03): 사용자가 Uniform Grid가 후보 충돌 계산을 줄이는 대신 각 탄환의 셀 위치 계산/셀 탐색 등 추가 비용을 만든다고 설명. 동적 Grid는 보통 전체 탄환을 한 번 순회해 셀에 재배치해야 하므로, 줄어드는 핵심은 전체 객체 순회 자체보다 이후 narrow-phase 후보 수라는 점을 보완 설명.
- Step 12-2 완료(2026-10-03): 셀 크기 64px 기준으로 (150,70)->(2,1), (63,63)->(0,0), (64,64)->(1,1)을 정확히 계산. 정수 나눗셈 경계에서 다음 셀로 넘어가는 규칙 이해 확인.
- Step 12-3 완료(2026-10-03): 2D 셀 좌표를 1D 배열 인덱스로 바꾸는 식 `cellY * gridWidth + cellX`의 의미를 확인. 반복 손계산 대신 실제 코드 연결을 우선하기로 진행 방식 조정.
- Step 12-4 주변 코드 준비(2026-10-03): `UniformGrid`를 추가하고 `std::vector<std::vector<std::size_t>>`로 셀별 Bullet Pool 인덱스를 저장하도록 구성. 생성 시 전체 셀 수를 만들고 `Clear`, `GetCell`, 2D→1D `ToIndex`는 에이전트가 구현. CMake 등록 완료.
- 현재 사용자 과제: `UniformGrid::Insert(cellX, cellY, bulletIndex)`에서 이미 준비된 `ToIndex`를 이용해 올바른 셀 벡터에 `bulletIndex`를 추가한다. 이전에 사용한 `std::vector::push_back` 외 새 API는 필요 없음.
- Step 12-4 완료 조건: Grid가 Bullet 복사본이 아니라 Pool 인덱스를 저장하는 이유와, 한 셀에 여러 Bullet 인덱스가 들어갈 수 있으므로 셀 자체가 vector인 이유를 설명할 수 있어야 함.
- Step 12-4 완료(2026-10-03): 사용자가 `cells_[ToIndex(cellX, cellY)].push_back(bulletIndex)`로 셀 삽입을 정확히 구현.
- Step 12 보완 학습(2026-10-03): 사용자가 고속 탄환이 한 fixed step 사이 여러 셀을 통과하면 현재 위치만 셀에 넣는 단순 Grid로는 시간축 경로를 표현하지 못하고 tunneling/CCD 문제가 생길 수 있음을 지적. Spatial broad phase와 continuous collision detection을 별도 문제로 구분해 설명하기로 함.
- Step 12 범위 결정(2026-10-03): 현재 프로젝트는 60Hz fixed update와 현재 탄속 범위에서 Discrete Collision + Uniform Grid를 사용. 범용 CCD는 구현 범위에서 제외하고 tunneling 조건/대안(swept test, substep)은 설계 한계로 설명 가능하게 유지.
- Step 12-5 주변 코드 준비(2026-10-03): Application에 64px 셀 크기의 Enemy Bullet Grid를 추가하고 창 resize 시 Grid dimensions도 갱신. 매 fixed update에서 Grid를 Clear한 뒤 활성 Enemy Bullet Pool 슬롯을 순회하는 Rebuild 뼈대를 연결. 화면 밖 중심은 등록에서 제외.
- 현재 사용자 과제: `RebuildEnemyBulletGrid()`에서 준비된 Enemy Bullet의 현재 `x/y`를 `kCollisionGridCellSize`로 나눠 `cellX/cellY`를 만들고, `enemyBulletGrid_.Insert(cellX, cellY, bulletIndex)`로 등록한다. 좌표는 앞에서 배운 셀 좌표 변환만 사용.
- Step 12-5 완료 조건: Grid는 매 fixed update 현재 위치 기준으로 다시 만들어지며, 움직이는 탄환이 이전 셀에 남지 않는 이유로 `Clear -> 재등록` 순서를 설명할 수 있어야 함.
- Step 12-5 완료(2026-10-03): 사용자가 Enemy Bullet의 x/y를 64px 셀 좌표로 변환하고 Pool index를 Grid에 등록. 최초 bounds check에서 rows/columns를 반대로 비교했으나 리뷰 후 `cellX < Columns`, `cellY < Rows` 의미에 맞게 수정.
- Step 12-6 주변 코드 준비(2026-10-03): 기존 전체 Enemy Bullet 순회를 제거하고 Player 중심 셀의 주변 3x3 셀에서만 Bullet Pool index를 꺼내 narrow-phase Hit/Graze 판정을 수행하도록 연결. 현재 Graze 24px + Bullet 8px = 최대 중심거리 32px이고 셀 크기 64px이므로 같은 셀 또는 인접 셀 조회로 후보를 포괄.
- 현재 사용자 과제: `CheckPlayerEnemyBulletCollisions()`의 `playerCellX/playerCellY`를 Player 현재 x/y와 `kCollisionGridCellSize`를 이용해 계산한다. 주변 셀 경계 처리와 후보 순회/충돌 판정은 준비되어 있음.
- Step 12-6 완료 조건: Debug Overlay의 `Collision Checks`가 전체 Enemy Bullet 수가 아니라 Player 주변 3x3 셀의 후보 수를 나타내며, 왜 현재 반경/셀 크기에서는 3x3이면 충분한지 설명할 수 있어야 함.
- Step 12-6 구현 완료(2026-10-03): 사용자가 Player x/y를 64px 셀 좌표로 정확히 변환. 3x3 Grid 셀 후보만 기존 Hit/Graze narrow-phase로 전달하는 코드 완성.
- 계측 명칭 정리(2026-10-03): 기존 `collisionCheckCount_`는 실제 `Intersects()` 호출 횟수가 아니라 Broad Phase가 넘긴 Bullet 후보 수였으므로 `collisionCandidateCount_` / Overlay `Collision Candidates`로 변경. 비교 기준으로 활성 Enemy Bullet 수도 함께 표시.
- 현재 사용자 확인: 실행하여 `Enemy Bullets`가 여러 발 쌓인 상태에서 Player 위치에 따라 `Collision Candidates`가 전체 Enemy Bullet보다 작게 유지/변화하는지 관찰한다.
- Step 12 완료(2026-10-03): 사용자가 Uniform Grid가 전체 적탄이 아니라 Player 주변 3x3 셀의 탄환만 narrow-phase 후보로 넘겨 `Collision Candidates`를 줄인다는 원리를 설명. Grid rebuild, 셀 인덱싱, 3x3 query, 비용/오버헤드 trade-off, CCD와의 역할 차이까지 이해 확인 완료.
- Step 13 시작(2026-10-03): Title / Game / Result를 명시적 Game State로 관리하는 구조 학습으로 전환. 첫 과제는 상태를 enum으로 표현하는 이유와 상태별 Update/Render 책임 차이를 이해하는 것.
- Step 13-1 완료(2026-10-03): 사용자가 여러 bool로 상태를 표현하면 둘 이상이 동시에 true가 되는 잘못된 조합이 가능하지만, 단일 `GameState` 값은 현재 상태를 하나로 제한해 모순 상태를 방지한다고 설명.
- Step 13-2 완료(2026-10-03): 사용자가 상태 변경이 여러 위치에 흩어지면 무엇이 언제 어떻게 상태를 바꿨는지 추적하기 어려워진다고 설명. 중앙 전환 함수는 허용 규칙과 전환 시 부수 작업을 한곳에서 관리하기 위한 통로로 사용.
- Step 13-3 주변 코드 준비(2026-10-03): `GameState { Title, Playing, Result }`와 `gameState_`를 추가. Enter 입력은 Title->Playing, Result->Title 전환을 요청하고, Playing 중 Player Hit은 Result 전환을 요청하도록 연결. Playing 상태에서만 fixed game update/pause가 동작하며 Debug Overlay에 현재 State를 표시.
- 현재 사용자 과제: `ChangeGameState(GameState nextState)`에서 허용된 전환 `Title -> Playing -> Result -> Title`만 실제 `gameState_` 변경으로 반영한다. 같은 상태 유지나 역방향/건너뛰기 전환은 거부한다. 구현 방식(if/switch 등)은 자유.
- Step 13-3 완료 조건: 세 허용 전환만 성공하고, 예를 들어 Title->Result나 Result->Playing은 거부되는 이유를 상태 전환 규칙 관점에서 설명할 수 있어야 함.
- Step 13-3 1차 리뷰(2026-10-03): `nextState` 기준 switch에서 Title->Playing과 Playing->Result는 정확히 구현했지만 `case GameState::Title`이 무조건 return하여 Result->Title 전환이 막혀 있음. Title case에서도 현재 상태가 Result인지 검사해 허용 전환만 반영하도록 수정 필요.
- Step 13-3 완료(2026-10-03): Result->Title 누락을 에이전트가 보완. `ChangeGameState`가 세 허용 전환만 검증하고 성공 시 단 한 번 `gameState_`를 변경하도록 정리.
- Step 13-4 주변 코드 준비(2026-10-03): 상태 변경 후 부수 작업을 `EnterGameState`로 분리. 새 게임 초기화를 위해 `Player::Reset()`과 `BulletSystem::Reset()`을 추가했으며, 기존 Object Pool의 256개 슬롯 자체는 유지하고 각 Bullet 상태만 기본값으로 되돌림.
- 현재 사용자 과제: `EnterGameState(GameState state)`의 Playing 진입 블록에서 새 게임 시작에 필요한 상태를 초기화한다. 사용 가능한 초기화는 `player_.Reset()`, `bulletSystem_.Reset()`과 Application의 gameplay 카운터/시간/일시정지 상태들이다. 어떤 값을 새 게임마다 초기화해야 하는지 스스로 골라 작성.
- Step 13-4 완료 조건: Result에서 Title로 돌아갔다가 새 Playing을 시작해도 이전 판의 Player 위치, 탄환, Graze/Shot 카운터, pause/게임 시간 등이 새 판에 새어 들어오지 않으며, 왜 이 초기화를 전환 호출부가 아니라 Playing 진입 한 곳에 모으는지 설명할 수 있어야 함.
- Step 13-4 완료(2026-10-03): 사용자 요청에 따라 에이전트가 Playing 진입 초기화 구현. Player/Bullet/Grid, game time/accumulator, 적 발사 상태, pause, Hit/Graze/카운터/계측을 새 판 기준으로 초기화하고 FPS/실시간/Debug Overlay/렌더 테스트 설정은 프로그램 전역 표시 상태로 유지.
- Step 13-5 시작(2026-10-03): Update는 이미 Playing 상태에서만 수행되지만 Render는 현재 상태와 무관하게 gameplay 전체(Player/Enemy/Bullet)를 항상 그림. 다음 학습은 상태별 Render 책임 분리.
- Step 13-5 주변 코드 준비(2026-10-03): 기존 `Render()`의 체크 배경, gameplay, debug overlay를 `RenderCheckerBackground()`, `RenderPlaying()`, `RenderDebugOverlay()`로 분리하고 `RenderTitle()` / `RenderResult()` 진입점을 추가. Result는 현재 마지막 gameplay 화면을 그대로 그려 정지된 결과 화면의 기반으로 사용하며, 상태별 실제 선택 부분만 사용자 과제로 남김.
- 현재 사용자 과제: `Application::Render()`의 TODO에서 `gameState_`에 따라 `RenderTitle()`, `RenderPlaying()`, `RenderResult()` 중 맞는 함수를 호출한다. 새 API는 사용하지 않는다.
- Step 13-5 완료 조건: Title에서는 gameplay 객체가 보이지 않고, Playing에서는 정상 gameplay가 보이며, Result에서는 마지막 gameplay 화면이 보이지만 Update가 멈춰 탄환/플레이어가 더 움직이지 않는 이유를 설명할 수 있어야 함.
- 마지막 검증(2026-10-03): Step 13-5 렌더 함수 분리 후 MSVC Debug 빌드/링크 성공. `clangd --check=src/Application.cpp --enable-config`는 C++ 컴파일 진단이 아니라 `ExtractFunction` tweak self-test 8건만 실패.
- Step 13-5 완료(2026-10-03): 사용자가 `gameState_` switch에서 Title/Playing/Result별 렌더 함수를 선택하도록 구현. Result에서도 `RenderPlaying()`은 계속 호출되지만 `RunFixedUpdates()`가 Playing에서만 실행되므로 게임 상태가 갱신되지 않아 마지막 장면이 정지 상태로 다시 그려진다는 점을 설명.
- Step 13 완료(2026-10-03): 단일 `GameState`, 중앙 전환 검증, 상태 진입 초기화, Playing 전용 Update, 상태별 Render 책임 분리까지 완료. 사용자 구현 포함 MSVC Debug 빌드/링크 성공.
- Step 14 시작(2026-10-03): 현재 `Graphics`가 `vertexShader_`, `pixelShader_`, `spriteTextureViews_` 등 GPU 리소스를 직접 소유하는 구조를 기준으로 Resource Management 학습 시작. 첫 주제는 Resource 생성/소유/조회/바인딩 책임과 cache가 필요한 이유.
- Step 14-1 개념 확인(2026-10-03): 사용자가 `ID3D11ShaderResourceView`의 `ComPtr`는 `TextureCache`가 소유하고 Graphics는 렌더링 책임에 집중해야 한다고 설명. 보완 사항으로 Bullet은 gameplay 상태를 소유하고 `SpriteDrawData`는 렌더 직전에 구성하는 임시 표시 데이터로 유지.
- Step 14-1 주변 코드 준비(2026-10-03): `TextureCache`를 추가하고 기존 `Graphics::CreateSpriteTextures()` 및 `spriteTextureViews_` 소유권을 캐시로 이동. `Graphics::BindSpriteTexture()`는 캐시에서 raw SRV pointer를 빌려 `PSSetShaderResources`에 바인딩하도록 변경. Texture ID 유효성 검사도 캐시 조회 쪽으로 이동.
- 현재 사용자 과제: `TextureCache::Get(SpriteTextureId id)`의 TODO에서 이미 계산된 `index`를 사용해 해당 `ComPtr`가 소유한 raw `ID3D11ShaderResourceView*`를 반환한다. 범위 검사는 주변 코드에 이미 구현되어 있음.
- Step 14-1 완료 조건: `ComPtr` 소유권은 TextureCache에 남고 Graphics가 받는 raw pointer는 수명을 소유하지 않는 borrowed pointer라는 점, 그리고 동일 Texture를 여러 Sprite가 공유해도 GPU 리소스는 한 번만 생성되는 이유를 설명할 수 있어야 함.
- 마지막 검증(2026-10-03): TextureCache 분리 및 Get placeholder 상태에서 MSVC Debug 빌드/링크 성공. 일반 sandbox 빌드는 기존과 동일하게 `C:\Users\Yuchan\AppData\Local\Microsoft SDKs` 접근 거부로 실패하고 권한 포함 빌드는 성공.
- Step 14-1 완료(2026-10-03): 사용자가 `textures_`가 `TextureCache`의 멤버로 유지된다는 점을 근거로 raw pointer 반환 후에도 리소스가 즉시 파괴되지 않는 이유를 설명하고 `return textures_[index].Get();`으로 구현. 보완 설명으로 실제 수명 근거는 멤버 자체보다 `ComPtr`가 유지하는 COM reference count이며, `Get()`은 ownership을 넘기지 않는 borrowed pointer임을 확인. 수정 코드 MSVC Debug 빌드/링크 성공.
- Step 14-2 시작(2026-10-03): 현재 TextureCache는 Initialize 시 모든 Texture를 한 번 생성하는 eager cache다. 다음 학습은 cache hit/miss와 같은 Texture 요청의 중복 GPU Resource 생성을 막는 이유, eager와 lazy 생성 정책의 trade-off.
- Step 14-2 완료(2026-10-03): 사용자는 100개 중 실제 12개 정도만 사용하는 조건이라면 초기 로딩 시간과 불필요 리소스 생성을 줄이기 위해 Lazy를 선택하겠다고 설명. 첫 사용 시 로딩 hitch 가능성과 Stage 진입 전 preload 절충안도 함께 학습.
- Step 14-3 시작(2026-10-03): 실제 파일 Texture 로딩을 위해 WIC(Windows Imaging Component)를 도입. 직접 PNG 포맷을 파싱하지 않고 WIC Decoder/Frame/FormatConverter로 RGBA8 픽셀을 얻은 뒤 기존 DX11 `CreateTexture2D` -> `CreateShaderResourceView` 경로로 GPU 리소스를 생성하도록 주변 코드 구현.
- 테스트 Asset: `assets/player_test.png` 64x64 투명 PNG 추가. 방향/투명도 확인이 쉬운 단순 진단용 Sprite이며 CMake가 실행 파일 옆 `assets` 폴더로 복사.
- WIC/COM 주변 코드: `wWinMain`에서 COM apartment 초기화/해제, `TextureCache`가 WIC factory 및 D3D11 device를 보유. `SpriteTextureId::Player` 추가 후 Player 렌더가 파일 기반 Texture ID를 사용하도록 연결. `windowscodecs` 링크 추가.
- 현재 사용자 과제: `TextureCache::Get()`에서 해당 슬롯이 비어 있는 cache miss일 때 이미 준비된 `GetTexturePath(id)`와 `LoadTextureFromFile(...)`을 사용해 SRV를 `textures_[index]`에 저장한다. cache hit에서는 기존 SRV를 그대로 반환해야 함. 새로운 WIC API를 직접 호출할 필요 없음.
- Step 14-3 완료 조건: 첫 Player texture 요청에서만 PNG decode/GPU Texture 생성이 일어나고 이후 Draw에서는 같은 `ComPtr` SRV가 재사용되며, `LoadTextureFromFile` 내부의 WIC decode 단계와 D3D11 upload 단계 역할을 구분해서 설명할 수 있어야 함.
- 마지막 검증(2026-10-03): WIC loader 주변 코드와 PNG asset copy 설정, `TextureCache::Get` TODO 상태까지 MSVC Debug 빌드/링크 성공.
- Step 14-3 완료(2026-10-03): 사용자가 cache miss에서 `textures_[index] = LoadTextureFromFile(GetTexturePath(id));`를 구현하고 실제 PNG Player Sprite 출력 성공을 확인. 이후 동일 ID 요청은 WIC decode와 GPU Texture 생성을 반복하지 않고 저장된 SRV를 재사용.
- Step 14-4 시작(2026-10-03): Texture와 같은 Resource ownership 관점을 Shader에도 적용. 현재 `Graphics::CreateTriangleResources()`가 HLSL 파일 탐색, VS/PS 컴파일, Shader 객체 생성, InputLayout 생성까지 모두 담당하고 있어 Shader Resource 생성/소유 책임을 분리할 여지가 있음.
- Step 14-4 책임 분석(2026-10-03): 사용자는 HLSL 컴파일과 Vertex/Pixel Shader 생성까지 ShaderCache 책임으로 두고 Vertex/Index Buffer는 Graphics 책임으로 남기는 것이 맞다고 설명. InputLayout은 Vertex Shader bytecode signature와 Vertex format 양쪽에 연결되므로 현재는 Graphics 소유로 유지하기로 함.
- Step 14-4 명칭 정리(2026-10-03): 초기 삼각형 실습 흔적인 `Triangle.hlsl`, `CreateTriangleResources`, `BindTrianglePipeline`을 각각 `Sprite.hlsl`, `CreateSpriteResources`, `BindSpritePipeline`으로 변경. 현재 렌더러가 4정점/6인덱스 Sprite Quad를 그린다는 실제 역할에 맞춤.
- Step 14-4 주변 코드 준비(2026-10-03): `ShaderCache` 추가. Sprite HLSL 컴파일, VS/PS 객체 생성 및 수명은 ShaderCache가 담당하며, InputLayout 생성에 필요한 Sprite VS bytecode도 캐시가 소유하고 Graphics가 빌려 사용. Graphics의 기존 `vertexShader_`/`pixelShader_` 소유 멤버 제거.
- 현재 사용자 과제: `Graphics::BindSpritePipeline()`에서 현재 `nullptr`로 바인딩 중인 Vertex/Pixel Shader를 `shaderCache_`가 제공하는 Sprite Shader로 교체한다. 기존 VS/PS 바인딩 API는 이전 Step에서 사용한 것과 동일하다.
- Step 14-4 완료 조건: ShaderCache가 Shader의 생성/수명을 소유하고 Graphics는 Draw 시 raw pointer를 빌려 바인딩한다는 점, 반면 Vertex/Index Buffer와 InputLayout은 현재 Graphics에 남아 있는 이유를 설명할 수 있어야 함.
- 마지막 검증(2026-10-03): ShaderCache 분리, Sprite 명칭 변경, CMake shader copy 경로 변경 후 MSVC Debug 빌드/링크 성공. 과제 완료 전에는 `BindSpritePipeline()`이 VS/PS를 `nullptr`로 바인딩하므로 Sprite 렌더 출력은 정상적으로 나오지 않는 상태.
- Step 14-4 완료(2026-10-03): 사용자가 `BindSpritePipeline()`에서 `shaderCache_.GetSpriteVertexShader()` / `GetSpritePixelShader()`를 기존 `VSSetShader` / `PSSetShader`에 연결. TextureCache와 동일하게 Resource 소유권은 Cache에 두고 Graphics는 raw pointer를 빌려 Pipeline에 바인딩하는 구조 완성. MSVC Debug 빌드/링크 성공.
- Step 14 완료(2026-10-03): 실제 PNG WIC 로딩 + Lazy Texture Cache, Sprite Shader compile/create ownership 분리, VS bytecode 재사용, Graphics의 bind 책임 분리까지 구현 완료. 레거시 Triangle 명칭도 Sprite 기준으로 정리.
- Step 15 시작(2026-10-03): Audio(BGM/SFX) 학습으로 이동. 첫 단계는 XAudio2의 Engine/Mastering Voice/Source Voice/PCM data 역할과 수명 관계를 이해하는 것.
- Step 15-1 완료(2026-10-03): 사용자가 SourceVoice가 재생 위치/버퍼 큐/재생 상태를 가지므로 동시 재생 시 여러 재생 인스턴스가 필요하고, 동일 PCM 원본 데이터는 여러 SourceVoice가 공유할 수 있다는 점을 이해.
- Step 15-2 주변 코드 준비(2026-10-03): `AudioSystem` 추가. XAudio2 Engine + MasteringVoice 초기화, 테스트용 PCM WAV(`assets/shot_test.wav`) 로딩, 같은 PCM을 공유하는 Shot SourceVoice 8개 Pool 생성. idle voice는 `BuffersQueued == 0`으로 판별하고 재생 요청 시 해당 voice에 동일 PCM buffer를 제출. Pool이 모두 사용 중이면 현재 첫 구현은 해당 요청을 건너뜀.
- 테스트 Audio Asset: 짧은 `shot_test.wav`를 프로젝트 assets에 추가하고 CMake가 실행 파일 옆 assets 폴더로 복사. `xaudio2` 라이브러리 링크 추가.
- 현재 사용자 과제: `Application::Update()`의 Player 발사 성공 블록에서 이미 준비된 `audio_.PlayShot()`을 호출해 실제 Shot SFX를 연결한다. 새로운 XAudio2 API를 직접 호출할 필요 없음.
- Step 15-2 완료 조건: Z 발사 성공 시에만 SFX가 들리고, 단순 키 입력 프레임마다 무조건 재생되지 않으며, PCM 1개를 SourceVoice 여러 개가 공유하는 이유를 설명할 수 있어야 함.
- 마지막 검증(2026-10-03): AudioSystem/WAV Loader/SourceVoice Pool/asset copy 설정 포함 MSVC Debug 빌드/링크 성공. Player 발사 이벤트의 `audio_.PlayShot()` 연결만 사용자 과제로 남김.
- Step 15-2 완료(2026-10-04): Player의 실제 발사 성공 블록에 `audio_.PlayShot()` 연결 완료. Shot SFX는 키 입력 자체가 아니라 `UpdateShooting()`이 실제 발사를 허용한 순간에만 재생.
- Step 15-3 개념 확인(2026-10-04): BGM은 현재 프로젝트에서 동시에 여러 트랙을 겹쳐 재생하지 않으므로 전용 SourceVoice 하나를 재사용하는 구조를 선택. 크로스페이드가 필요해지면 별도 Voice를 추가할 수 있음.
- Step 15-3 주변 코드 준비(2026-10-04): `AudioSystem`에 전용 BGM PCM/SourceVoice와 `PlayBgm()` / `StopBgm()` 추가. `PlayBgm()`은 동일 PCM buffer를 `XAUDIO2_LOOP_INFINITE`로 제출해 반복 재생하고 중복 Start 요청은 무시. `StopBgm()`은 Voice를 Stop한 뒤 Queue를 비워 다음 재생이 처음부터 시작되도록 구성. 테스트용 `assets/bgm_test.wav` 추가 및 실행 폴더 복사 설정 완료.
- 현재 사용자 과제: `Application::EnterGameState()`에서 Playing 진입 시 `audio_.PlayBgm()`, Result 진입 시 `audio_.StopBgm()`을 호출해 상태와 BGM 제어를 연결한다. 새 XAudio2 API는 직접 사용하지 않는다.
- Step 15-3 완료 조건: Title에서는 BGM이 없고, Playing 진입에서 반복 BGM이 시작되며, Result 진입에서 멈춘다. BGM이 Shot SFX처럼 Voice Pool을 쓰지 않는 이유를 설명할 수 있어야 함.
- Step 15-2 완료(2026-10-04): Player 실제 발사 성공 블록에 `audio_.PlayShot()` 연결 확인. Shot PCM은 공유하고 idle SourceVoice를 Pool에서 골라 겹쳐 재생하는 구조 유지. 사용자 연결 코드 포함 MSVC Debug 빌드/링크 성공.
- Step 15-3 시작(2026-10-04): BGM은 짧은 SFX처럼 매 이벤트마다 여러 Voice를 풀링하기보다, 보통 한 트랙당 장기 재생 SourceVoice 하나를 유지하고 loop/stop 전환을 관리하는 구조로 학습 진행.
- 마지막 검증(2026-10-02): Step 11-4 주변 코드 추가 후 MSVC Debug 빌드/링크 성공. 탄환당 1회 Graze 조건 구현만 사용자 과제로 남아 있음.
- 마지막 검증(2026-10-02): Step 11-3 Graze 주변 코드 추가 후 MSVC Debug 빌드/링크 성공. 실제 Graze 조건 반영은 사용자 과제로 남아 있음.
- 마지막 검증(2026-10-02): Step 11-1 비교 연산 수정 및 Step 11-2 BulletOwner/충돌 연결 주변 코드 추가 후 MSVC Debug 빌드/링크 성공. `CheckPlayerEnemyBulletCollisions()`의 실제 Intersects 결과 반영은 사용자 과제로 남아 있음.
- 마지막 검증(2026-10-02): Step 10-4 답 반영 및 Step 11-1 Collision 뼈대 추가 후 MSVC Debug 빌드/링크 성공. `Intersects`는 사용자 과제를 위해 현재 `false`를 반환하는 placeholder 상태.
- 마지막 검증(2026-10-02): Step 10-4 주변 코드 준비 후 MSVC Debug 빌드/링크 성공. `fanCenterAngle`은 아직 기본 중심각만 사용하므로 사용자 과제 완료 전에는 부채꼴이 회전하지 않는 것이 정상.
- 마지막 검증(2026-10-02): Step 10-3 `SpawnFan` 주변 코드와 Application 테스트 연결 포함 MSVC Debug 빌드/링크 성공. TODO 상태에서는 7발 부채꼴이 아직 발사되지 않는 것이 정상.
- 마지막 검증(2026-10-02): Step 10-2 회전 오프셋 주변 코드 포함 MSVC Debug 빌드/링크 성공. TODO 상태에서는 `startAngleRadians`가 아직 방향 계산에 반영되지 않아 원형 패턴이 회전하지 않는 것이 정상.
- 마지막 검증(2026-10-02): 수정된 `SpawnRing` 포함 MSVC Debug 빌드/링크 성공.
- 마지막 검증(2026-10-02): Step 10-1 Enemy/DanmakuPattern/BulletSystem 임의 velocity Spawn 주변 코드 포함 CMake 재생성 및 MSVC Debug 빌드/링크 성공. `SpawnRing`은 사용자 과제를 위해 현재 발사하지 않는 placeholder 상태.
- Debug Overlay 준비(2026-10-02): Direct2D + DirectWrite를 DX11 Back Buffer에 연결. 좌상단 패널에 FPS, 활성탄/Pool 수, Game/Real time, Pause, Sampler/Address/Shape/Blend, 현재 조작키를 표시하고 F1로 토글하도록 구성.
- Debug Overlay 검증(2026-10-02): CMake 재생성 포함 MSVC Debug 빌드/링크 성공. 자동 GUI 실행 스모크 테스트는 실행 제어 요청이 차단되어 실제 화면 출력은 사용자 실행 확인이 필요.
- 마지막 검증(2026-10-02): Step 9 주변 코드의 `Application.cpp`/`BulletSystem.cpp` 컴파일은 성공했으나, 실행 중인 `DanmakuShooter.exe`(PID 6752)가 출력 파일을 점유해 링크가 `LNK1168`로 중단됨. 코드 컴파일 오류는 확인되지 않았고 최종 링크 재검증은 실행 종료 후 필요.
- 원형 완료 조건: 키4는 네모, 키5는 중심을 유지하며 바깥쪽으로 투명해지는 원형 탄환. 기존 tint alpha0.5/맥동/pause 유지. 모서리는 투명. 표시는 바뀌지만 Vertex/Index Buffer와 게임 위치·속도는 공유/유지.
- 원형 1차 리뷰: 사용자 `float2 v = 1 - length(localUV - float2(0.5,0.5)); return smoothstep(inner,outer,v);` 작성. 거리 자체를 반전해 반경0.5에서 마스크가1로 나와 요구값0과 다름. length 결과는 float 하나인데 float2로 선언해 같은 값이 두 성분에 확장됨. 거리 계산/자료형과 범위 전환 방향을 구분하는 힌트 제공, 정답 코드는 아직 제공하지 않음. 사용자가 pulse 적용을 주석 처리했으며 테스트를 위한 변경으로 유지.
- UV 완료 조건: 1 전체 네 색, 2 Player 전체가 빨강(맥동 유지), 3 왼쪽 위 초록/오른쪽 위 빨강/왼쪽 아래 흰색/오른쪽 아래 파랑. 크기·위치·발사·time/tint 동작 유지. UV 범위 선택과 화면 크기가 다른 이유 설명.
- 현재 핵심: 시간(s)→라디안 위상(2πt/T), sin의 -1~1 범위를 0~1로 변환, 구간 [a,b]로 매핑(a+(b-a)u). 선택하는 시간 정책은 Sprite마다 달라도 같은 Shader/함수 사용. 프레임 시간은 프레임당 전송, Sprite tint/선택 값은 Draw당 전송.
- 시간 과제 완료 조건: Player/Bullet 밝기가 2초 주기로 반복, P pause 시 Player 맥동 지속/Bullet 맥동과 이동 정지, resume 후 중지 시간 몰아서 따라잡지 않음. 제목 Game/Real 누적값 확인. alpha를 바꾸지 않아 기존 반투명 유지. 사용자 실행 확인과 시간 선택 이유 설명.
- 완료된 Tint 과제 조건: HLSL VS/PS 컴파일, C++ 빌드 정상. 색/투명도 출력과 tint 모두1 복원 테스트의 구체적 실행 보고는 미확인. 현재는 위 시간 과제 조건 적용.
- 현재 구현 상태: 시간·UV·원형 알파 마스크 계산 완성. 키4 Rectangle / 키5 SoftCircle 비교의 실제 화면 확인은 아직 사용자 보고가 없음. `ComputePulse` 적용은 모양 테스트를 위해 현재 주석 처리 상태.
- 빌드 보완: HLSL만 수정해도 CopyShaders 타겟이 실행 폴더에 최신 소스를 복사하도록 POST_BUILD 방식을 변경. executable 재링크가 없어도 반영. 원본/복사 HLSL SHA256 일치 확인.
- 마지막 검증(2026-10-02): Glow/Blend 주변 코드 포함 MSVC Debug 빌드 성공. Windows SDK fxc로 VSMain/PSMain 컴파일 성공. clangd Application.cpp 0 errors. Graphics.cpp의 clangd check는 코드 진단이 아니라 ExtractFunction tweak self-test 2건만 실패. 실제 Glow 화면 비교는 사용자 과제 구현 후 확인.
- 다음 학습: Step 9 Object Pool 슬롯 재사용 → 풀 포화 정책 → 실제 비용 비교. 이후 Step 10 Enemy / Danmaku Pattern.
- 실행: `.\build.ps1`.

## Step 8B 인수인계 이력 — 2026-10-01

아래는 Step 8B 당시 상태이며, 현재 과제는 위의 Step 8C 인수인계를 따른다.

- 현재 Step: **8B — Texture 교체와 선택**, 진행 중. Step 0~8A 학습/구현 완료.
- 8A 이해 확인: 사용자 동일 Quad VB 공유, 게임 쿨다운은 Graphics 책임 아님, 다른 Texture 선택은 Vertex Buffer와 별개라는 관계 설명. 통과.
- 준비 완료: SpriteTextureId::Checker(0)/Yellow(1), SpriteDrawData.texture 추가. Graphics가 2×2 RGBA8 Texture 두 개와 SRV 배열 spriteTextureViews_[2] 생성. 공통 Sampler/Quad/Shader 공유.
- 현재 사용자 코드의 Checker 알파는 모두 255였으며 그대로 유지. Yellow는 RGBA (255,220,40,255) 네 texel. Texture 데이터는 파일 로딩 없이 C++에서 생성해 바인딩 학습에 집중.
- Application은 Player→Checker, Bullet→Yellow 선택. 렌더러는 게임 타입 대신 텍스처 ID를 받음.
- DrawSprite는 ID 유효성을 검사하고 BindSpriteTexture(sprite.texture)를 매 Draw 전에 호출. 기존 파이프라인의 고정 SRV 바인딩은 제거. Invalid ID 예외는 주변 코드에서 처리.
- 현재 핵심: CPU 배열 인덱스 0/1은 텍스처 선택 번호이고, PS t0는 선택한 SRV를 연결하는 GPU 슬롯. Yellow가 ID 1이라고 t1에 연결하지 않음. HLSL의 spriteTexture는 여전히 register(t0).
- 현재 코딩 과제: `Graphics::BindSpriteTexture(SpriteTextureId texture)` 구현. enum→std::size_t 변환으로 해당 배열 원소를 선택하고 PSSetShaderResources로 PS t0에 SRV 하나 연결. GetAddressOf는 보관 중인 raw pointer의 주소, Get은 raw pointer 값이라는 차이를 사전 설명.
- 완료 조건: 올바른 SRV 선택과 슬롯/개수/포인터 연결, 빌드/정적 분석 통과. 실행 시 Player는 네 색 Checker, Bullet은 노란색. Application 표시 데이터의 texture를 교체하면 VB 수정 없이 이미지가 교체됨을 확인.
- 과제 미구현 상태: SRV가 아직 바인딩되지 않아 Player/Bullet이 보이지 않을 수 있음. 생성/이동/발사 코드는 유지.
- 마지막 검증: MSVC Debug 빌드 성공, clangd Graphics.cpp/Application.cpp 각각 0 errors. 시각 검증은 과제 구현 후 진행.
- 다음 Step: 8C Shader Tint/Alpha/UV 실습. 아직 시작하지 않음.
- 실행: `.\build.ps1`.

## Step 8A 인수인계 이력 — 2026-10-01

아래는 Step 8A 당시 상태이며, 현재 과제는 위의 Step 8B 인수인계를 따른다.

- 현재 Step: **8A — Sprite 렌더링 구조**, 학습/구현 완료. Step 8B 준비 대기.
- 사용자 요구: Graphics/DX/Win32 기반 우선. 기본 자료구조/최적화보다 렌더링 객체 구조, 텍스처 교체, 셰이더 실습을 먼저 진행하도록 로드맵 8A~8D 추가. 사용자 동의 완료.
- 준비 완료: `src/SpriteDrawData.h`에 center/size 두 float2 배열을 가진 CPU 표시 데이터 추가. GPU 객체/게임 로직을 소유하지 않음.
- Application::Render가 Player/Bullet 게임 상태에서 SpriteDrawData를 만들고 Graphics::DrawSprite(const SpriteDrawData&)에 전달. Graphics는 Player/Bullet을 직접 참조하지 않음.
- 기존 사용자 SetSpriteTransform 구현은 유지하고 Graphics 내부 함수로 이동. DrawSprite가 이 함수와 Constant Buffer 갱신/DrawIndexed를 호출. 기존 픽셀 변환/프레임 Clear/Present 방식 유지.
- 현재 핵심: 게임 상태(Player/Bullet), 그리는 데 필요한 CPU 데이터(SpriteDrawData), DX11 GPU 리소스(Graphics)의 책임 구분. 화면 크기/패딩을 담는 GPU 전송용 SpriteConstants와 호출자 표시 데이터는 서로 다른 목적. 현재 DrawSprite는 데이터 참조를 호출 이후 보관하지 않음.
- 현재 과제: 문답 두 개. (1) 플레이어/탄환/장식이 같은 Quad와 텍스처를 쓸 때 객체마다 Vertex Buffer/SRV가 필요한지와 개별 데이터가 무엇인지. (2) 발사 쿨다운을 SpriteDrawData/Graphics에 넣는 설계가 적절한지와 이유.
- 8A 문답 리뷰: 사용자 Quad Vertex Buffer는 동일하므로 공유 가능하다고 설명, 쿨다운은 게임 로직이므로 Graphics 책임이 아니라고 설명. 두 내용 통과. SRV 공유 여부는 모르겠다고 답해 보완 설명: 같은 Texture를 같은 방식으로 읽으면 SRV도 공유 가능, 위치/크기는 Constant Buffer로 개별 전달. 다른 Texture를 읽게 하려면 그 Texture를 참조하는 SRV를 선택/바인딩해야 함.
- 현재 이해 확인: Player와 Bullet이 서로 다른 Texture를 사용할 때 기존 Quad Vertex Buffer를 유지해도 되는지, PS의 t0에 어떤 View를 선택해야 하는지와 이유 설명. SRV/Texture 선택 이해 확인 후 8B로 이동.
- 8A 최종 이해 확인 통과: 사용자가 서로 다른 텍스처여도 Vertex Buffer는 별개라고 설명. 현재 Quad의 위치/UV는 공유하고 읽을 Texture는 SRV 바인딩으로 선택한다는 관계 확인. 현재 과제 없음, 다음은 8B 텍스처 선택/교체 코드 준비.
- 완료 조건: 공통 GPU 리소스와 개별 표시 데이터, 게임 상태와 렌더링 상태를 자기 말로 구분. 현재 단원에 사용자 코딩 TODO는 없음. 다음 단원의 텍스처 바인딩 핵심은 사용자 과제로 진행.
- 다음 Step: 8B Texture 교체/선택. 텍스처 선택 필드는 이번에 선행 추가하지 않음.
- 마지막 검증: CMake Debug 설정/MSVC Debug 빌드 성공. clangd Graphics.cpp/Application.cpp 각각 0 errors. 구조 변경 후 화면 실행 확인은 미수행.
- 실행: `.\build.ps1`.
- 사용자 선호: 필요한 개념/API/인자를 먼저 설명하고 그 범위에서 핵심 구현을 과제로 남김. 주변 구조/기계적 작업은 에이전트가 수행. 한 주제 설명/과제/검증을 한 번에 제공.

## Step 8 인수인계 이력 — 2026-10-01

아래는 Step 8 당시 상태이며, 현재 과제는 위의 Step 8A 인수인계를 따른다.

- 현재 Step: **8 — Bullet System**, 학습/구현 완료. Step 0~8 학습/구현 통과, Step 8A 준비 대기.
- Step 7 최종 코드 확인: `shotCooldownSeconds_ = std::max(shotCooldownSeconds_, 0.0);`로 보정 수정 확인. 입력/간격/대기 중 재입력 정책 코드 통과. 사용자가 수정 후 바로 다음 진행을 요청함. 구체적 실행 결과와 dt 관련 문답은 미보고이며 시각 검증을 완료했다고 해석하지 않음.
- 준비 완료: Bullet/BulletSystem 파일, std::vector<Bullet> 소유, Spawn(push_back), 화면 밖 탄환 RemoveOutside(erase_if), Z키 발사 요청의 실제 Spawn 연결, CMake 등록.
- Bullet 데이터: 중심 x/y, velocityX/Y(이미 방향과 속도를 합친 부호 있는 px/s), 전체 width/height. 기본 생성 속도 (0,-480)px/s, 크기 12×20, 생성 중심은 플레이어 위쪽 가장자리.
- 렌더 준비: Graphics를 BeginFrame / DrawSprite / EndFrame으로 분리. 프레임당 Clear/Present 한 번, Player와 각 Bullet마다 Constant Buffer 갱신/DrawIndexed. 같은 테스트 텍스처를 공유. 기존 사용자 SetSpriteTransform 구현은 그대로 사용.
- 구현 리뷰: 사용자 for(auto& bullet : bullets_) 순회와 x/y의 velocity×dt 누적 모두 정확. 에이전트가 완료된 TODO/불필요한 (void) 줄만 제거. MSVC Debug 빌드 성공, clangd BulletSystem.cpp 0 errors. 사용자가 다음 진행을 요청했으나 실행 결과의 구체적 보고는 없음.
- 현재 사용자가 이해해야 하는 핵심: size/capacity 구분, vector 재할당 시 원소 참조/포인터 무효화, 중간 삭제 시 뒤 원소 이동과 인덱스 변화. 이동 순회 중 Spawn/RemoveOutside를 수행하지 않는 이유. erase_if는 조건이 true인 원소 삭제, 조건 lambda는 화면 크기를 값으로 캡처하고 탄환을 const 참조로 확인.
- 문답 통과: (1) 재할당으로 기존 참조/포인터가 유효하지 않게 된다는 핵심을 사용자 설명. 포인터 값이 임의로 변하는 것이 아니라 기존 주소가 더 이상 유효한 원소 주소가 아니라는 점 보완. (2) 삭제 시 인덱스를 증가시키지 않아야 한다는 해결책과 alive/dead 상태 분리 제안. 해당 예에서 다음 검사 D / 누락 C로 구체화. 참조 무효화와 순회 누락 이해 확인 통과.
- 현재 과제: 없음. 다음 주제는 Step 8A Sprite 렌더링 구조. 아직 코드를 준비하지 않음.
- 완료 조건: Z키 연속 발사 시 탄환 위로 이동, 화면 완전히 벗어나면 삭제, 놓은 뒤 남은 탄환도 이동, 이동하면서 생성한 탄환은 생성 후 Player와 독립된 위치로 이동. 빌드/정적 분석 통과 및 원소 참조를 사용해야 하는 이유 이해.
- 현재 구현 상태: 탄환 생성/이동/화면 밖 삭제/출력 연결 완료. 제목 Shots는 누적 생성 수, Bullets는 현재 저장 개수. 제목은 약 1초마다 갱신.
- 다음 학습: 사용자 요청으로 그래픽스 기반을 우선. Step 8A 게임 객체/Sprite 표시 데이터/GPU 리소스 책임 분리 → 8B 텍스처 교체/SRV 선택 → 8C Shader UV/Tint/Alpha → 8D 좌표 변환/렌더 상태 실습. 이후 Object Pool. 거대한 범용 렌더러/추상 클래스 계층을 선행하지 않고 현 프로젝트에서 필요한 경계부터 다룬다.
- 사용자 우선순위: Graphics/DX/Win32 기반이 약하므로 텍스처·셰이더·렌더링 객체 구조 실습을 원함. 오브젝트 풀을 당장 우선할 필요가 없다는 문제 제기. 최적화는 측정과 설계 선택을 함께 설명.
- 마지막 검증: 주변 코드 clangd 4파일 각각 0 errors 이력. 최신 사용자 이동 코드와 주석 정리 후 MSVC Debug 빌드 성공, clangd BulletSystem.cpp 0 errors. 출력/탄환 이동 실행 확인은 구체적 보고가 없어 미확인.
- 실행: `.\build.ps1`.
- 사용자 선호: 한 주제 설명/코드 준비/과제/검증을 한 번에 제공하고 불필요한 중간 확인을 줄임. 과제는 사전 설명한 개념/API만으로 풀 수 있게 구성.

## Step 7 인수인계 이력 — 2026-10-01

아래는 Step 7 당시 상태이며, 현재 과제는 위의 Step 8 인수인계를 따른다.

- 현재 Step: **7 — Player**, 연속 발사 간격 과제 진행 중. Step 0~6 학습/구현 완료. 이동 구현은 통과.
- Step 6 이해 확인: GPU로 재복사하지 않으면 처음 전달한 중심을 사용한다고 사용자 답변. 통과. 사용자가 다음 단계로 진행을 요청함. Step 6 픽셀 출력/resize의 구체적인 실행 보고는 없으며 에이전트 시각 검증도 수행하지 않음.
- 완료된 주변 코드: Player 클래스/CMake 등록, Application의 방향키 입력→방향 정규화→fixed Update→화면 경계 제한, Player 위치/크기를 Graphics에 전달.
- 이동 리뷰: 사용자가 불필요한 정규화를 제거하고 방향 × 속도 × dt 누적을 올바르게 구현. CPU→GPU 데이터 분리 이해 확인 통과. 이동 실행/경계의 구체적 결과와 이동 dt 문답은 미보고, 사용자가 다음 학습을 요청해 진행.
- 현재 사용자가 이해해야 하는 핵심: 쿨다운은 다음 발사까지 남은 시간(s), 매 fixed Update의 dt로 시간 경과를 반영. 이동/발사 같은 지속 동작은 눌린 상태(IsDown)를 사용하며 Pressed만 사용하면 연속 입력을 표현하지 못함.
- 현재 코딩 과제: `src/Player.cpp`의 `Player::UpdateShooting(bool shootHeld, double fixedDeltaSeconds)` 구현. CPU 상태 `shotCooldownSeconds_`(초기 0), 상수 `kShotIntervalSeconds`(0.15초)를 사용해 이번 Update 발사 여부를 bool로 반환.
- 발사 정책: 최초 누름은 준비 상태이므로 즉시 가능, 쿨다운 동안 불가, 키를 놓아도 대기 시간 경과, 미입력 동안 발사 불가, 키 재입력으로 대기 시간을 초기화하지 않음. 쿨다운은 음수 누적하지 않고 0에 멈춤. Update당 최대 1회.
- 발사 주변 코드: Application이 Z키 IsDown을 전달하고 true일 때 shotRequestCount_ 증가. 창 제목 FPS/누적 Shots를 약 1초마다 표시. 이번 단원에서는 탄환 생성/표시 대신 발사 요청 카운터로 확인.
- 발사 1차 리뷰: 사용자가 dt 차감, 입력/쿨다운 조건, 발사 시 간격 설정, bool 반환을 올바르게 구현. 키를 놓는 동안에도 시간이 흐르고 재입력으로 양수 쿨다운을 우회하지 않음. 다만 미입력 동안 shotCooldownSeconds_가 계속 음수로 누적되어 조건 3 미충족. 발사 시 간격을 다시 대입하므로 현재 코드가 미입력 시간만큼 몰아서 발사하는 문제는 없음.
- 현재 수정 과제: 남은 대기 시간을 최소 0으로 유지. 정답 코드는 제공하지 않고 사용자가 수정하도록 남김. 완료 보고만 있으며 실행 결과의 구체적 확인은 아직 미보고.
- 발사 2차 리뷰: 사용자가 `shotCooldownSeconds_ = std::max(fixedDeltaSeconds, 0.0);`를 추가했으나 비교 대상 오류. 매 Update에서 양수 dt를 쿨다운에 대입하므로 `<= 0` 조건이 항상 거짓이 되어 발사가 차단됨. std::max는 두 값 중 큰 값을 반환한다는 설명과, 보정 대상은 dt가 아닌 차감 후 남은 시간이라는 힌트를 제공. 수정 확인 전 Step 8로 이동하지 않음.
- Player 초기 중심 (320,240), 전체 크기 48×48, 속도 240px/s. 픽셀 좌표 y는 아래로 증가.
- 완료 조건: Z키 유지 시 약 0.15초 간격으로 Shots 증가, 놓으면 증가 정지, 빠른 재입력으로 쿨다운 우회 불가, 이동과 동시 입력 가능. 빌드/정적 분석 통과, 쿨다운을 초 단위로 관리하는 이유 설명. 시간 비교는 fixed Update 단위로 수행되므로 1틱 수준 차이를 허용.
- 과제 미구현 상태: Player는 이동하지만 Shots가 0에 머무름.
- 다음 학습: 연속 발사 구현/이해 확인과 Step 7 실행 확인 후 Step 8 Bullet System. 아직 이동하지 않음.
- 마지막 검증: CMake Debug 설정/MSVC Debug 빌드 성공. clangd Player.cpp/Application.cpp 각각 0 errors. 이동 실행 검증은 사용자 과제 구현 후 진행.
- 발사 사용자 코드 검증: MSVC Debug 빌드 성공, clangd Player.cpp 0 errors. 쿨다운 음수 제한 수정 후 리뷰 필요.
- 실행: `.\build.ps1`.
- 과제 운영: 설명한 개념/API만으로 해결할 수 있도록 범위를 맞춤. 주변 코드는 에이전트가 완성하고 핵심 함수만 사용자에게 남김. 한 주제 설명/과제/검증을 한 번에 제공.

### Step 7 이동 — 1차 코드 리뷰

- 사용자가 방향 × 속도 × dt를 x_/y_에 누적 구현. 이동 계산의 핵심은 맞음.
- 추가한 `(x + y) >= 2` 정규화 판정은 대각선 일반 판정으로는 부정확함. 다만 현재 Application이 정규화한 방향을 전달하므로 해당 조건은 성립하지 않고 l은 항상 1. 현재 연결에서 이동 계산을 망가뜨리지는 않음.
- 정규화는 Application 책임이므로 Player의 root2/l/나눗셈은 불필요. 사용자에게 제거하도록 리뷰하며 직접 수정하지 않음.
- MSVC Debug 빌드 성공, clangd Player.cpp 0 errors. 실행 이동/경계 확인과 dt 관련 문답은 아직 미확인.
- 현재 과제: 불필요한 정규화 제거, 실행 확인, 업데이트 빈도와 이동 속도 문답. Step 7 이동 진행 중.

## Step 6 인수인계 이력 — 2026-10-01

아래는 Step 6 당시 상태이며, 현재 과제는 위의 Step 7 인수인계를 따른다.

- 현재 Step: **6 — 2D Sprite Renderer**, 진행 중. Step 0~5 완료.
- 완료 구현/과제: SpriteVertex(위치+UV), Quad 4정점, Index Buffer, Input Layout/TEXCOORD, VS의 UV 전달, 2×2 RGBA8 Texture/SRV/Point-Clamp Sampler, PS 샘플링, SRV의 t0 연결, 일반 Alpha Blend 생성/적용.
- 코드 확인: `BindSpriteTexture()`의 PSSetShaderResources와 `BindAlphaBlendState()`의 OMSetBlendState 구현 확인.
- 사용자 실행 확인: Alpha Blend 결과 확인 완료라고 보고. 해당 과제 통과.
- 학습 확인: Resource와 View 분리 이유/참조 관계, UV 보간, Semantic 이름+인덱스, stride/Index Format, SysMemPitch, Alpha 혼합 식, 초기화 시 Create와 Draw 전 Set 차이, 일반 혼합과 덧셈 혼합 차이.
- 현재 학습: Sprite 중심 위치·전체 크기(픽셀), 로컬 좌표에서 픽셀 좌표와 NDC로 변환, CPU 데이터 → Map/Unmap → VS b0 흐름. 설명을 제공하고 사용자 이해/구현 확인 대기.
- 준비 완료: 32바이트 SpriteConstants, Constant Buffer 생성/갱신/VS 바인딩, resize 화면 크기 반영, HLSL 좌표 변환. 기존 Quad의 위쪽 양수 y를 픽셀 좌표의 아래쪽 양수 y로 변환.
- 현재 코딩 과제: `Graphics::SetSpriteTransform(float centerX, float centerY, float width, float height)`에서 CPU 데이터 `spriteConstants_.center`와 `.size` 설정. screenSize/padding, GPU 전송은 에이전트 코드가 담당. 입력은 유효한 위치와 양수 크기로 가정.
- 테스트 호출: Application 생성자에서 중심 (320,240), 전체 크기 (160,120). 과제 미구현 시 size가 0이라 Sprite가 보이지 않는 것이 정상.
- 완료 조건: 좌상 (240,180) / 우하 (400,300)에 해당하는 Sprite 출력, resize 후 픽셀 위치·크기 유지(창 밖 부분은 잘림), 빌드/정적 분석 통과. 중심과 좌상 위치의 차이, CPU 데이터만 바꿔서는 GPU에 반영되지 않는 이유를 자기 말로 설명.
- 다음 Step: Step 6 남은 구현과 이해 확인 후 Step 7 Player(이동/발사/경계). 아직 이동하지 않음.
- 마지막 검증: 2026-10-01 현재 Alpha Blend 사용자 코드 포함 MSVC Debug 빌드 성공. clangd Graphics.cpp/Application.cpp 각각 0 errors. 변경 HLSL의 VSMain/PSMain을 fxc로 각각 컴파일 성공. 픽셀 좌표 출력/resize 실행 확인은 사용자 과제 구현 후 진행.
- 실행 명령: `.\build.ps1` (설정 → Debug 빌드 → 게임 실행).
- 사용자 선호: 한 주제의 설명/주변 코드 준비/과제/검증 방법을 한 번에 제공. 짧은 확인 질문으로 반복 중단하지 않음. 설명은 내부 동작과 이유까지 제공하되 같은 설명은 반복하지 않음. 과제 정답은 먼저 제공하지 않고 사용자 코드는 직접 파일에서 확인.
- 사용자 과제 범위: 설명한 개념과 API만으로 전부 풀 수 있게 구성. 미설명 API/개념을 검색해야 해결할 수 있는 과제는 내지 않음. 코딩/문답 난이도는 학습 내용에 맞게 조절.

### Sprite 위치·크기 과제 리뷰 — 2026-10-01

- 사용자가 SetSpriteTransform에서 center[0/1], size[0/1]에 인자를 올바르게 저장. 코딩 구현 통과.
- 최신 사용자 코드 MSVC Debug 빌드 성공, clangd Graphics.cpp 0 errors.
- 사용자가 완료를 보고했으나 출력 위치·크기/resize 확인 결과는 구체적으로 보고하지 않음. 실행 검증은 미확인.
- 문답 답변: CPU→GPU 복사가 필요한 이유를 NDC 변환이라고 답함. 메모리/리소스 분리와 좌표 변환을 구분하는 보완 설명 제공. 이해 확인 대기, 다음 Step으로 이동하지 않음.
- 기존 방식: 정점의 -0.5~+0.5를 w=1의 clip position으로 직접 출력. NDC 전체 폭 2 중 폭 1이므로 화면 중앙에서 viewport 너비/높이의 절반을 차지. 별도 픽셀 크기 기본값은 없었음.
- 현재 방식: 정점을 로컬 좌표로 사용하고 중심/크기로 픽셀 위치를 계산한 뒤 NDC로 변환. CPU spriteConstants_와 GPU spriteConstantBuffer_는 별개이며 복사 후 VS가 읽음.
- 현재 이해 확인 과제: GPU 전송을 생략하고 CPU center만 변경했을 때 다음 Draw에서 어떤 위치 데이터를 사용하는지와 이유 설명.

## 현재까지 완료된 구현

- CMake 기반 프로젝트 생성
- Visual Studio 2022 / MSVC x64 빌드 구성
- C++23 기본 설정, C++20 전환 옵션 제공
- Win32 Window Class 등록
- Win32 Window 생성
- Direct3D 11 Device 생성
- Direct3D 11 DeviceContext 생성
- DXGI SwapChain 생성
- BackBuffer에서 RenderTargetView 생성
- Window resize 시 SwapChain buffer 재생성
- 기본 화면 Clear + Present 렌더 루프
- D3D11 Debug Layer 사용 시도 및 미설치 fallback
- `ComPtr` 기반 COM 수명 관리
- Antigravity/clangd용 `.clangd` 설정 추가
- `UNICODE`, `_UNICODE`, `WIN32_LEAN_AND_MEAN`, `NOMINMAX`를 CMake/clangd에 일치시킴

## 검증된 환경

- Visual Studio 2022 Professional
- MSVC toolset 14.41.34120 / compiler 19.41.34120
- Windows SDK 10.0.19041
- CMake 3.30.0-rc4
- clang/clangd 18.1.8
- GCC 없음
- Ninja 없음

## 마지막 검증 결과

- `cmake --preset msvc-debug` 성공
- `cmake --build --preset build-debug --parallel` 성공
- 실행 파일 생성 확인: `out/build/msvc-debug/Debug/DanmakuShooter.exe`
- `clangd --check=src/main.cpp --enable-config` → `0 errors`

## 현재 사용자가 학습한/설명 받은 개념

- `wWinMain`과 GUI 프로그램 진입점
- `WNDCLASSEXW`, `RegisterClassExW`, `CreateWindowExW`
- `HWND`와 Win32 Handle
- `WindowProc`, `WM_SIZE`, `WM_DESTROY`, `WM_QUIT`
- `PeekMessageW`를 게임 루프에 사용하는 이유
- `HRESULT`, `FAILED`, `ThrowIfFailed`
- `ID3D11Device`와 `ID3D11DeviceContext`의 역할 차이
- `IDXGISwapChain`의 역할
- BackBuffer와 `ID3D11Texture2D`
- Resource와 View의 분리
- `ID3D11RenderTargetView`
- `OMSetRenderTargets`
- `ClearRenderTargetView`
- `Present`
- Feature Level과 DirectX API 버전의 차이
- COM reference counting과 `ComPtr`/RAII
- `ResizeBuffers` 전 RTV 참조 해제가 필요한 이유
- `GetMessage`와 `PeekMessage` 차이

## 현재 과제

**Step 0 과제 1 — DX11 핵심 객체와 한 프레임 흐름 설명** 완료.

사용자는 다음을 자기 말로 설명한다.

1. `ID3D11Device`의 역할
2. `ID3D11DeviceContext`의 역할
3. `IDXGISwapChain`의 역할
4. `ID3D11RenderTargetView`의 역할
5. `Render()` 한 프레임에서 `OMSetRenderTargets` → `ClearRenderTargetView` → `Present`가 어떤 순서와 의미로 동작하는지
6. 보너스: `ID3D11Texture2D`와 `ID3D11RenderTargetView`가 왜 별도 객체인지

### 사용자 1차 답변 상태

- `ID3D11Device`: GPU 리소스 관리라고 답함 → 방향은 맞지만 "리소스 생성"으로 더 정확히 설명 필요
- `ID3D11DeviceContext`: GPU 명령 처리라고 답함 → 핵심 맞음
- `IDXGISwapChain`: 프론트/백 버퍼 상태 관리라고 답함 → 대략 맞지만 현대 DXGI 관점에서는 presentation buffer 관리와 `Present` 역할로 정교화 필요
- `ID3D11RenderTargetView`: 보여지는 부분 관리라고 답함 → 보완 필요. GPU Resource를 Output Merger의 렌더링 출력 대상으로 해석하는 View
- `OMSetRenderTargets`: Pixel Shader 결과를 어디에 쓸지 지정한다고 답함 → 핵심 맞음
- `ClearRenderTargetView`: BackBuffer를 지정 색으로 지운다고 답함 → 맞음
- `Present` 설명 및 Texture/RTV 차이는 아직 답변 필요

### 사용자 2차 답변 상태

- `Present`를 `ClearBackBuffer()`에 넣지 않는 이유: 함수 책임과 맞지 않고 실제 화면 제시는 별도 단계라고 설명 → 통과
- `ID3D11Texture2D` vs `ID3D11RenderTargetView`: "텍스처와 화면에 보여질 뷰라 서로 다르다"고 설명 → 부분 통과
  - 보완 필요: Texture는 실제 GPU Resource, RTV는 그 Resource를 Output Merger의 렌더링 출력 대상으로 해석/참조하는 View
- Step 0 과제 1의 코딩 제출은 아직 필요

### 코딩 과제 1 결과

- 사용자가 `ClearBackBuffer(const std::array<float, 4>& color)` 구현
- `OMSetRenderTargets`와 `ClearRenderTargetView`를 함수 내부로 분리
- `Present`는 `Render()`에 유지해 책임 분리
- `std::array::data()`로 `ClearRenderTargetView`에 색상 포인터 전달
- MSVC Debug 빌드 성공
- clangd 검사 0 errors
- 결과: **통과**

### 다음 과제

**Step 0 과제 2 — Win32 메시지 루프와 WindowProc 이해** 진행 중.

개념 과제:

1. `PeekMessageW`를 사용하는 이유와 `GetMessageW`와의 차이
2. `TranslateMessage`의 역할
3. `DispatchMessageW`와 `WindowProc`의 관계
4. `WM_DESTROY`와 `WM_QUIT`의 차이
5. `PostQuitMessage(0)`이 실제로 무엇을 하는지
6. 메시지가 없을 때 `Render()`를 호출하는 현재 게임 루프 구조의 의미

코딩 과제:

- `ProcessMessages()` 함수를 직접 작성한다.
- 메시지가 있으면 `TranslateMessage` / `DispatchMessageW`까지 처리한다.
- `WM_QUIT`을 받았는지 호출자가 알 수 있도록 `bool`을 반환한다.
- `Render()`는 `ProcessMessages()` 안에 넣지 않는다.
- 메인 루프는 메시지 처리와 렌더링 책임이 눈에 보이게 분리되도록 수정한다.

### 사용자 1차 답변/코드 리뷰

- `GetMessageW` vs `PeekMessageW`: blocking / non-blocking 차이 설명 → 통과
- `TranslateMessage`: 사전 설명이 없었으므로 추가 설명 필요
- `DispatchMessageW` → `WindowProc`: `WNDCLASSEXW::lpfnWndProc = WindowProc` 연결을 언급 → 통과
- `WM_DESTROY` vs `WM_QUIT`: Window 파괴 메시지와 message loop 종료 메시지로 구분 → 통과
- `PostQuitMessage(0)`: 종료 신호를 보낸다고 설명 → 거의 통과. 정확히는 호출 스레드의 message queue에 `WM_QUIT`을 게시하고 0을 `wParam` exit code로 저장
- 메시지와 Render 관계: 메시지가 있어도 Render가 필요할 수 있다는 핵심 의문 제기 → 좋은 포인트. queue를 먼저 drain한 뒤 매 outer-loop마다 Render하는 구조로 해결 가능

코드 리뷰:

- 사용자가 `ProcessMessages()`로 메시지 처리를 분리했고 메인 루프에서 `Render()`와 책임을 분리함
- MSVC Debug 빌드 성공
- clangd 0 errors
- 논리 버그 존재: `const bool rt = message.message != WM_QUIT;`가 `PeekMessageW()` 전에 한 번만 평가되어, 이후 `WM_QUIT`을 읽어도 `rt`가 갱신되지 않음
- 따라서 Step 0 과제 2는 아직 진행 중. 사용자가 `WM_QUIT` 감지 로직을 수정해야 통과

### 사용자 2차 코드 리뷰

- `rt`를 `const`에서 변경 가능한 `bool`로 수정해 `WM_QUIT` 반영 가능하게 개선
- MSVC Debug 빌드 성공
- clangd 0 errors
- 남은 문제: `WM_QUIT` 확인이 `TranslateMessage` / `DispatchMessageW` 뒤에 있어 종료 메시지를 일반 Window 메시지처럼 dispatch한 뒤에야 감지함
- 통과 조건: `PeekMessageW()`로 메시지를 가져온 직후 `WM_QUIT`을 검사하고, 해당 경우 `TranslateMessage`/`DispatchMessageW` 없이 종료 상태를 반환할 것

### 사용자 3차 코드 리뷰

- `PeekMessageW()` 직후 `WM_QUIT` 검사로 수정
- `WM_QUIT`은 `TranslateMessage`/`DispatchMessageW`로 보내지 않고 즉시 종료 상태 반환
- 일반 메시지만 `TranslateMessage` → `DispatchMessageW` 처리
- queue drain 후 계속 실행 상태 반환
- MSVC Debug 빌드 성공
- clangd 0 errors
- 결과: **Step 0 과제 2 통과**

### 다음 과제

**Step 0 과제 3 — Win32 창 크기와 D3D11 BackBuffer resize 이해** 진행 중.

- `AdjustWindowRect`와 Client Area / Window Area 차이
- `WM_SIZE`의 `lParam`에서 width/height 추출
- 최소화 시 resize를 건너뛰는 이유
- `ResizeBuffers` 전에 RTV를 해제하고 OM에서 분리하는 이유
- resize 후 RTV를 다시 생성해야 하는 이유
- 작은 코딩 과제 포함

### 사용자 답변/코드 리뷰

- `AdjustWindowRect`: 원하는 Client Area와 실제 Window 전체 크기의 차이를 설명 → 통과
- `WM_SIZE`의 `lParam`: width/height가 하나의 값에 함께 들어 있음을 설명 → 통과
- 최소화 시 resize 건너뛰기: drawable client size가 0에 가까워지는 상황을 인지 → 통과
- Resize 전 참조 해제 필요성: RTV와 pipeline binding이 BackBuffer를 참조한다는 점 설명 → 통과
- `ReleaseRenderTarget()` 함수 분리 구현
- MSVC Debug 빌드 성공
- clangd 0 errors
- 현재 구현은 `g_renderTargetView.Reset()` 후 `OMSetRenderTargets(0, ...)` 순서. 동작은 가능하지만 의미상/수명 관리상 `OMSetRenderTargets(0, ...)`로 context binding을 먼저 끊은 뒤 `Reset()`으로 소유 참조를 해제하는 순서가 더 명확함

### Step 0 결과

- 과제 1: DX11 핵심 객체와 Render 흐름 → 통과
- 과제 2: Win32 Message Loop → 통과
- 과제 3: Window Resize / BackBuffer → 통과
- **Step 0 완료**

## 다음 단계

**Step 1 — 프로젝트 구조 분리** 진행 중.

목표:

- `main.cpp`에 몰린 Win32/D3D11 전역 상태와 함수 책임을 분리
- `Window`, `Graphics`, `Application` 수준의 책임 경계 설계
- RAII와 객체 lifetime을 코드 구조에 반영
- 개념 과제 1개 + 코딩 과제 1개로 진행

### Step 1 과제 1 — 책임 설계

사용자가 현재 `main.cpp`의 상태와 함수들을 보고 다음 세 클래스에 어떤 책임을 둘지 직접 분류한다.

- `Window`
- `Graphics`
- `Application`

통과 기준:

- `Window`: Win32 창 생성/메시지/크기 변경 이벤트 등 OS Window 책임
- `Graphics`: D3D11 Device/Context/SwapChain/RTV 생성 및 렌더링/resize 책임
- `Application`: Window와 Graphics를 소유하고 메인 루프를 조정하는 상위 오케스트레이션 책임
- 전역 상태를 각 객체의 멤버로 옮기는 이유를 ownership/lifetime 관점에서 설명

### 사용자 1차 답변

- 전역 DirectX 객체를 `Graphics` 멤버로 옮기는 이유를 "렌더링 객체이기 때문"이라고 설명 → 방향은 맞음. ownership/lifetime과 전역 상태 축소 관점 보완 필요
- 생성 순서를 `Window` → `Graphics`로 제안 → 통과. SwapChain 생성에 `HWND`가 필요하므로 자연스러운 순서
- `Graphics`는 `Window` 자체를 소유하지 않고 생성 시 `HWND`만 전달받는 방식을 제안 → 좋은 방향. 완전한 무의존은 아니며 Win32 handle 의존은 남지만 클래스 간 결합도는 낮아짐
- 단순 책임 분류는 학습 효과 대비 반복 작업 비중이 높아 에이전트가 수행하기로 조정

### 과제 운영 방식 조정

- 단순 분류, 반복적 보일러플레이트, 문서 정리처럼 학습 효과가 낮은 작업은 에이전트가 수행
- 사용자는 설계 판단, 핵심 구현, 디버깅처럼 면접/실무 학습 가치가 높은 작업에 집중

### Step 1 책임 분류 — 에이전트 수행

`Window`
- `kWindowClassName`
- `kWindowTitle`
- `kInitialWidth`, `kInitialHeight` (초기 Window/Client 정책, 추후 config 분리 가능)
- `WindowProc()`
- `ProcessMessages()`
- `RegisterClassExW()` 관련 초기화
- `CreateWindowExW()` 관련 생성
- `ShowWindow()`

`Graphics`
- `g_device`
- `g_context`
- `g_swapChain`
- `g_renderTargetView`
- `ThrowIfFailed()` (현재는 Graphics 내부 helper, 추후 공용 오류 처리로 이동 가능)
- `CreateRenderTarget()`
- `ReleaseRenderTarget()`
- `ResizeBackBuffer()`
- `CreateGraphicsDevice()`
- `ClearBackBuffer()`
- `Render()`

`Application`
- `Window`와 `Graphics` 객체 소유
- 생성 순서 및 메인 루프 조정
- Window resize 이벤트를 Graphics resize로 연결하는 상위 흐름

**Step 1 과제 1 완료**

### Step 1 과제 2 — 실제 구조 분리

- 에이전트가 `main.cpp` 단일 구조를 실제 `Window`, `Graphics`, `Application` 클래스로 분리 완료
- 추가된 파일:
  - `src/Window.h`, `src/Window.cpp`
  - `src/Graphics.h`, `src/Graphics.cpp`
  - `src/Application.h`, `src/Application.cpp`
- `main.cpp`는 `Application` 생성과 `Run()` 호출만 담당하도록 축소
- DirectX 전역 객체를 `Graphics` 멤버로 이동
- `Window`는 Win32 생성/메시지/Client size/pending resize만 담당
- `Application`이 `Window`와 `Graphics`를 멤버로 소유하고 메인 루프를 조정
- `Window`는 `Graphics`에 직접 의존하지 않고 resize를 pending event로 기록
- CMake에 새 소스/헤더 반영
- MSVC Debug 전체 빌드 성공
- clangd: `Application.cpp`, `Window.cpp` 0 errors 확인

#### 사용자 코딩 과제 — 이번 Step에서 유일하게 비워 둔 부분

`Application::HandlePendingResize()` 구현.

목표:

- `window_.ConsumePendingResize()`로 resize 이벤트를 가져온다.
- 이벤트가 존재할 때만 `graphics_.Resize(width, height)`로 전달한다.
- `Window`가 `Graphics`를 직접 알지 않고 `Application`이 둘을 연결하는 구조를 유지한다.

현재 프로젝트는 과제 미구현 상태에서도 빌드된다. 단, 과제를 구현하기 전까지 창 크기 변경 시 D3D11 BackBuffer resize가 연결되지 않는다.

통과 기준:

- `HandlePendingResize()` 구현
- resize 이벤트가 없을 때 `Graphics::Resize`를 호출하지 않음
- MSVC Debug 빌드 성공
- clangd 오류 없음
- 왜 `Application`이 이 연결을 담당하는지 설명 가능

### 사용자 1차 구현 리뷰

사용자 구현:

```cpp
void Application::HandlePendingResize() {
  const auto t = window_.ConsumePendingResize();
  graphics_.Resize(t->width, t->height);
}
```

- MSVC Debug 빌드 성공
- clangd `Application.cpp` 0 errors
- 논리 버그 존재: `ConsumePendingResize()`가 `std::nullopt`를 반환할 수 있는데 존재 여부 확인 없이 `operator->`로 역참조함
- 메인 루프에서 매 프레임 호출되므로 resize 이벤트가 없는 대부분의 프레임에서 잘못된 optional 접근이 발생할 수 있음
- 수정 조건: optional에 값이 있을 때만 `Graphics::Resize` 호출
- 사용자 수정 후 `if (t)`로 optional 존재 여부 확인 완료
- MSVC Debug 빌드 성공
- clangd `Application.cpp` 0 errors
- **Step 1 과제 2 통과 / Step 1 완료**

## 현재 단계

**Step 2 — 게임 루프와 시간** 진행 중.

에이전트 구현 완료:

- `GameTimer` 클래스 추가
- Windows `QueryPerformanceFrequency` / `QueryPerformanceCounter` 기반 고해상도 delta time 측정
- `Tick()`에서 초 단위 `double` 반환
- 비정상적으로 긴 프레임은 최대 0.25초로 clamp
- `Application::Run()`에서 매 프레임 delta time 측정
- `Application`에 `kFixedDeltaSeconds = 1.0 / 60.0`과 accumulator 추가
- `Update(double fixedDeltaSeconds)` 진입점 추가
- CMake에 `GameTimer` 소스 반영
- MSVC Debug 전체 빌드 성공
- clangd `Application.cpp`, `GameTimer.cpp` 0 errors

### Step 2 코딩 과제 1 — Fixed Timestep Accumulator

사용자에게 비워 둔 부분:

`Application::RunFixedUpdates(double deltaSeconds)`

구현 조건:

- `deltaSeconds`를 `accumulatorSeconds_`에 누적
- accumulator가 `kFixedDeltaSeconds` 이상인 동안 `Update(kFixedDeltaSeconds)` 반복 호출
- 매 Update 뒤 accumulator에서 `kFixedDeltaSeconds` 차감
- Render 횟수와 Update 횟수를 분리하는 fixed timestep 구조를 이해하고 설명

### 사용자 구현 결과

```cpp
void Application::RunFixedUpdates(double deltaSeconds) {
  accumulatorSeconds_ += deltaSeconds;
  while (accumulatorSeconds_ >= kFixedDeltaSeconds) {
    Update(kFixedDeltaSeconds);
    accumulatorSeconds_ -= kFixedDeltaSeconds;
  }
}
```

- delta time 누적 정상
- fixed timestep 이상 누적된 경우 여러 번 Update 수행 정상
- Update 한 번마다 fixed timestep 차감 정상
- MSVC Debug 빌드 성공
- clangd `Application.cpp` 0 errors
- **Step 2 코딩 과제 1 통과**

### Step 2 다음 학습 포인트

- Variable timestep과 Fixed timestep의 차이
- 한 프레임이 길어졌을 때 여러 번 Update하는 이유
- delta time을 0.25초로 clamp한 이유와 spiral of death
- Render 주기와 simulation Update 주기를 분리하는 이유
- 이후 FPS 측정/표시를 추가한 뒤 Step 2 완료 예정

### Step 2 코딩 과제 2 — FPS 측정/표시

에이전트 구현 완료:

- `Window::SetTitle(std::wstring_view)` 추가
- 메인 루프에서 `Application::UpdateFps(deltaSeconds)` 호출 연결

사용자에게 비워 둔 부분:

```cpp
void Application::UpdateFps(double deltaSeconds) {
  // TODO: FPS를 측정해서 창 제목에 표시하세요.
  (void)deltaSeconds;
}
```

과제 요구사항:

- 실제 FPS를 계산할 것
- 매 프레임마다 창 제목을 갱신하지 말고 적절한 주기로 표시할 것
- 필요한 상태는 사용자가 직접 설계해 `Application`에 추가할 것
- `Window::SetTitle()`을 사용해 결과를 표시할 것

검증 상태:

- 코드 변경 적용 완료
- clangd `Window.cpp` 0 errors
- MSVC Debug 전체 빌드 성공

### 사용자 1차 구현 리뷰

사용자 구현:

```cpp
void Application::UpdateFps(double deltaSeconds) {
  window_.SetTitle(std::format(L"{}", 1 / deltaSeconds));
}
```

- MSVC Debug 빌드 성공
- clangd `Application.cpp` 0 errors
- 순간 FPS(`1 / deltaSeconds`) 계산 자체는 가능
- 그러나 매 프레임 `SetTitle()`을 호출하고 있어 과제 요구사항인 "적절한 주기로 표시"를 만족하지 못함
- `Application`에 FPS 측정을 위한 상태값도 아직 추가되지 않음
- 표시값이 프레임마다 크게 흔들릴 수 있어 실용적인 FPS 표시로는 부적절
- **Step 2 코딩 과제 2는 아직 진행 중**

### 사용자 2차 구현 리뷰

사용자 구현:

```cpp
void Application::UpdateFps(double deltaSeconds) {
  static int fps = 0;
  static double adt = 0;
  fps++;
  adt += deltaSeconds;
  if (adt >= 1.f) {
    window_.SetTitle(std::format(L"{}", fps));
    fps = 0;
    adt = 0;
  }
}
```

- MSVC Debug 빌드 성공
- clangd `Application.cpp` 0 errors
- 매 프레임 title 갱신 문제는 해결
- 일정 시간 동안 frame count와 elapsed time을 누적하는 방향은 맞음
- 남은 문제 1: 상태를 함수 `static` 지역 변수로 숨겨 두어 `Application` 객체의 명시적 상태/수명과 분리됨. 과제 요구사항은 필요한 상태를 `Application` 멤버로 설계하는 것
- 남은 문제 2: `adt >= 1.0`일 때 단순히 `fps` 카운트만 표시하면 실제 elapsed가 정확히 1초가 아닐 경우 진짜 FPS(rate)가 아님. 측정 FPS는 frame count와 실제 누적 시간의 비율이어야 함
- 남은 문제 3: 누적 시간을 0으로 리셋하면 1초를 초과한 나머지 시간이 버려져 장기적으로 측정 주기가 조금씩 밀릴 수 있음
- **Step 2 코딩 과제 2는 아직 진행 중**

### 사용자 3차 구현 리뷰

- `fps`를 `Application` 멤버로 이동 → 개선
- `fps / adt`로 실제 경과시간 기준 rate를 계산 → 방향 맞음
- `adt -= 1.0`으로 초과 시간 보존 시도 → 방향 맞음
- 그러나 `adt`는 여전히 함수 `static` 지역 상태라 `Application` 멤버로 완전히 이동되지 않음
- `fps -= fps / adt`는 frame count(int)에서 FPS(double)를 빼는 단위 불일치이며 MSVC C4244 경고 발생
- FPS 표시 이후 다음 측정 구간의 frame count 처리 방식을 다시 설계해야 함
- MSVC Debug 빌드 성공(경고 1개)
- clangd `Application.cpp` 0 errors
- Step 2 코딩 과제 2는 아직 진행 중

### Step 0 코딩 과제 1

현재 `Render()`의 Clear 작업을 별도 함수로 분리한다.

요구사항:

```cpp
void ClearBackBuffer(const std::array<float, 4>& color);
```

- 함수 내부에서 `OMSetRenderTargets`로 현재 RTV를 Output Merger에 연결한다.
- `ClearRenderTargetView`로 전달받은 색을 BackBuffer에 기록한다.
- `Present`는 `ClearBackBuffer` 안에 넣지 않고 `Render()`에 그대로 둔다.
- `Render()`는 색상 준비 → `ClearBackBuffer(...)` → `Present(...)` 순서가 되도록 수정한다.

과제 의도:

- DeviceContext와 RTV의 역할을 직접 코드로 다시 사용
- 렌더링 명령과 화면 표시(`Present`)의 책임 차이 확인

### 과제 1 통과 기준

사용자가 최소한 다음 내용을 설명할 수 있어야 한다.

- Device: GPU 리소스 생성
- DeviceContext: GPU 렌더링 명령/파이프라인 상태 설정
- SwapChain: 화면 표시용 Buffer 관리 및 Present
- RTV: Texture/BackBuffer를 렌더링 출력 대상으로 사용하는 View
- 대략적인 흐름: Render Target 설정 → Clear/Draw → Present

표현이 완벽할 필요는 없고, 개념 관계를 자기 말로 설명할 수 있으면 통과시킨다.

## 다음 예정

Step 0 과제를 여러 개 통과하면 **Step 1 — 프로젝트 구조 분리**로 이동한다.

Step 1에서는 전역 상태가 몰린 현재 `main.cpp`를 `Application`, `Window`, `Graphics` 수준으로 분리하며 책임, RAII, 객체 수명을 학습한다.

## 현재 주요 소스

- `src/main.cpp` — Win32 + DirectX 11 최소 실행 구조
- `CMakeLists.txt` — 빌드/컴파일 정의
- `CMakePresets.json` — MSVC Debug/Release preset
- `.clangd` — Antigravity clangd 분석 설정

## 포트폴리오용으로 이미 확보된 문제 해결 사례

### clangd와 실제 MSVC 빌드 설정 불일치

증상:

- `LoadCursorW(nullptr, IDC_ARROW)`에서 clangd가 `LPSTR` → `LPCWSTR` 변환 오류 표시
- 실제 MSVC 빌드는 정상 성공

원인:

- CMake 타깃에는 `UNICODE/_UNICODE`가 정의되어 있었으나 clangd가 동일한 전처리 정의를 읽지 못함

해결:

- `.clangd`에 CMake와 동일한 Unicode/Win32 전처리 정의와 C++23 설정 추가
- clangd 재검증 결과 0 errors

면접 포인트:

- Win32 ANSI/Wide API 차이
- 전처리 매크로와 IDE 정적 분석 환경
- 실제 빌드 설정과 Language Server 설정 일치의 중요성

## 마지막 업데이트

## Current Progress Update

- Step 2 complete: delta time, fixed timestep, FPS measurement.
- Step 3 complete: input system with Up / Pressed / Held / Released states.
- `Input::GetKeyState(UINT virtualKey)` now uses previous/current key state as a 2-bit lookup index.
- Invalid virtual-key indices safely return `Up`.
- MSVC Debug build succeeded without the previous bitwise-type warning.
- clangd `Input.cpp` completed with 0 errors.
- Step 4 in progress: DirectX 11 graphics pipeline and first triangle rendering.

### Step 4 concept check

- Vertex Buffer stores vertex data used by the pipeline.
- Vertex Shader processes vertices and outputs transformed position/data for later stages.
- Rasterizer turns primitives into covered pixel candidates.
- Pixel Shader computes per-pixel output such as final color.
- RTV is used near the end of the pipeline as the Output Merger render target.
- Concept assignment passed.

### Step 4 scaffolding

- Added `shaders/Triangle.hlsl` with a minimal vertex/pixel shader pair.
- Added immutable triangle vertex buffer creation.
- Added vertex shader, pixel shader, input layout resources.
- Added viewport setup and resize update.
- Added shader copy step and `d3dcompiler` linkage in CMake.
- Render path now clears, binds the triangle pipeline, issues `Draw(3, 0)`, then presents.
- `Graphics::BindTrianglePipeline()` is intentionally left as the user coding task.
- MSVC Debug build succeeded.
- `Graphics.cpp` clangd completed with 0 errors.
- Built shader file was confirmed in the executable output directory.

### Step 4 coding task — first review

- User correctly bound the input layout, vertex buffer, triangle-list topology, vertex shader, and pixel shader.
- MSVC Debug build succeeded.
- clangd `Graphics.cpp` completed with 0 errors.
- Remaining logic issue: vertex-buffer stride was set to `sizeof(vertexBuffer_)`, which is the wrapper/object size rather than the byte size of one vertex element.
- Step 4 coding task remains in progress until stride is corrected and the triangle is verified at runtime.

### Step 4 coding task — final review

- `Vertex` type was moved to shared scope so both resource creation and pipeline binding use the same vertex definition.
- Vertex-buffer stride corrected to `sizeof(Vertex)`.
- Input layout, vertex buffer, triangle-list topology, vertex shader, and pixel shader bindings are all correct.
- MSVC Debug build succeeded without warnings.
- clangd `Graphics.cpp` completed with 0 errors.
- Step 4 pipeline-binding coding task passed.
- Runtime visual confirmation of the triangle remains the final manual check for Step 4 completion.

### Step 4 completion

- User moved on after the triangle pipeline task and runtime stage, so Step 4 is considered complete.

### Step 5 — Shader / Constant Buffer

- Added `TriangleConstants` CPU-side data with a float4 color.
- Added a dynamic DirectX 11 constant buffer with CPU write access.
- Added `cbuffer TriangleConstants : register(b0)` to `Triangle.hlsl`.
- Pixel shader now reads `triangleColor` from the constant buffer.
- Constant buffer is bound to the pixel shader with `PSSetConstantBuffers`.
- `UpdateTriangleConstants()` is intentionally left as the user coding task.
- MSVC Debug build succeeded without warnings.
- clangd `Graphics.cpp` completed with 0 errors.

### Step 5 concept check

- User correctly identified Constant Buffer as storage for data shared across shader work in a draw.
- `register(b0)` / `PSSetConstantBuffers(0, ...)` relationship needed clarification: both refer to constant-buffer slot 0 of the pixel-shader stage.
- CPU-side `triangleConstants_` and GPU-side constant buffer are separate memory/resources; changing the CPU object does not automatically copy bytes into the GPU resource.
- Concept check passed after clarification.
- Next task: implement `UpdateTriangleConstants()` using Map/Unmap.

### Step 5 coding task — first review

- User attempted the Map/Unmap flow.
- MSVC Debug build succeeded and clangd reported 0 errors, but the implementation is not logically valid yet.
- `Map(0, ...)` and `Unmap(0, ...)` pass a null resource pointer instead of `triangleConstantBuffer_`.
- No bytes are copied from CPU-side `triangleConstants_` into the mapped GPU resource.
- `PSSetConstantBuffers` is binding state and does not perform the data copy; it is already handled in `BindTrianglePipeline()`.
- User is explicitly stuck, so more concrete API guidance is appropriate for the next attempt.

### Step 5 completion

- `UpdateTriangleConstants()` now maps `triangleConstantBuffer_` with `D3D11_MAP_WRITE_DISCARD`, copies `triangleConstants_` through `mapped.pData`, and unmaps the buffer.
- MSVC Debug build succeeded.
- clangd `Graphics.cpp` completed with 0 errors.
- Step 5 complete.

### Step 6 — 2D Sprite Renderer

- Step 6 started.
- Learning target: render a textured quad and understand UV coordinates, shader resource views, samplers, and alpha blending.
- DirectX assignments from this point provide the relevant API names and roles, while leaving argument/resource wiring to the user.
- `SpriteVertex` now stores position + UV.
- Quad vertex data uses four shared vertices with UVs covering `(0,0)` to `(1,1)`.
- Added a DX11 immutable index buffer with indices `0,1,2, 0,2,3`.
- Input Layout now accepts `POSITION` and `TEXCOORD`; the vertex shader passes UV through the pipeline.
- Pipeline binds the index buffer with `IASetIndexBuffer`, uses `sizeof(SpriteVertex)` stride, and renders the quad with `DrawIndexed(6, 0, 0)`.
- Removed the accidental Direct3D 12 resource type introduced during the Step 6 explanation; the implementation remains DirectX 11.
- MSVC Debug build succeeded.
- clangd `Graphics.cpp` completed with 0 errors.
- Runtime startup check succeeded after the shader/input-layout changes.
- Next learning point: create a texture resource + `ID3D11ShaderResourceView`, then sample it in the pixel shader.

### Step 6 현재 과제 — 2026-10-01

- 루트에 `build.ps1` 추가: `.\build.ps1` 한 줄로 Debug 설정/병렬 빌드/게임 실행. 실패 시 실행하지 않음. README에도 사용법 기록.
- Resource/View 차이와 RGBA8 행 pitch 이해 확인 완료.
- `CreateTestTexture()`에 2×2 RGBA8 Texture, SRV, Point/Clamp Sampler 생성 구현.
- HLSL은 `t0` Texture와 `s0` Sampler로 UV를 샘플링하도록 변경.
- Sampler 바인딩과 렌더 호출 연결은 에이전트가 완료.
- 현재 핵심: Resource 생성, SRV 생성, 파이프라인 슬롯 바인딩은 별개 작업.
- `Graphics::BindSpriteTexture()`의 SRV/t0 연결 구현 확인 완료.
- 완료 조건: 올바른 SRV/슬롯 연결, 빌드 성공, 실행 시 좌상 빨강/우상 초록/좌하 파랑/우하 흰색 확인.
- 과제 미구현 상태에서는 Texture가 바인딩되지 않아 Quad가 검게 나올 수 있음.
- MSVC Debug 빌드 성공, clangd `Graphics.cpp`: 0 errors. 실행/색상 확인은 과제 구현 후 진행.
- 다음 학습: Alpha Blend. Step 6은 아직 진행 중.

### Step 6 현재 과제 — Alpha Blend

- 일반 Alpha Blend 식과 Create(초기화)/Set(Draw 전) 역할 차이 이해 확인.
- 에이전트가 `CreateAlphaBlendState()` 구현, 생성자에서 한 번 생성하고 멤버 ComPtr로 보관.
- 테스트 Texture: 빨강 A=255, 초록 A=128, 파랑 A=0, 흰색 A=128.
- `Render()`에서 Draw 전에 `BindAlphaBlendState()` 호출 연결.
- 현재 핵심: 생성된 상태를 Draw 전에 연결해야 Output Merger 혼합에 적용됨.
- 사용자 과제: `BindAlphaBlendState()` 구현. `OMSetBlendState`로 일반 Alpha Blend 적용.
- 완료 조건: 올바른 상태 연결, 모든 sample 허용, 빌드 성공, 좌하가 배경색이고 초록/흰색이 반투명인지 실행 확인.
- 마지막 검증: MSVC Debug 빌드 성공, clangd Graphics.cpp 0 errors. 과제 미구현 상태에서 혼합은 아직 비활성.
- 다음: 적용 코드 리뷰와 이해 확인 후 Sprite Renderer의 남은 범위 진행. Step 6은 진행 중.

### Step 3 concept check

- User correctly explained Pressed / Held / Released.
- User correctly identified that current key state alone cannot distinguish a new press from a continuing hold; previous-frame state is required.
- Concept assignment passed.
- Next task: implement `Input::GetKeyState(UINT virtualKey)`.

### Step 3 coding task — first review

- MSVC Debug build succeeded.
- clangd `Input.cpp` completed with 0 errors.
- Current lookup-index expression is logically incorrect.
- `(previous,current) = (false,false)` maps to `Held` instead of `Up`.
- `(true,false)` maps to `Pressed` instead of `Released`.
- `(true,true)` produces index 4 for a 4-element array, causing out-of-bounds access / undefined behavior.
- Invalid `virtualKey` values are also not bounds-checked before indexing the state arrays.
- Step 3 coding task remains in progress.

- 2026-09-22
- 인수인계/진행 상황 추적 문서 구조 구축
