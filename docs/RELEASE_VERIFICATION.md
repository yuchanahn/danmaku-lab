# Release Verification — 2026-10-05

## 결과

- MSVC x64 Debug 및 Release 빌드/링크 성공.
- Debug 실행 파일 --smoke-test: 종료 코드 0.
- Release 패키지 --smoke-test: 종료 코드 0.
- ZIP 압축 해제본을 프로젝트 밖 TEMP 작업 경로에서 실행: 종료 코드 0.
- assets 폴더를 제외한 별도 복사본 실행: WAV 파일 오류로 종료 코드 1, 검사 모드의 오류 팝업 없음.
- Release 의존성 확인: MSVCP140.dll, VCRUNTIME140.dll, VCRUNTIME140_1.dll 등. 대상 PC의 x64 VC++ v14 Runtime 필요.

배포 파일: `out/packages/DanmakuLab-20261005-143507-351.zip`

## 자동 검사 범위

`src/ApplicationSmokeTest.cpp`는 실제 Application 및 게임 객체를 사용한다.

- 키 Pressed→Held와 입력 초기화 후 Up.
- 체력바 비율을 설정한 뒤 크기 변경 시 비율 유지, 왼쪽 끝 고정, 동일 설정 반복 시 위치 보정 비누적.
- 플레이어 탄환 명중 시 피해1/탄환 소비, 소비한 탄환을 다시 검사해도 추가 피해 없음.
- 적 탄환 명중 시 플레이어 피해1/무적 설정, 무적 중 추가 적 탄환은 소비하되 피해 없음.
- 적 HP0 CLEAR, 새 판 HP/탄환/결과/시간 초기화, 동시 사망 FAILED 우선.
- Title/Playing/Glow/Result 렌더 경로, HLSL 컴파일과 Player PNG 로딩, WAV 및 오디오 Voice 초기화와 BGM 시작/종료.

검사 모드는 숨김 Win32 창으로 실행한다. 기본 게임 실행과 분리되어 테스트 이후 게임 플레이 상태를 남기지 않고 종료한다.

## 수동 확인과 한계

이전 학습에서 사용자가 승패/재시작/일시정지, 체력바, 탄환 색상, 최소 창 크기와 최대화·복원을 확인했다. 포커스 손실 입력 고착은 원인 확인/수정 및 빌드했고, 실제 Alt+Tab 재현 성공은 상세 보고가 없어 별도 자동 검증 결과로 주장하지 않는다.

최소화/포커스 Win32 이벤트 자동 조작, 소리 청취 판정, 화면 픽셀 비교, 다른 PC의 런타임/드라이버/고DPI 환경은 이번 검사에 포함하지 않는다. 이는 상용 QA 인증이 아니라 현재 작은 게임의 구현 및 패키지 확인 기록이다.

## 재현

```powershell
.\package.ps1
```

`package.ps1`은 Release를 빌드하고 새 패키지 폴더에서 자동 검사를 수행한다. 로그와 ZIP은 `out/packages`에 생성하며 이전 패키지는 유지한다. 성공 로그를 출력한 뒤 ZIP 경로를 표시한다.
