# 스테이지 FPS 자동 측정

저장소 루트에서 `./benchmark-stage-fps.ps1` 실행. Release 빌드가 끝난 뒤 자동으로 측정 창을 표시하고 약63초 동안 수집한다. CSV는 `out/benchmarks/stage-fps-<실행 시각>.csv`에 매번 새 파일로 기록한다. 창을 닫거나 최소화하거나 크기를 바꾸면 측정을 취소한다. Present가 DXGI_STATUS_OCCLUDED를 반환해도 취소한다.

## 고정 조건

- Release / client1920×1080 기본값 / Present(0,0) / 강화 탄환 인스턴싱 / Uniform Grid.
- 기본 HUD·체력바·배경·안개 ON, F1 디버그 오버레이 OFF, BGM OFF.
- 플레이어 시작 위치에서 무적 ON, 이동·발사 입력 없음. 충돌 검사와 명중 탄환 제거는 유지한다.
- 각 스테이지를 새로 구성하고60Hz 업데이트로 게임 시간15초까지 준비한다. 최종보스는 HP50% 강화 패턴을 사용한다. 준비 구간은 측정에서 제외한다.
- 실제 업데이트와 렌더를2초간 실행해 리소스를 준비한 뒤5초간 측정한다.1→2→3을3회 반복한다. 단계당 수집 구간은 대략 게임 시간17~22초다.
- DXGI device에서 실제 사용 중인 GPU를 조회해 CSV에 저장한다. GPU 드라이버, 전원/온도와 백그라운드 부하는 고정하지 않았으므로 비교할 때 외부 조건도 관리해야 한다.

## 지표 의미

- FPS = 완료한 게임 루프/Present 호출 수 ÷ 실제 측정 시간. 모니터에 표시된 고유 프레임 수나 GPU 단독 처리 시간은 아니다.
- 시간 측정은 QueryPerformanceCounter/QueryPerformanceFrequency를 사용한다. [Microsoft 시간 측정 문서](https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps)
- mean_frame_ms = 전체 측정 시간 ÷ 프레임 수. 메시지 처리·고정 업데이트·FPS 제목 갱신·HUD·렌더·Present 및 계측 bookkeeping을 포함한다.
- p95/p99_frame_ms는 프레임 시작부터 Present 종료까지의 시간 분포다. 종료 후 통계 저장 비용은 포함하지 않으므로 mean과 정확히 같은 모집단은 아니다.
- bullets_mean은 프레임별 활성 탄수 평균이며 시간 가중 평균이 아니다. dropped는 해당 스테이지 준비부터 측정 종료까지의 생성 누락 수다.
- 이 결과는 정해진 위치와 패턴의 자동 플레이 구간이며, 전체 스테이지 평균이나 사람의 수동 플레이 FPS를 뜻하지 않는다. 과거 수동 관찰값과 탄수/패턴/시간이 다르므로 곧바로 최적화 배율로 계산하지 않는다.

## 무적 치트

F6으로 ON/OFF. 기본 OFF이며 재시작 시 선택을 유지한다. 무적은 HP 감소만 막고 피격 깜빡임은 추가하지 않는다. 전체 순회/Grid 모두 충돌 처리와 명중 탄환 제거를 그대로 수행한다. HUD에 상태를 표시한다.

## 최적화 전후 비교

[2026-10-07 전후 실제 결과](measurements/STAGE_COMPARISON_2026-10-07.md)

`./benchmark-stage-fps.ps1 -Compare` 또는 Release EXE의 `--benchmark-stage-fps-compare`로 수집한다. 약126초 동안18구간을 기록하며 CSV 이름은 `stage-compare-<실행 시각>.csv`다. 기본 단일 방식 측정은 그대로 사용할 수 있다.

- 매 반복/스테이지마다 개별 Draw와 인스턴스 Draw의 두 구간을 새로 준비한다. 첫·셋째 반복은 개별→인스턴스, 둘째 반복은 인스턴스→개별 순서다.
- 두 모드는 강화 효과ON으로 동일한 후광 padding16/alpha0.25, 몸체 shape와 크기, 색·회전·UV·샘플러·블렌드·그리는 순서를 사용한다. B키의 효과OFF 경로를 최적화 전으로 사용하지 않는다.
- 개별 경로는 과거 탄환별 두 상수 버퍼 갱신 및 DrawIndexed 제출 방식을 복원한다. 후광/몸체 순회도 분리한다. 인스턴스 경로는 활성 탄환 한 번 순회/영구 저장 공간/CPU 회전 기저/두 배열 업로드와 DrawIndexedInstanced를 사용한다.
- 같은 현재 EXE에서 렌더 전략을 선택한다. 과거 커밋 전체의 실행 파일 비교가 아니라, 현재 콘텐츠와 같은 측정 장치에서 최적화 전 렌더 경로를 복원한 대조 실험이다.
- 모든 빌드와 자동 검사를 끝낸 뒤 측정하며 다른 게임 인스턴스를 동시에 돌리지 않는다. 기존 수집 결과를 새 전후 비교의 '후' 수치로 재사용하지 않는다.
- CSV의 mode를 기준으로 각 스테이지의3회 FPS 중앙값을 비교한다. 향상 배율=후FPS/전FPS, 프레임 시간 감소율=1-전FPS/후FPS. 반복 편차와 활성 탄수/생성 누락도 함께 확인한다.

## 최종보스 공간분할 비교

`./benchmark-stage-fps.ps1 -Grid` 또는 Release EXE의 `--benchmark-grid`로 실행한다. `-Compare`와 동시에 사용하지 않는다. 약42초 동안 최종보스 HP50%만 두 방식×3회 수집하고 `grid-compare-<실행 시각>.csv`를 만든다.

렌더링은 양쪽 모두 최적화 전의 개별 Draw로 고정한다. 준비 상태는 같고 warmup부터 전체 순회/Uniform Grid를 선택한다. 순서는 Linear→Grid / Grid→Linear / Linear→Grid다. 일반 실행 기본 Grid·인스턴싱은 유지한다.

추가 열 `collision_ticks`, `build_ms_per_tick`, `query_ms_per_tick`, `collision_ms_per_tick`, `candidates_per_tick`은 수집 중 고정 업데이트의 구축/플레이어 대 적 탄환 조회 비용과 후보 수다. 구축+조회 합계로 판단하고 후보 수만으로 성능 향상을 주장하지 않는다. 이 계측은 Grid 비교에만 켜므로 다른 벤치마크의 추가 열0은 미계측을 뜻한다.

[2026-10-07 결과와 CSV](measurements/GRID_COMPARISON_2026-10-07.md): 조회는 빨라졌지만 구축 포함 충돌 비용은 약5.6% 증가했다. 별도 실행의 렌더링 비교 FPS와 합쳐 연속 개선율로 계산하지 않는다.

## 탄환 풀 탐색 비교

`./benchmark-stage-fps.ps1 -Pool` 또는 Release EXE의 `--benchmark-pool`. 다른 비교 스위치와 함께 사용하지 않는다. 최종보스 HP50%에서 인스턴싱·Grid를 고정하고 전체 순회/빈 인덱스 최소 힙을 각3회 비교한다.

각 반복에서 타이머OFF 두 구간으로 전체 FPS를 수집한 뒤 타이머ON 두 구간으로 생성/반납 시간을 수집한다. 구간마다 같은 상태15초 준비/2초warmup/5초수집, 총12구간·약84초다. 순서는 각 반복마다 반전한다. CSV는 `pool-compare-<실행 시각>.csv`다.

추가 열 `pool_timing`이0이면 개별 시간 미계측,1이면 계측이다. `spawn_calls`, `release_calls`, `inspected_slots`, `spawn_ms`, `release_ms`는 계측 구간 전체 합계다. 마지막 두 값은 1프레임 시간이 아니다. 힙의 inspected_slots0은 활성 플래그 탐색이 없다는 뜻이며 힙 비교 연산까지0이라는 뜻은 아니다.

[2026-10-07 결과와 CSV](measurements/POOL_COMPARISON_2026-10-07.md): 생성+반납 비용 감소, 전체 FPS의 일관된 향상은 확인하지 못했다. 일반 실행 기본은 최소 힙이며 과거 렌더/Grid 측정 CSV는 변경하지 않는다. 이후 재실행에는 새 풀 기본값이 적용되므로 과거 기록과 직접 개선율로 비교하지 않는다.

## 최초 단일 FPS 결과

[2026-10-07 실제 결과](measurements/STAGE_FPS_2026-10-07.md)에9개 측정 행과 요약을 보존했다. 첫 예비 수집은 Debug 빌드와 일부 겹쳤으므로 공식 결과에서 제외한다.
