# Danmaku Lab — Win32 & DirectX 11 Shooter

C++23, Win32, DirectX 11로 만드는 Windows용 2D 탄막 슈팅 학습 프로젝트입니다. 게임 루프, 그래픽 파이프라인, 리소스 수명, 충돌 처리와 UI를 직접 구현하며 클라이언트 프로그래밍의 기반을 익히고 있습니다.

일반 적10마리 → 중간보스 → 일반 적을 소환하는 최종보스의3스테이지 전투를 구현했습니다. 공용 숲·안개 배경, 시트 애니메이션과 사망 디졸브를 연결했으며, 새 아트·추가 파티클·UI 디자인은 확장 중입니다. 이후 최종보스의 대량 탄막 장면에서 렌더링 병목을 측정하고 개선합니다.

## 구현 기능

- 60Hz 고정 업데이트, 게임 시간과 실제 시간 분리, 일시정지
- 방향키 이동, 연속 발사, 8192개 고정 탄환 풀과 생성 누락 카운터
- 시간표 기반 일반 적10마리, 중간보스·최종보스, HP에 따른 강화 패턴과 적 소환
- 조준 부채꼴/이중 회전 원형 탄막, 보통탄과 얇은 탄
- 숲·안개2레이어 반복 스크롤,8프레임 캐릭터 시트와 UV 반전
- 원형 Hitbox, Graze 점수, Uniform Grid 기반 적 탄환 충돌 후보 검색
- 플레이어 HP와 피격 무적, 무적 중 알파 깜빡임
- 플레이어/일반 적/보스 체력바, 스테이지·점수 HUD, 디버그 정보 표시
- Title / Playing / Ending / Result 전환, 개별 적 사망 디졸브, 클리어·실패와 새 판 초기화
- HLSL Sprite 렌더링: Tint, Alpha, UV 영역, 회전, 원형 마스크와 Glow 표현
- Point/Linear 필터, Clamp/Wrap 주소 모드, Alpha/Additive 블렌딩 비교
- TextureCache의 WIC 이미지 로딩, ShaderCache의 HLSL 컴파일 및 셰이더 관리
- XAudio2 효과음 SourceVoice 풀과 전용 BGM Voice

Glow는 스프라이트 셰이더와 블렌딩을 활용한 표현이며, 후처리 Bloom은 구현하지 않았습니다.

## 개발 환경

- Windows x64
- 기본 창(client 영역): 1920×1080, 최소960×540, 논리 전투 영역720×960
- Visual Studio 2022의 MSVC C++ 도구 및 Windows SDK
- CMake 3.25 이상
- 기본 언어 표준 C++23 (CMake 옵션으로 C++20 선택 가능)
- DirectX 11, Direct2D, DirectWrite, WIC, XAudio2

IDE와 무관하게 터미널에서 빌드할 수 있습니다. 사용 중인 편집기는 Antigravity입니다.

## 빌드 및 실행

저장소 루트에서 실행합니다.

```powershell
cmake --preset msvc-debug
cmake --build --preset build-debug --parallel
.\out\build\msvc-debug\Debug\DanmakuShooter.exe
```

설정, Debug 빌드, 실행을 한 번에 진행하려면:

```powershell
.\build.ps1
```

Release 빌드:

```powershell
cmake --preset msvc-release
cmake --build --preset build-release --parallel
```

빌드 시 `shaders`와 `assets`를 실행 파일 옆으로 복사합니다. 실행 파일을 다른 곳으로 옮길 때도 두 폴더를 함께 옮겨야 합니다.

## Release 패키지 만들기

```powershell
.\package.ps1
```

Release 설정/빌드 후 실행 파일, 이미지·오디오, HLSL, 실행 안내를 `out/packages`의 새 폴더에 모읍니다. 프로젝트 폴더 밖을 작업 경로로 사용해 자동 검사를 실행하고, 성공한 경우 ZIP을 생성합니다. 생성물은 Git에 포함하지 않습니다.

배포 대상 PC에는 [Microsoft Visual C++ v14 x64 Runtime](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist)이 필요합니다. 현재 개발 PC에서 검사했으며 다른 PC의 드라이버·런타임 호환성을 모두 검사한 것은 아닙니다.

`--smoke-test`는 숨김 창으로 리소스 로딩과 렌더 경로, 입력 초기화, 체력바 비율, 명중/탄환 소비, 무적, 클리어·실패와 재시작을 확인한 뒤 종료합니다. 성공은 종료 코드 0, 실패는 1이며, 검사 중에는 오류 팝업 대신 표준 오류에 기록합니다.

## 조작

| 키 | 기능 |
|---|---|
| Enter | Title에서 시작 / Result에서 Title로 복귀 |
| 방향키 | 플레이어 이동 |
| Z | 연속 발사 |
| P | 플레이 중 일시정지 / 재개 |
| F1 | 디버그 패널 표시 / 숨김 |
| F2 | 어두운 기본 배경 / 밝은 테스트 격자 전환 |
| F4 | 테스트 치트: 현재 살아 있는 모든 적·보스에게50 데미지(한 번 누를 때1회) |

그래픽 실습용 키도 유지하고 있습니다.

| 키 | 기능 |
|---|---|
| 1 / 2 / 3 | 플레이어 UV 전체 / 부분 / 좌우 반전 |
| 4 / 5 / 6 | 탄환 사각형 / 부드러운 원 / Glow |
| 7 / 8 | 탄환 Alpha / Additive 블렌딩 |
| 9 / 0 | 플레이어 회전 증가 / 회전 초기화 |
| B | 새 탄환 표현 / 기존 셰이더 실습 모드 전환 |
| N / L | 플레이어 Point / Linear 필터 |
| C / W | 플레이어 Clamp / Wrap 및 UV 반복 범위 |

## 주요 구조

- `Application`: 창·게임 루프, 입력 전달, 화면 전환과 오디오 연결
- `GameScene`: 전투 데이터, 이동·발사·충돌·승패·사망 연출과 전투 렌더 구성
- `StageDirector`: 1스테이지 스폰 시간표와3스테이지 진행 상태
- `ScrollingBackground`: 게임시간 기반 숲·안개 반복 스크롤 표시
- `GameHud`: 체력바, 점수·디버그 정보, 타이틀·결과 UI
- `GameSpriteRenderer`, `PlayfieldLayout`: 시트 UV 선택과 논리 전투 좌표의 화면 변환
- `Player`, `Enemy`, `BulletSystem`: 게임 데이터와 동작
- `Collision`, `UniformGrid`: 충돌 판정과 후보 검색
- `Graphics`: DX11 렌더링 및 Direct2D/DirectWrite 텍스트 출력
- `SpriteDrawData`, `HealthBar`: 표시 데이터와 체력바 구성
- `TextureCache`, `ShaderCache`: GPU 리소스 생성과 수명 관리
- `AudioSystem`: WAV 로딩과 XAudio2 재생
- `shaders/Sprite.hlsl`: 스프라이트 셰이더
- `assets`: 테스트 이미지와 오디오

## 현재 범위와 후속 작업

- 1스테이지는0~14초에 적10마리 등장 후 전원 처치,2는HP120 중간보스,3은HP240 최종보스와 최대8마리 소환 적
- 중간 단계에서 HP·점수·게임시간 보존, 탄환 정리. 최종보스 사망 후 잔여 적 디졸브와 최종 클리어
- 충돌은 각 고정 업데이트의 위치에서 검사하며 CCD는 구현하지 않음
- BGM은 게임 일시정지 중에도 계속 재생되는 정책
- 사망 디졸브와 경계 발광 구현 완료. 적1/2·중간/최종보스의 개별8프레임 시트를 적용했으며 추가 파티클·타이틀/결과 UI는 후속 작업
- 자동 검사에서3스테이지 진행과 최종보스24초 패턴(최대3230발/생성 누락0)을 확인했으며 실제 난이도·가독성은 플레이 검증 필요
- 합성 배치의 충돌 CPU 비교는 측정했으며 렌더링 병목 측정은 확장 게임 구현 이후 진행
- 고DPI/다른 PC 환경의 전체 UI 검증은 미수행

## 학습 기록

- [현재 진행 상황](docs/PROJECT_STATUS.md)
- [학습 및 구현 로드맵](docs/ROADMAP.md)
- [기술 결정과 이유](docs/DECISIONS.md)
- [충돌 비용 비교 및 실행 방법](docs/COLLISION_BENCHMARK.md)
- [Release 패키지 검증 범위](docs/RELEASE_VERIFICATION.md)
- [학습 진행 규칙](AGENTS.md)

이 저장소는 AI의 설명·주변 코드 지원과 직접 구현·리뷰를 병행한 학습 과정을 기록합니다. 구현 완료 여부와 실행 검증 여부는 진행 상황 문서에서 구분합니다.
