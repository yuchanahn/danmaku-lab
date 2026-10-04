# Danmaku Shooter

C++23, Win32, DirectX 11로 만드는 Windows용 2D 탄막 슈팅 학습 프로젝트입니다. 게임 루프, 그래픽 파이프라인, 리소스 수명, 충돌 처리와 UI를 직접 구현하며 클라이언트 프로그래밍의 기반을 익히고 있습니다.

현재 개발 중이며, 플레이어와 보스의 전투 및 클리어/실패 결과 처리가 구현되어 있습니다. 전투 조정, 성능 비교와 코드 정리는 진행 예정입니다.

## 구현 기능

- 60Hz 고정 업데이트, 게임 시간과 실제 시간 분리, 일시정지
- 방향키 이동, 연속 발사, 256개 고정 탄환 풀
- 회전 부채꼴/원형 탄막과 보스 HP에 따른 패턴 선택
- 원형 Hitbox, Graze 점수, Uniform Grid 기반 적 탄환 충돌 후보 검색
- 플레이어 HP와 피격 무적, 무적 중 알파 깜빡임
- 플레이어/보스 체력바, 점수 HUD, 디버그 정보 표시
- Title / Playing / Result 전환, 클리어·실패 구분과 새 판 초기화
- HLSL Sprite 렌더링: Tint, Alpha, UV 영역, 회전, 원형 마스크와 Glow 표현
- Point/Linear 필터, Clamp/Wrap 주소 모드, Alpha/Additive 블렌딩 비교
- TextureCache의 WIC 이미지 로딩, ShaderCache의 HLSL 컴파일 및 셰이더 관리
- XAudio2 효과음 SourceVoice 풀과 전용 BGM Voice

Glow는 스프라이트 셰이더와 블렌딩을 활용한 표현이며, 후처리 Bloom은 구현하지 않았습니다.

## 개발 환경

- Windows x64
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

## 조작

| 키 | 기능 |
|---|---|
| Enter | Title에서 시작 / Result에서 Title로 복귀 |
| 방향키 | 플레이어 이동 |
| Z | 연속 발사 |
| P | 플레이 중 일시정지 / 재개 |
| F1 | 디버그 패널 표시 / 숨김 |

그래픽 실습용 키도 유지하고 있습니다.

| 키 | 기능 |
|---|---|
| 1 / 2 / 3 | 플레이어 UV 전체 / 부분 / 좌우 반전 |
| 4 / 5 / 6 | 탄환 사각형 / 부드러운 원 / Glow |
| 7 / 8 | 탄환 Alpha / Additive 블렌딩 |
| 9 / 0 | 플레이어 회전 증가 / 회전 초기화 |
| N / L | 플레이어 Point / Linear 필터 |
| C / W | 플레이어 Clamp / Wrap 및 UV 반복 범위 |

## 주요 구조

- `Application`: 게임 루프, 입력 연결, 전투 진행과 화면 상태
- `Player`, `Enemy`, `BulletSystem`: 게임 데이터와 동작
- `Collision`, `UniformGrid`: 충돌 판정과 후보 검색
- `Graphics`: DX11 렌더링 및 Direct2D/DirectWrite 텍스트 출력
- `SpriteDrawData`, `HealthBar`: 표시 데이터와 체력바 구성
- `TextureCache`, `ShaderCache`: GPU 리소스 생성과 수명 관리
- `AudioSystem`: WAV 로딩과 XAudio2 재생
- `shaders/Sprite.hlsl`: 스프라이트 셰이더
- `assets`: 테스트 이미지와 오디오

## 현재 제한 및 남은 작업

- 보스 패턴과 난이도 조정, 클리어/실패/재시작의 전체 실행 검증
- 원형 패턴 발사 간격은 현재 1.5초이며 학습 과제 목표는 1.0초
- 플레이어 체력바는 현재 플레이어 아래쪽에 배치되어 있으며 위쪽 배치 보완 예정
- 충돌은 각 고정 업데이트의 위치에서 검사하며 CCD는 구현하지 않음
- BGM은 게임 일시정지 중에도 계속 재생되는 정책
- 성능 비교 수치와 코드 정리는 아직 진행 전

## 학습 기록

- [현재 진행 상황](docs/PROJECT_STATUS.md)
- [학습 및 구현 로드맵](docs/ROADMAP.md)
- [기술 결정과 이유](docs/DECISIONS.md)
- [학습 진행 규칙](AGENTS.md)

이 저장소는 AI의 설명·주변 코드 지원과 직접 구현·리뷰를 병행한 학습 과정을 기록합니다. 구현 완료 여부와 실행 검증 여부는 진행 상황 문서에서 구분합니다.
