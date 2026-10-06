# Antigravity 개발 + Visual Studio 프로파일링

편집과 CMake 빌드는 Antigravity에서 계속한다. Visual Studio는 완성된 EXE의 분석 도구로 사용한다.

## 준비된 파일

- `./build-release.ps1`: Release 빌드와 게임 실행.
- EXE: `out/build/msvc-release/Release/DanmakuShooter.exe`
- PDB: `out/build/msvc-release/Release/DanmakuShooter.pdb`
- Release C++ 최적화 유지. `/Zi`와 `/DEBUG:FULL`로 심벌 생성, `/OPT:REF`·`/OPT:ICF` 유지, 증분 링크 비활성. `_DEBUG`는 켜지지 않는다.
- VS2022 Professional과 Performance Tools 디렉터리는 설치돼 있다. CPU/GPU 수집의 실제 동작은 첫 실행에서 확인한다.

## Visual Studio에서 수집

1. 기존 게임을 종료한다. Visual Studio 2022에서 `디버그 → 성능 프로파일러` 또는 `Alt+F2`를 연다.
2. 대상을 `실행 파일(Executable)`로 선택하고 위 Release EXE를 지정한다. 작업 폴더는 프로젝트 폴더로 지정한다. 프로젝트를 VS로 다시 작성할 필요는 없다.
3. `CPU 사용량(CPU Usage)`을 선택하고 시작한다.
4. 최종보스 강화 패턴에서10~20초 플레이하고 `수집 중지`를 누른다. 예비 조사는 수동 장면으로 가능하며 이후 정밀 비교에는 재현 조건을 고정한다.
5. 해당 구간을 선택하고 Call Tree / Functions에서 `Application::Render`, `GameScene::RenderBullets`, `Graphics::DrawSprite`, `GameScene::UpdateCombat` 경로를 살핀다.

Total CPU는 하위 호출을 포함하고 Self CPU는 함수 자체의 CPU 비용이다. GPU 시간과 동일하지 않다. 최적화로 인라인된 함수는 독립 항목으로 보이지 않을 수 있다. 함수 이름이 주소로만 보이면 같은 빌드의 EXE/PDB인지와 심벌 로딩 상태를 확인한다.

## VS 보고서 실패 시: WPR 수집 + WPA 분석

2026-10-06 사용자 실행에서 VS 보고서 생성이 실패했다. 오류 문구는 보존되지 않아 원인은 확정하지 않는다. 이 컴퓨터의 기본 WPR 10.0.26100과 CPU 프로필은 확인했으며 당시 실행 중인 WPR 기록은 없었다. WPA는 PATH와 Windows Kits 디렉터리에서 찾지 못했다. 실제 수집은 아직 하지 않았다.

WPR은 ETW로 CPU 샘플과 호출 스택을 기록하고 WPA는 결과 ETL을 분석한다. 게임에 계측 코드를 추가할 필요가 없다. 2026-10-06 사용자 요청으로 Microsoft ADK의 **Windows Performance Toolkit**을 설치했다(설치 종료0). WPA 실행 경로는 `C:/Program Files (x86)/Windows Kits/10/Windows Performance Toolkit/wpa.exe`다.

1. `./build-release.ps1`로 게임을 실행하고 최종보스 장면으로 이동한다.
2. 별도의 **관리자 PowerShell**을 프로젝트 폴더에서 연다. `wpr -status`로 다른 기록이 진행 중이지 않은지 확인한다. 기존 기록을 임의로 취소하지 않는다.
3. 다음 명령으로 수집을 시작한다. 실패하면 중지 명령으로 넘어가지 말고 오류를 확인한다.

```powershell
wpr -start CPU -filemode
```

4. 게임으로 돌아가 약20초 플레이한 뒤 PowerShell에서 파일로 저장한다. `out`은 빌드로 생성된 폴더이며 Git에서 제외된다. 기존 파일과 겹치지 않는 이름을 사용한다.

```powershell
wpr -stop ./out/cpu-stage3-01.etl
```

5. WPA에서 ETL을 열고 `CPU Usage (Sampled)`를 추가한다. `DanmakuShooter.exe`와 전투 시간 구간을 선택하고 호출 스택별 비용을 본다. 시스템 전체 기록이므로 게임 프로세스로 필터링한다.
6. `Trace → Configure Symbol Paths`에 같은 빌드의 `out/build/msvc-release/Release` **절대 경로**를 추가한 뒤 `Trace → Load Symbols`를 실행한다. 게임 EXE/PDB는 기록 때의 파일을 보존한다.

먼저 렌더 제출 경로(`Application::Render → GameScene::RenderBullets → Graphics::DrawSprite`)와 업데이트 경로의 비중을 비교한다. CPU 샘플은 GPU 실행 시간이나 각 Draw의 정확한 소요 시간을 뜻하지 않는다. 인라인된 함수는 상위 함수에 합쳐질 수 있다. 수집 중 FPS는 도구 오버헤드가 있으므로 기존 무계측 FPS와 분리해 기록한다.

공식 자료: [WPR 시작·중지 명령](https://devblogs.microsoft.com/performance-diagnostics/wpr-start-and-stop-commands/), [Windows Performance Toolkit](https://learn.microsoft.com/en-us/windows-hardware/test/wpt/), [WPA 심벌 로딩](https://learn.microsoft.com/en-us/windows-hardware/test/wpt/loading-symbols).

## GPU 확인과 보조 도구

GPU Usage는 DX11을 지원하지만 설치 구성 요소와 드라이버의 수집 지원을 확인해야 한다. Direct2D HUD는 이 도구의 지원 범위와 구분한다. 아직 수집을 실행하지 않았으며 GPU 병목은 확정하지 않았다.

RenderDoc는 Draw·버퍼·텍스처·블렌딩과 한 프레임의 출력을 조사할 때 사용한다. 직접 타이머/쿼리는 이후 같은 장면의 반복 계측에 보완적으로 사용한다.

공식 문서: [CPU Usage](https://learn.microsoft.com/en-us/visualstudio/profiling/cpu-usage?view=vs-2022), [GPU Usage](https://learn.microsoft.com/en-us/visualstudio/profiling/gpu-usage?view=vs-2022).
