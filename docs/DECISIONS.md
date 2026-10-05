# Technical Decision Log

프로젝트에서 이미 결정한 기술 선택을 기록한다. 새 에이전트는 특별한 이유가 없는 한 아래 결정을 임의로 뒤집지 않는다.

## D001 — MSVC를 기본 컴파일러로 사용

- 상태: 확정
- 이유:
  - Windows / Win32 / DirectX 11이 핵심 플랫폼
  - 현재 PC에 Visual Studio 2022 Professional + MSVC 툴셋이 설치되어 있음
  - Windows SDK와 DirectX 11 연동이 가장 자연스러움
  - 디버깅과 Windows 네이티브 개발 경험을 포트폴리오에 연결하기 좋음
- 참고:
  - clang 18.1.8은 설치되어 있으나 현재는 언어 서버/정적 분석 용도로 사용
  - GCC/MinGW는 설치되어 있지 않음

## D002 — CMake 사용

- 상태: 확정
- 이유:
  - IDE 종속 프로젝트 파일 대신 빌드 설정을 코드로 관리
  - Debug/Release 및 C++ 표준 설정을 명시적으로 유지
  - Antigravity에서도 동일한 프로젝트 구조 사용 가능

## D003 — 기본 C++23, C++20 호환 옵션 유지

- 상태: 확정
- 기본값: C++23
- 이유:
  - 최신 C++ 기능을 학습/활용
  - 필요하면 `DANMAKU_CXX_STANDARD=20`으로 전환 가능하게 유지

## D004 — DirectX 11 사용

- 상태: 확정
- 이유:
  - 사용자가 DX11을 학습 대상으로 선택
  - 그래픽 파이프라인, 리소스/View, Shader, DeviceContext 등 기본 개념 학습에 적합
  - 포트폴리오에서 로우레벨 그래픽 API 이해를 보여주기 적절함

## D005 — Win32 Unicode API 사용

- 상태: 확정
- 규칙:
  - `UNICODE`, `_UNICODE` 정의
  - `CreateWindowExW`, `DefWindowProcW`, `LoadCursorW` 등 `W` API 사용
- 이유:
  - 현대 Windows 애플리케이션에서 UTF-16 기반 Wide API 사용을 명확하게 유지

## D006 — COM 객체는 ComPtr로 관리

- 상태: 확정
- 이유:
  - 직접 `Release()` 호출 누락 방지
  - RAII 기반 lifetime 관리
  - DirectX 코드에서 ownership을 명확하게 유지

## D007 — 학습 중심 개발 방식

- 상태: 확정
- 규칙:
  - 에이전트가 전체 정답 코드를 먼저 제공하지 않음
  - 핵심 개념은 사용자가 직접 과제로 구현/설명
  - 각 단계가 끝날 때 면접 질문으로 연결
  - 프로젝트 완성 후 종합 면접과 최종 포트폴리오 작성 진행

## D008 — 학습 과제 분담 원칙

- 상태: 확정
- 규칙:
  - 각 Step에서 과제로 지정된 핵심 부분을 제외한 나머지 코드는 에이전트가 먼저 완성
  - 단순 분류, 반복적 파일 생성, 보일러플레이트, 문서 정리, 기계적 리팩터링은 에이전트가 수행
  - 사용자는 명확하게 비워 둔 소수의 핵심 구현/설계/디버깅 과제에 집중
  - 목적은 과제량 자체가 아니라 핵심 코드를 설명하고 다시 구현할 수 있는 수준의 이해 확보

## D009 — 과제 힌트 최소화

- 상태: 확정
- 규칙:
  - 과제용 TODO/주석에는 정답 알고리즘이나 구현 순서를 미리 적지 않음
  - "무엇을 구현할지"만 명확히 제시
  - 사용자가 직접 문서/코드/개념을 찾아 해결하도록 유도
  - 사용자가 막힌 경우에만 힌트를 단계적으로 제공

## D010 — DirectX 과제는 API 시작점을 제공

- 상태: 확정
- 규칙:
  - DirectX 과제에서는 처음 접하는 핵심 API 이름과 각 API의 역할을 먼저 알려줌
  - 과제에 필요한 개념과 API 사용법은 먼저 설명하고, 그 범위 안에서 실제 구현을 사용자에게 남김
  - 미설명 API/개념을 스스로 찾아야만 풀 수 있는 과제는 내지 않음(2026-10-01 사용자 지시)
  - TODO 주석 자체에는 정답 순서나 완성 코드를 적지 않음
- 이유:
  - DirectX는 API 표면적이 넓어 API 이름조차 숨기면 개념 학습보다 검색 시작점 찾기가 과제가 되기 때문

## D011 — Player는 픽셀 단위 게임 상태를 소유

- 상태: 확정 (Step 7)
- Player가 중심 위치/전체 크기/초당 이동 속도를 소유하고 fixed Update에서 위치를 갱신.
- Application이 Input에서 방향을 읽고 정규화해서 Player에 전달. Player는 Win32/DirectX에 의존하지 않음.
- Application이 창 내부 크기를 전달해 Player의 화면 경계를 제한하고, Player 데이터를 Graphics에 전달해 표시.
- 이유: 입력 수집, 게임 상태 변경, GPU 표시의 책임을 나누고 게임 로직을 렌더링 호출 횟수와 분리.

## D012 — Bullet System의 기본 구현과 여러 Sprite 출력

- 상태: 확정 (Step 8 기본 구현)
- BulletSystem이 std::vector<Bullet>로 위치/속도/크기를 값으로 소유. Application이 발사 요청을 생성으로 연결하고 게임 상태를 Graphics에 전달.
- Spawn은 push_back, 화면 밖 삭제는 erase_if로 구현. 이동 순회 중에는 생성/삭제하지 않아 컨테이너 변경과 이동을 분리.
- Graphics는 BeginFrame(Clear/공통 상태), DrawSprite(각 Sprite 상수 전송/Draw), EndFrame(Present)으로 분리.
- 이유: 기존 Quad/Texture를 공유하면서 플레이어와 탄환을 같은 프레임에 출력. 프레임당 Clear/Present는 한 번만 수행.
- 현재는 Sprite당 Map/Draw를 수행하는 이해하기 쉬운 기준 구현. Object Pool 및 렌더링 최적화는 후속 단계에서 필요와 측정에 따라 진행.

## D013 — Object Pool 전에 그래픽스 기반 심화

- 상태: 확정 (2026-10-01 사용자 학습 우선순위)
- Step 8 뒤에 8A Sprite 렌더링 구조, 8B Texture 선택/교체, 8C Shader 실습, 8D 좌표 변환/렌더 상태 실습을 추가.
- 이유: 사용자가 Graphics/DirectX/Win32 기반을 현재 약점으로 지정. 현재 게임의 최적화보다 렌더링 내부와 구조 이해를 우선.
- 구조는 게임 상태/표시 데이터/GPU 리소스의 책임을 구분하는 수준부터 시작. 가상 함수/상속이나 범용 엔진 구조를 먼저 요구하지 않음.
- Object Pool은 이후 기존 vector 구현과 비용을 비교하며 진행. vector가 풀의 기반이 될 수도 있고 현재 규모에서 풀이 반드시 유리하다고 전제하지 않음.

## D014 — Sprite 표시 데이터를 통한 렌더링 경계

- 상태: 확정 (Step 8A)
- SpriteDrawData는 GPU/Win32에 의존하지 않는 CPU 데이터. 현재는 중심 위치와 전체 크기만 포함.
- Application이 게임 상태에서 표시 데이터를 만들고 Graphics::DrawSprite(const SpriteDrawData&)로 전달. Graphics는 게임 객체 타입이나 이동/발사 규칙에 의존하지 않음.
- Graphics는 표시 데이터를 셰이더용 SpriteConstants로 옮기며 화면 크기/패딩 등 GPU 전송 세부 사항을 내부에서 처리.
- DrawSprite는 즉시 상수를 전송하고 Draw를 호출하며 인자 참조를 저장하지 않음. 지연 렌더 큐는 도입하지 않음.
- 이유: 같은 Quad/Texture를 공유하면서 여러 게임 객체를 같은 렌더 경로로 표시. 후속 Texture/Tint/UV 선택을 추가할 경계를 마련.

## D015 — 표시 데이터의 Texture ID와 GPU 리소스 분리

- 상태: 확정 (Step 8B)
- SpriteDrawData는 SpriteTextureId로 이미지를 지정하고, Graphics가 해당 Texture/SRV를 소유.
- 현재 ID는 Checker/Yellow라는 시각 리소스 이름이며 Player/Bullet 타입과 분리. Application이 각 객체에 사용할 이미지를 선택.
- GPU 바인딩은 매 Sprite Draw 전에 PS t0로 수행. Texture ID와 shader register 번호는 서로 다른 개념.
- 이유: 게임 측에서 ComPtr/ID3D11ShaderResourceView를 소유하거나 알 필요 없이 표시할 이미지를 선택. 리소스 캐시/범용 핸들은 후속 범위.

## D016 — Shader 슬롯 명명과 Sprite별 Tint 전송

- 상태: 확정 (Step 8C)
- Shader register와 C++ binding 슬롯을 인터페이스 약속으로 취급. C++에 역할별 명명 상수를 두고 HLSL 선언과 일치시킴. 사용자 config에서 독립적으로 바꾸는 설정으로 취급하지 않음.
- SpriteDrawData.tint는 곱셈 RGBA이며 기본 (1,1,1,1). 매 Sprite Draw마다 PS b1 Constant Buffer에 전송.
- 기존 Triangle 색상 전송 버퍼를 SpriteTintConstants로 정리해 실제 Sprite PS 입력으로 사용.
- HLSL 전용 수정도 실행 폴더로 반영되도록 CopyShaders 빌드 타겟을 사용. 셰이더는 기존대로 게임 초기화 시 D3DCompileFromFile로 컴파일.

## D017 — 실제 경과 시간과 게임 시간 분리

- 상태: 확정 (2026-10-02 Step 8C 시간 실습)
- GameTimer는 비clamp 실제 dt/누적값을 제공. 게임의 긴 프레임 보호(0.25초 clamp)는 Application에서만 적용.
- 게임 시간은 수행한 fixed step의 합, 실제 시간은 pause와 독립적으로 증가. P pause에서는 fixed Update만 멈추고 메시지/입력/렌더링은 유지. pause 전환 때 accumulator를 비워 중지 시간 catch-up 방지.
- FrameConstants는 PS b2로 프레임당 한 번 전송. Sprite별 Game/Real 선택은 표시 데이터로 PS b1에 전달. Renderer가 pause 정책을 결정하지 않음.
- 시간 효과는 현재 RGB 밝기 맥동으로 한정하고 alpha/게임 상태에는 영향을 주지 않음. 후속 UV 실습과 Bloom은 별도 범위.

## D018 — 샘플링 UV와 로컬 UV 구분

- 상태: 확정 (2026-10-02 Step 8C UV 실습)
- SpriteDrawData.uvRect는 startU/startV/endU/endV이며 기본은 전체0~1 범위. 종료값이 시작값보다 작으면 반전할 수 있도록 허용.
- PS b1로 UV 범위를 전달하고 텍스처 Sample 직전에만 변환. Vertex Buffer의 원본0~1 UV는 변경하지 않음.
- 이유: 영역 선택/반전을 리소스 재생성 없이 Sprite마다 지정하고, 후속 원형/빛무리 계산은 Texture Atlas 영역과 독립된 로컬 UV에서 수행.

## D019 — 로컬 UV로 Sprite 알파 형태 생성

- 상태: 확정 (2026-10-02 Step 8C 원형 실습)
- SpriteShape를 표시 데이터로 Graphics에 전달해 공통 PS에서 Rectangle/SoftCircle 선택.
- shape 마스크는 원본 로컬UV에서 계산하고 기존 Alpha에 곱함. 텍스처UV 변환/tint/pulse와 분리.
- 테스트 탄환은20×20 Quad로 생성해 UV 원이 화면에서도 원이 되도록 함. 비정사각형에서는 같은 마스크가 타원으로 출력될 수 있다는 점을 설명.
- 이유: Vertex Buffer/Texture를 교체하지 않고 PS/Alpha Blend로 형태를 표현하는 학습. 충돌 형태 변경이나 Bloom 구현은 이번 범위에 포함하지 않음.

## D020 — Sprite별 Blend Mode 선택

- 상태: 확정 (2026-10-02 Step 8C Glow/Blend 실습)
- SpriteDrawData가 Alpha/Additive Blend Mode를 표시 데이터로 전달하고 Graphics가 각 Draw 직전에 대응하는 Blend State를 바인딩.
- Alpha Blend RGB는 `src.rgb * src.a + dst.rgb * (1 - src.a)`, Additive RGB는 `src.rgb * src.a + dst.rgb`로 설정.
- Shader는 빛 세기를 Alpha에 담고, 최종 framebuffer 합성 방식은 Output Merger의 Blend State가 결정.
- 이유: 같은 Shader 출력도 Blend State에 따라 화면 결과가 달라지는 것을 비교하고, 게임 객체 타입과 DX11 Blend State 선택을 분리.

## D021 — Sprite 회전은 표시 데이터로 전달하고 VS에서 적용

- 상태: 확정 (2026-10-02 Step 8D)
- SpriteDrawData에 `rotationRadians`를 두고 기본값은 0. Graphics가 기존 VS b0 SpriteConstants에 회전각을 함께 전달.
- 회전은 Sprite 중심을 더하기 전의 로컬 좌표에 적용한다. 이후 Sprite 크기를 반영하고 중심 위치를 더한 뒤 기존 픽셀→NDC 변환을 수행.
- 이유: 게임 객체 타입과 무관하게 같은 Quad를 위치·크기·회전만 바꿔 재사용하고, 변환 순서가 최종 화면에 미치는 영향을 직접 학습하기 위함.

## D022 — Sprite별 Sampler Filter 선택

- 상태: 확정 (2026-10-02 Step 8D)
- `SpriteSamplerMode`은 Point/Linear 표시 선택만 보관하고, 실제 `ID3D11SamplerState`는 Graphics가 소유한다.
- Point와 Linear는 AddressMode를 모두 Clamp로 고정해 이번 실습에서는 Filter 차이만 비교한다.
- HLSL의 `SamplerState spriteSampler : register(s0)`는 그대로 두고, Draw 직전에 PS s0에 선택한 Sampler State를 바인딩한다.
- 이유: Texture/SRV와 Sampler가 서로 다른 리소스이며, 같은 Texture도 Sampler State에 따라 읽는 결과가 달라진다는 점을 분리해 학습하기 위함.

## D023 — Sampler Filter와 AddressMode를 독립 표시 상태로 관리

- 상태: 확정 (2026-10-02 Step 8D)
- `SpriteSamplerMode`은 Point/Linear Filter를, `SpriteAddressMode`은 Clamp/Wrap을 각각 표현한다.
- Graphics는 Point/Linear × Clamp/Wrap 조합의 Sampler State 네 개를 소유하고 Draw마다 두 선택값에 맞는 상태를 PS s0에 바인딩한다.
- 이유: Filter는 texel 사이 값을 어떻게 얻는지, AddressMode는 UV가 0~1 밖일 때 어떤 좌표를 읽는지 정하는 서로 독립된 Sampler 설정이기 때문.

## D024 — Bullet Object Pool은 고정 슬롯 + active 재사용으로 시작

- 상태: 확정 (2026-10-02 Step 9)
- 기존 `std::vector<Bullet>` 컨테이너는 유지하되 시작 시 256개 슬롯을 한 번 생성하고 이후 `push_back`/`erase_if`로 크기를 바꾸지 않는다.
- 각 Bullet은 `active` 상태를 가지며, 발사 시 비활성 슬롯을 다시 초기화하고 화면 밖으로 나가면 삭제 대신 비활성화한다.
- Update/Render는 비활성 슬롯을 건너뛴다. UI의 Bullet 수는 vector의 고정 size가 아니라 활성 개수를 표시한다.
- 이유: 컨테이너 종류 변경과 Object Pool 개념을 섞지 않고, 동적 생성/삭제 대신 슬롯 재사용이라는 핵심부터 비교하기 위함. 풀 크기와 성능 효과는 후속 측정에서 검증한다.

## D025 — Debug Overlay는 Direct2D + DirectWrite로 DX11 Back Buffer 위에 출력

- 상태: 확정 (2026-10-02)
- Application이 FPS/게임 상태/렌더 상태/조작키 문자열을 구성하고 Graphics는 문자열만 받아 출력한다.
- Graphics가 Direct2D/DirectWrite 리소스를 소유하고 DX11 Swap Chain Back Buffer에 텍스트와 반투명 배경 패널을 그린다.
- F1으로 Overlay를 표시/숨김한다. 콘솔은 상시 상태 표시 용도로 사용하지 않는다.
- 이유: 게임 실행 중 필요한 상태와 조작키를 화면에서 즉시 확인하면서 게임 로직과 텍스트 렌더링 책임을 분리하기 위함.

## D026 — 탄막 패턴은 임의 속도 Bullet Spawn과 패턴 생성 책임을 분리

- 상태: 확정 (2026-10-02 Step 10)
- BulletSystem은 개별 탄환의 위치/속도/수명만 관리하고, 원형·부채꼴·회전 같은 발사 방향 계산은 DanmakuPattern에서 담당한다.
- 기존 Player 발사용 2인자 Spawn은 유지하고 내부에서 기본 상향 속도를 사용하는 4인자 Spawn으로 위임한다.
- 이유: Object Pool 관리와 탄막 수학을 분리하여 같은 Pool을 여러 발사 패턴에서 재사용하기 위함.

## D027 — Uniform Grid 셀은 Bullet 복사본 대신 Object Pool 인덱스를 저장

- 상태: 확정 (2026-10-03 Step 12)
- 첫 구현은 `std::vector<std::vector<std::size_t>>`로 각 셀에 Bullet Pool 인덱스를 저장한다.
- Bullet Pool은 생성 시 256개 고정 슬롯을 만들고 이후 vector 크기를 바꾸지 않으므로, 슬롯 인덱스는 탄환 활성 수명과 무관하게 안정적으로 사용할 수 있다.
- Grid에는 위치/속도 등 Bullet 전체 상태를 복사하지 않고 원본 Pool을 가리키는 인덱스만 보관한다. 셀 하나에는 여러 탄환이 들어갈 수 있으므로 셀별로 인덱스 목록을 가진다.
- 이유: 공간 분할은 충돌 후보를 분류하는 보조 구조이며 Bullet 상태의 소유권은 계속 BulletSystem에 두기 위함. 포인터 안정성 문제를 피하고 현재 Object Pool 구조와 직접 연결하기 쉽다.

## D028 — 현재 충돌 최적화는 Discrete Uniform Grid로 제한하고 범용 CCD는 제외

- 상태: 확정 (2026-10-03 Step 12)
- 현재 게임은 60Hz fixed update와 낮은 탄환 이동량을 전제로 현재 위치 기반 원 충돌 판정을 유지한다.
- Uniform Grid는 현재 위치의 공간 후보를 줄이는 Broad Phase로만 사용하며, 이전 위치부터 현재 위치까지의 swept path는 저장하지 않는다.
- 고속 projectile에서 tunneling이 문제가 되면 특정 타입에 swept circle/segment test 또는 substep을 추가할 수 있으나, 범용 CCD/TOI 시스템은 현재 포트폴리오 범위에서 구현하지 않는다.
- 이유: 현 게임 요구에서 정확도 이득이 작고 구현 복잡도가 큰 범용 CCD보다 DX11/게임 루프/데이터 구조/측정 가능한 Broad Phase 최적화 학습을 우선한다.

## D029 — Player 충돌 Broad Phase는 현재 셀 기준 3x3 이웃을 조회

- 상태: 확정 (2026-10-03 Step 12)
- Enemy Bullet은 현재 중심 위치 기준으로 하나의 64px Grid 셀에 등록한다.
- Player 충돌 후보는 Player 중심 셀과 주변 8개 셀에서 조회한다.
- 현재 가장 넓은 판정은 Player Graze 반경 24px + Enemy Bullet 반경 8px = 중심거리 32px이며 셀 크기 64px보다 작다. 따라서 실제 판정 가능성이 있는 두 중심이 두 셀 이상 떨어질 수 없다.
- 셀 크기나 Hit/Graze 반경을 바꿔 최대 상호작용 거리가 셀 크기보다 커지면 조회 반경도 함께 늘려야 한다.

## D030 — Game State는 단일 enum 값과 중앙 전환 함수로 관리

- 상태: 확정 (2026-10-03 Step 13)
- 현재 화면/게임 흐름은 `GameState { Title, Playing, Result }` 중 정확히 하나의 값으로 표현한다.
- 상태 변경은 직접 대입을 여러 위치에 흩뿌리지 않고 `ChangeGameState`를 통해 요청한다.
- 첫 전환 그래프는 `Title -> Playing -> Result -> Title`로 제한한다.
- 이유: 복수 bool에서 가능한 모순 상태를 제거하고, 허용 전환과 향후 상태 진입/이탈 시 초기화 작업을 한 위치에서 추적하기 위함.

## D031 — 상태 전환 검증과 상태 진입 초기화를 분리

- 상태: 확정 (2026-10-03 Step 13)
- `ChangeGameState`는 현재 상태와 요청 상태를 보고 전환 허용 여부를 판단하고 상태 값을 변경한다.
- 성공한 전환의 초기화/부수 작업은 `EnterGameState`에서 상태별로 수행한다.
- 새 Playing 시작 시 Player와 Bullet Pool은 객체를 재생성하지 않고 `Reset`으로 기존 객체/슬롯을 재사용한다.
- 이유: 같은 Playing 진입이 여러 입력 경로에서 발생해도 새 게임 초기화 규칙을 한 곳에서 보장하고, 전환 그래프 판단과 상태별 초기화 책임을 섞지 않기 위함.

## D032 — TextureCache가 Sprite SRV 수명을 소유하고 Graphics는 조회 결과를 빌려 바인딩

- 상태: 확정 (2026-10-03 Step 14)
- `TextureCache`가 `ComPtr<ID3D11ShaderResourceView>` 배열을 소유하고 Texture 생성과 ID 기반 조회를 담당한다.
- `Graphics`는 `SpriteTextureId`를 캐시에 전달해 raw `ID3D11ShaderResourceView*`를 잠시 빌린 뒤 `PSSetShaderResources`에 바인딩한다.
- Bullet 같은 gameplay 객체는 GPU Resource/COM pointer를 소유하지 않는다. 현재 `SpriteDrawData`는 Application이 렌더 직전에 구성하는 표시 데이터로 유지한다.
- 이유: GPU Resource의 생성·수명·조회 책임과 Draw/바인딩 책임을 분리하고, 여러 Sprite가 같은 Texture를 별도 생성 없이 공유할 수 있게 하기 위함.

## D033 — 실제 이미지 파일은 WIC로 디코딩하고 TextureCache에서 Lazy 생성

- 상태: 확정 (2026-10-03 Step 14)
- PNG/JPG 같은 파일 포맷은 직접 파싱하지 않고 Windows Imaging Component로 RGBA8 픽셀 데이터까지 디코딩한다.
- 디코딩된 픽셀은 DX11 `ID3D11Texture2D`와 `ID3D11ShaderResourceView`로 변환하여 TextureCache가 소유한다.
- 파일 기반 Texture는 첫 요청 시 생성하여 캐시에 저장하고 이후 요청은 같은 SRV를 재사용한다. 필요하면 Stage 진입 전에 필요한 ID를 미리 요청해 preload할 수 있다.
- 이유: 이미지 포맷 구현과 GPU Resource 관리를 분리하고, 사용하지 않는 Texture의 초기 생성 비용을 피하면서 실제 게임에서는 preload로 첫 사용 hitch를 제어할 수 있게 하기 위함.

## D034 — ShaderCache가 Sprite Shader와 VS bytecode를 소유하고 Graphics는 Pipeline 상태를 조립

- 상태: 확정 (2026-10-03 Step 14)
- `ShaderCache`가 Sprite HLSL 컴파일과 `ID3D11VertexShader` / `ID3D11PixelShader` 생성·수명을 담당한다.
- InputLayout 생성에는 Vertex Shader의 compiled signature가 필요하므로 Sprite VS bytecode도 ShaderCache가 소유하고 Graphics가 생성 시 빌려 쓴다.
- Vertex/Index Buffer와 InputLayout은 현재 Sprite geometry / vertex format / pipeline 조립 책임에 속한다고 보고 Graphics에 유지한다.
- Graphics는 렌더 시 ShaderCache에서 raw Shader pointer를 빌려 VS/PS stage에 바인딩한다.
- 이유: Shader resource 생성·수명을 Draw orchestration에서 분리하되, Vertex format과 geometry까지 ShaderCache에 섞지 않아 책임 경계를 유지하기 위함.

## D035 — SFX PCM은 공유하고 SourceVoice는 고정 Pool로 재생 인스턴스를 분리

- 상태: 확정 (2026-10-03 Step 15)
- Shot SFX의 PCM/WAVEFORMAT 데이터는 `AudioSystem`이 한 번 로드해 공유한다.
- 동시에 겹칠 수 있는 재생 상태는 여러 `IXAudio2SourceVoice`가 담당하며, 첫 구현은 Shot용 8개 고정 Pool을 사용한다.
- SourceVoice는 생성 후 시작 상태로 유지하고 `BuffersQueued == 0`인 voice를 idle slot으로 재사용한다.
- Pool이 모두 사용 중이면 첫 버전에서는 추가 동적 생성 없이 해당 SFX 요청을 건너뛴다.
- 이유: 오디오 원본 데이터와 재생 인스턴스의 책임/수명을 분리하고, 짧은 반복 SFX마다 SourceVoice 생성/파괴를 반복하지 않기 위함.

## D036 — BGM은 전용 SourceVoice 하나를 재사용하고 GameState 진입에서 제어

- 상태: 확정 (2026-10-04 Step 15)
- 현재 BGM은 동시에 여러 곡을 겹쳐 재생하지 않으므로 전용 `IXAudio2SourceVoice` 하나를 사용한다.
- BGM PCM은 `AudioSystem`이 한 번 로드해 수명을 소유하고, 재생 Buffer는 `XAUDIO2_LOOP_INFINITE`로 반복한다.
- Playing 진입에서 BGM을 시작하고 Result 진입에서 중지하는 정책으로 GameState와 오디오 상태를 연결한다.
- `StopBgm()`은 Stop 후 SourceVoice queue를 비워 다음 Playing 진입에서 곡이 처음부터 시작되게 한다.
- 이유: SFX의 동시 재생 인스턴스 Pool과 장기 단일 BGM 재생의 요구가 다르므로 구조를 분리하고, 상태 전환 지점에서 오디오 정책을 명확히 관리하기 위함.

## D037 — 점수는 게임 이벤트에서 변경하고 HUD는 현재 값을 표시

- 상태: 확정 (2026-10-04 Step 16)
- Application이 score와 grazeCount를 소유하며 새 Playing에서 초기화한다. 탄환당 첫 Graze에서 100점을 가산한다.
- HUD 문자열과 상태별 안내는 Application에서 구성하고 Graphics는 받은 문자열/영역만 렌더한다. F1 디버그 표시와 HUD 표시를 분리한다.
- 이유: 렌더 빈도가 점수에 영향을 주지 않고 UI 표시와 게임 규칙의 책임을 분리하기 위함.

## D038 — Player가 HP와 피격 무적 시간을 관리

- 상태: 확정 (2026-10-04 Step 16)
- Player 기본 HP 3, 피해 1, 피격 후 무적 1초로 시작한다. 무적 시간은 fixed game update에서 감소해 pause 중 유지된다.
- 충돌 검사부는 TryTakeDamage를 요청하고 HP 0에서 Result로 전환한다. Player는 피해 허용 판단과 HP/무적 상태 변경을 담당한다.
- Player 몸에 닿은 적탄은 무적 여부와 무관하게 소비하며 해당 접촉은 Graze로 처리하지 않는다.

## D039 — src 소스 파일을 CMake에서 자동 수집

- 상태: 확정 (2026-10-04 사용자 요청)
- `file(GLOB DANMAKU_SOURCES CONFIGURE_DEPENDS ...)`로 `src` 바로 아래 `.cpp`, `.h`, `.hpp`를 수집하여 실행 파일 대상에 전달한다. 하위 폴더는 자동 수집하지 않는다.
- `CONFIGURE_DEPENDS`로 빌드 시 파일 목록 변경을 확인하고 필요한 경우 CMake를 다시 구성한다. 헤더도 IDE의 대상 파일 목록에 포함한다.
- 이유: 학습 중 클래스 파일을 추가할 때마다 소스 목록을 직접 수정하는 반복 작업과 등록 누락을 줄이기 위함.

## D040 — 전투 결과와 화면 상태를 분리하고 동시 사망은 실패 우선

- 상태: 확정 (2026-10-05 Step 16-7 과제 정책)
- GameState는 Title/Playing/Result 화면·진행 상태, BattleOutcome은 None/Clear/Failed 전투 결과를 나타낸다. 새 Playing에서 결과를 초기화한다.
- 플레이어 및 적 피격 처리가 모두 끝난 뒤 전투 결과를 판정한다. 같은 fixed update에서 양쪽 HP가 0이면 실패를 우선한다.
- 이유: 충돌 함수 호출 순서가 승패를 결정하지 않게 하고, 하나의 Result 화면에서 결과에 맞는 문구를 표시하기 위함.

## D041 — 사용자 요청으로 Present 수직동기화 대기를 해제

- 상태: 확정 (2026-10-05)
- Present의 SyncInterval을 1에서 0으로 변경한다. 게임의 고정 업데이트 간격은 계속 1/60초다.
- 이번 변경은 현재 swapchain에서 Present(0,0)을 사용하는 것으로 한정하며 tearing 지원이나 표시 시스템의 다른 프레임 제한 해제를 보장하지 않는다.

## D042 — 키보드 포커스를 잃으면 입력 상태를 초기화

- 상태: 확정 (2026-10-05 입력 고착 수정)
- WM_KILLFOCUS 수신 시 포커스 손실 알림을 기록하고 기존 미처리 키 이벤트를 비운다. Application이 Input의 현재/이전 키 배열을 모두 초기화한 뒤 이후 이벤트를 반영한다.
- 메시지 처리 직후 최소화 대기 이전에 입력 알림을 소비한다. Window는 Input 객체를 직접 소유하거나 참조하지 않는다.
- 이유: 다른 창에서 키를 놓아 KEYUP이 누락된 경우 이전 Held 상태가 남지 않게 하고, 포커스 경계에서 불필요한 Pressed/Released 이벤트를 만들지 않기 위함.

## D043 — 최소 client 크기 960×540으로 창 축소를 제한

- 상태: 확정 (2026-10-05 게임 마무리)
- WM_GETMINMAXINFO에서 AdjustWindowRect로 계산한 외곽 크기를 ptMinTrackSize에 적용한다. 최대화/확대 및 기존 Resize 처리는 유지한다.
- 이유: 고정 보스 위치와 현재 HUD 배치가 보이는 최소 게임 영역을 보장하기 위함. 현재 범위에서는 가상 해상도 및 레터박스 시스템을 추가하지 않는다.

## D044 — 게임 콘텐츠·아트·이펙트 확장 후 실제 장면을 최적화

- 상태: 확정 (2026-10-05 사용자 명시 요청)
- 정교한 텍스처, 다중 적/보스/패턴, 스테이지, 파티클·탄환 효과, 플레이어/적 디졸브, 타이틀·결과 UI를 먼저 확장한다. 이후 실제 게임의 렌더링 및 성능을 측정·개선한다.
- 기존 단일 보스 Release는 첫 완성본으로 보존한다. 인스턴싱 등 최적화 방법은 사전에 확정하지 않고 실제 장면의 CPU/GPU 병목 확인 후 선택한다.
- 이유: 사용자가 원하는 완성도와 실제 표현 규모를 먼저 만들고, 그 비용을 기준으로 개선하기 위함. 이전 빠른 3D 진입 계획은 확장 진행 후 재검토한다.

## D045 — 확장 게임 아트는 캐릭터 중심 판타지

- 상태: 확정 (2026-10-05 사용자 선택)
- 우주선 기반 SF 대신 캐릭터 중심 판타지로 아트 방향을 정한다. 마법사 플레이어는 첫 시안이며 캐릭터 디자인/적 종류/배경의 세부 설정까지 확정한 것은 아니다.
- 원본 이미지 품질과 게임에서의 작은 표시 크기 가독성을 함께 확인한다. 시안을 실제 게임용 최종 자산으로 바로 간주하지 않는다.

## D046 — 사용자 제작 이미지와 스프라이트 시트 애니메이션

- 상태: 확정 (2026-10-05 사용자 명시)
- 이미지는 사용자가 직접 제작한다. 캐릭터 및 필요한 효과는 동일 크기 셀을 배열한 스프라이트 시트로 준비한다. 에이전트는 필요한 자산 목록, 규격, 적용 코드와 학습을 지원한다.
- 게임 해상도와 캐릭터 표시 크기는 확장에 맞춰 조정할 수 있다. 기존 48×48을 최종 규격으로 강제하지 않는다. 기본 창 client 크기는 이후 D047에서 Full HD로 확정했다.
- docs/ASSET_PLAN.md의 콘텐츠 수/프레임 수/셀 크기는 제작량 계산을 위한 초안으로 확정값이 아니다.

## D047 — 기본 창 client 크기 Full HD

- 상태: 확정 (2026-10-05 사용자 요청)
- 기본 게임 영역(client)을1920×1080으로 변경한다. 기존 AdjustWindowRect로 제목 표시줄/테두리 크기를 더해 일반 창을 생성한다.
- 기존 Resize 및 최소 client 크기960×540 정책을 유지한다. 캐릭터 표시 크기와 배치 조정은 자산 적용 단계에서 다룬다.

## D048 — Full HD 창 안에 720×960 세로 전투 영역

- 상태: 확정 (2026-10-05 사용자 승인)
- 전투는 논리 좌표720×960에서 진행한다. 기본1920×1080 창에서는 전투 영역이(600,60)에 배치되고, 그 밖은 불투명 패널/게임 HUD 영역이다.
- 화면 표시 배율은 min(clientWidth/1920, clientHeight/1080). 전투 Sprite 중심/크기를 Application에서 화면 좌표로 변환하고 가운데 배치한다. 창 크기가 달라져도 이동 속도/충돌 반경/스폰 좌표/Grid는 논리 좌표를 유지한다.
- 플레이어 초기(360,800), 현재 보스(360,140). 탄환은720×960 바깥으로 완전히 나가면 반환하며 Grid는64px 셀의12×15 고정 구성을 사용한다.
- 현재 경계 표현은 전투 렌더 이후 주변4개 불투명 Sprite로 덮어 Glow/탄환이 HUD 영역에 보이지 않도록 한다. 전용 Scissor/Viewport 변경이나 RenderTexture는 이번 연결에 추가하지 않는다.
- D043 당시 가상 해상도를 추가하지 않던 범위는 이번 사용자 요청에 따라 변경됐다. 스테이지 배경은3:4, 타이틀/결과 전체 배경은16:9로 제작한다.

## D049 — 빌드마다 assets 디렉터리 전체 복사

- 상태: 확정 (2026-10-05 이미지 적용 오류 수정)
- CopyAssets에서 파일별 하드코딩 대신 cmake -E copy_directory로 프로젝트 assets를 실행 파일 옆 assets에 복사한다. 새 이미지 추가 시 CMake 목록 누락으로 로딩 실패하는 문제를 해결한다.
- 복사는 매 빌드 수행하며 원본에서 삭제된 파일을 출력 폴더에서 자동 삭제하지 않는다. 최종 배포는 기존 package.ps1의 새 패키지 폴더에서 준비한다.

## D050 — Sprite 애니메이션은 재생 규격 데이터와 시간 기반 UV 계산

- 상태: 확정 (2026-10-05 사용자 명시 구현 요청)
- SpriteAnimationData는 같은 크기 셀의 열/행/시작 프레임/프레임 수/fps/반복 여부만 보관한다. 별도 Animator 클래스나 Update/현재 프레임 상태는 추가하지 않는다.
- DrawGameSprite 오버로드에서 UV를 계산해 기존 SpriteDrawData/렌더 경로를 재사용한다. 시간은 SpriteTimeSource 선택 또는 명시적 경과 시간 인자로 전달한다. 개별 효과는 후자에 생성 이후 경과 시간을 넘길 수 있다.
- 플레이어는6×3 시트의 각 행6프레임을12fps로 반복하고, 방향 전환 시 공통 게임 시간의 위상을 유지한다. 임의 프레임 배치/가변 셀/상태별 전환 블렌딩은 현재 규격에 포함하지 않는다.

## D051 — Texture 캐시 크기는 ID 개수에서 계산

- 상태: 확정 (2026-10-05 Enemy 추가 오류 수정)
- SpriteTextureId 마지막에 Count를 두고 TextureCache 배열 크기를 그 값에서 계산한다. 실제 리소스 ID는0부터 연속으로 배정하며 Count 자체는 그릴 수 있는 텍스처 ID가 아니다.
- 이유: ID 추가 시 별도 배열 크기 리터럴 수정 누락으로 유효 ID가 거부되는 문제를 방지한다.

## D052 — 리소스 제작을 잠시 두고 셰이더 이펙트부터 학습

- 상태: 확정 (2026-10-05 사용자 요청)
- 디졸브 마스크/경계/사망 연결과 탄막 효과를 먼저 한 Step씩 진행한다. 시작 노이즈는 로컬UV의 절차적 계산으로 제공하며 별도 노이즈 이미지 생성은 요구하지 않는다.
- 디졸브 진행도는 Sprite 표시 데이터이며 게임 판정과 분리한다. 첫 실습은 F3 수동 미리보기이고 실제 사망 수명과의 연결은 후속 Step이다. 아트 및 콘텐츠 확장 범위는 취소하지 않는다.

## D053 — 전투 종료 판정 뒤 Ending에서 사망 연출

- 상태: 확정 (2026-10-05 사용자 승인)
- 현재 단일 보스 전투는Playing→Ending→Result로 종료한다. 승패는HP0에서 결정하고0.8초 동안 죽은 캐릭터의 디졸브만 진행한다. 기존 동시 사망 실패 우선 규칙을 유지한다.
- Ending에서는 기존 전투 Update를 호출하지 않아 추가 피해/이동/발사를 방지한다. 게임시간과 시트 포즈는 정지, 별도Ending 경과 시간만fixed update로 증가한다. P pause와 최소화는 연출도 멈춘다.
- BGM은Ending 진입에서 정지한다. F3 미리보기와 실제 사망 진행도를 분리하고 새Playing에서 연출 상태를 초기화한다.
- 현재는 죽음이 전투 종료인 단일 보스의 연결이다. 다중 적에서 일반 적 사망마다 전투를 멈추지 않도록 객체별 사망 연출 수명으로 확장할 예정이다.

## D054 — 전투와 HUD를 Application에서 분리

- 상태: 확정 (2026-10-05 사용자 코드 이동 요청)
- Application은 창·메시지/고정 루프·입력 전달·일시정지·화면 전환·오디오를 담당한다. GameScene은 캐릭터·탄환·충돌 Grid·전투 시간·승패·사망 연출을 소유한다. 발사 여부와 결과를 전달받은 Application이 소리와 화면 전환을 연결한다.
- GameHud는 체력바와 화면 UI를 소유하고 GameScene을 const로 읽는다. GameSpriteRenderer는 Graphics를 빌려 시트 UV/논리 좌표 변환을 적용한다. PlayfieldLayout은 화면 배치 값과 변환만 보관한다. D048의 Application 좌표 변환 책임은 이 도우미로 이동했다.
- 사용자 플레이어8×2 시트는 Idle/Left/Right 상태로 표현하고 렌더 시 행·반전을 선택한다. D050의 초기6×3 규격은 현재 자산으로 대체하며 시간 기반 UV 계산 방식은 유지한다.
- 이유: 다중 적·스테이지·효과 확장에 앞서 변경 책임과 데이터 수명을 명확히 하기 위함. GPU 리소스 소유권이나 기존 게임 동작은 유지하며 범용 Scene 상속/엔진 체계는 도입하지 않는다.

## D055 — 총알 종류와 후속 콘텐츠 범위

- 상태: 총알2종/보스 소환/3스테이지는 사용자 요청으로 확정. 표시 크기와 판정 수치는 조정할 초기값이다.
- 소속은BulletOwner, 형태는BulletType Normal/Thin으로 표현한다. 첫 값은Normal20×20/반경8, Thin28×8/반경3. 타입 규격에서 스폰 크기와 충돌 반경을 얻으며 GPU 리소스는 소유하지 않는다.
- 얇은 탄은긴 타원 표시와 중심 원 판정으로 시작한다. 전체 길이에 판정을 맞추는 캡슐/타원 충돌은 현재 도입하지 않는다. 장식용 후광은 충돌 영역에 포함하지 않는다.
- 진행 방향 각도는속도벡터의atan2(y,x)로 계산한다. 기존 정사각형 중심 렌더에서 가려졌던 비균일 크기와 회전의 순서 문제는 사용자 학습 과제로 둔다.
- 후속은보스의 작은 적 소환,1/2/3스테이지와 매우 어려운 최종 보스 패턴. 난이도 세부 수치와 보스/플레이어 확대값은 아직 확정하거나 적용하지 않았다.

## D056 — 탄환 몸체와 후광의 분리 합성

- 탄환 몸체는Alpha로 경계를 유지하고, 후광은기본Additive/alpha0.25로 약하게 합성한다. 전체후광 후 전체몸체 순서로 후광이 다른탄의몸체를 덮는 문제를 줄인다.
- 흰 중심은BulletBody 셰이더의RGB 혼합으로 처리하여 추가 중심 스프라이트/드로우콜을 만들지 않는다. 몸체와 후광으로 활성탄당2회DrawSprite가 발생하며 이후 실제 확장 장면의 렌더 성능 측정 대상으로 둔다.
- 새 이미지나Bloom/렌더타깃은 추가하지 않는다. B로기존실습모드와비교하며 기본새표현에서7/8은후광합성만변경한다. 충돌과 표시 효과는 별도로 유지한다.

## D057 — 적2종·중간보스·최종보스로 구성한3스테이지

- 상태: 확정(사용자구성).1스테이지는적1/적2시간차등장총약10마리전원처치,2는중간보스,3은적1/적2소환패턴을가진최종보스다. 중간보스처치후최종보스로진행한다.
- 최종보스는매우어려운대량탄막을구현하고해당실제장면으로성능개선스토리를이어간다. 스폰수10은대략값이며종류별배분/간격/난이도수치는후속구현에서조정한다.
- 상세진행조건/책임/구현순서는docs/STAGE_PLAN.md에기록한다. 최종보스사망시잔여소환적처리및스테이지전환연출은아직확정하지않는다.

## D058 — 기본8프레임 자산과 일반 적의 독립 상태

- 사용자 자산은 적1/적2/중간보스/최종보스 각 기본 이동8프레임이다. 별도 공격 시트는 필수가 아니다. 열/행은 적용 시 자산에 맞추고 현재Enemy.png4×2를 임시 공유한다.
- 일반 적은 Enemy 값 객체의 개별 쿨다운/생존시간/사망시간으로 동작한다. 우선 기존 보스와 별도vector에 적2종을 연결하고, StageDirector 단계에서 보스와 진행 연동을 통합한다.
- 일반 적 사망은 Alive→Dying→Removed,0.8초 동안 판정/발사/이동 중단. 시트 위상은 생존시간에 고정하고 순회가 끝난 뒤 제거한다. 적 포인터를 프레임 사이에 보관하지 않는다.

## D059 — 캐릭터 표시 크기와 일반 적 체력바

- 사용자 요청으로 Player64×64/일반 적48×48/보스128×128을 적용한다. 표시 크기와 판정은 분리하며 기존 판정 반경5/18/24를 유지한다.
- 일반 적 HP바는 GameHud가 재사용하는 HealthBar 한 개로 각 적의 현재 HP비율과 위치를 설정한 뒤 즉시 그린다. 적마다 UI객체나 HP사본을 소유하지 않는다. 살아 있는 적만 그리며 표시폭과 같은 폭/높이5/위쪽 간격8을 사용한다.
- 플레이어 HP바48×6, 보스 상단 HP바240×12. 창 배율은 기존 GameSpriteRenderer의 논리 좌표 변환을 공유한다.

## D060 — 공용 숲·안개2레이어 스크롤

- 상태: 사용자확정(2026-10-06). 전스테이지공용bg0숲/bg1안개를사용하고스테이지별배경은제작하지않는다.
- 현재941×1672원본을폭720에맞춰비율을유지한다. 게임시간기반하향스크롤40/15px/s,안개alpha0.16,레이어별2장연결/fmod반복으로구현한다. 안개는캐릭터·탄환보다먼저그려탄을덮지않는다.
- GPU리소스는TextureCache,표시구성은ScrollingBackground,시간은GameScene이담당한다. 별도배경Update/누적타이머는두지않는다. P/Ending에서정지하고게임재시작시초기화한다.

## D061 — 3스테이지 연결과 최적화 전 대량 탄막 기준

- 2026-10-06 사용자 위임: 에이전트가3스테이지 흐름 전체를 구현하고 최적화부터 사용자 참여로 전환한다. 이번 범위에서는 핵심 코딩 TODO를 남기지 않는다.
- StageDirector는1스테이지 시간표와 단계 진행을 소유한다. GameScene은 실제 적 생성/충돌/정리를 연결하고 Enemy는 개체별 이동·HP·발사·사망 수명을 소유한다. 현재 보스 한 개와 일반 적 vector를 유지하고 상속/ECS는 도입하지 않는다.
- 1스테이지 시간표10마리 소진과 전원 제거가 모두 필요하다. 중간보스 사망은 탄 정리 및0.8초 디졸브 후 다음 단계, 최종보스 사망은 탄 정리 및 소환 적 디졸브 후 Clear다. 동시 플레이어 사망은 Failed 우선. 중간 전환에서 HP·점수·게임시간은 보존하고 회복은 없다.
- 최종보스는 이중 회전 원형 탄막/조준 부채꼴/일반 적 소환/HP절반 강화로 실제 대량 렌더 장면을 제공한다. 일반 적 소환 제한8은 Dying도 포함한다. 세부 수치는 조정 가능한 초기 설정이며 플레이 밸런스 확정값이 아니다.
- 탄환 풀8192, 생성 요청과 부족 누락 카운터로 실제 탄수 보존을 확인한다. 슬롯 탐색/개별 렌더는 기존 방식으로 남겨 측정 전 기준을 유지한다. 용량 확대나 자동 검사 탄수는 성능 개선으로 기록하지 않는다.

## D062 — 종류별 적·보스 시트 매핑

- 2026-10-06 사용자 요청으로 Enemy1.png/Enemy2.png를 일반 적1/2, boss1.png/boss2.png를 중간/최종보스에 연결했다. 모두1024×128의8열×1행 시트,8프레임/12fps다.
- TextureCache에 종류별 ID/파일 경로를 등록하고 기존 지연 로딩을 유지한다. 렌더링은 EnemyKind로 표시 리소스를 선택하며 GPU 객체는 Enemy가 소유하지 않는다.
- 공유 시트용 임시 Tint를 흰색으로 바꿔 원본 색을 유지한다. 개체별 생존시간에 따른 애니메이션 및 사망 시 포즈 고정/디졸브, 기존 표시 크기와 충돌 반경을 유지한다.

## D063 — 캐릭터 표시 크기25% 확대

- 2026-10-06 사용자 크기 확대 요청으로 Player80×80/일반 적60×60/보스160×160을 적용한다. D059의 표시 크기를 대체하며 판정 반경5/18/24는 유지한다.
- 플레이어 HP바60×6, 일반 적 HP바60×5, 보스 상단바240×12다. 표시 크기와 판정을 분리해 아트 가독성을 조정하며 공격 패턴·속도·탄 크기는 유지한다.
