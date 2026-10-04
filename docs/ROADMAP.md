# Learning & Implementation Roadmap

이 프로젝트는 각 Step마다 **설명 → 과제 → 리뷰 → 이해 확인 → 다음 Step** 순서로 진행한다.

| Step | 주제 | 핵심 결과물 | 상태 |
|---:|---|---|---|
| 0 | 현재 코드 완전 이해 | Win32/DX11 최소 구조를 말로 설명 | 완료 |
| 1 | 프로젝트 구조 분리 | Application / Window / Graphics | 완료 |
| 2 | 게임 루프와 시간 | Delta Time / Fixed Timestep / FPS | 완료 |
| 3 | 입력 시스템 | Press / Hold / Release | 완료 |
| 4 | DX11 Graphics Pipeline | 직접 삼각형 렌더링 | 완료 |
| 5 | Shader / Constant Buffer | HLSL + CPU→GPU 데이터 | 완료 |
| 6 | 2D Sprite Renderer | Texture Quad / Alpha Blend | 완료 |
| 7 | Player | 이동 / 발사 / 경계 | 완료 |
| 8 | Bullet System | vector 기반 대량 탄환 | 완료 |
| 8A | Sprite 렌더링 구조 | 게임 객체 / Sprite 표시 데이터 / GPU 리소스 책임 분리 | 완료 |
| 8B | Texture 교체와 선택 | Player/Bullet 텍스처 분리, SRV 선택·바인딩 | 완료 |
| 8C | Sprite Shader 실습 | UV / Tint / Alpha / Constant Buffer 연결 | 완료 |
| 8D | 좌표 변환과 렌더 상태 | 이동·크기·회전, Sampler / Blend 동작 비교 | 완료 |
| 9 | Object Pool | 탄환 풀 + 성능 비교 | 완료 |
| 10 | Enemy / Danmaku Pattern | 원형/부채꼴/회전 탄막 | 완료 |
| 11 | Collision | Hitbox / Graze | 완료 |
| 12 | Collision Optimization | Uniform Grid / Broad Phase | 완료 |
| 13 | Scene / Game State | Title / Game / Result | 완료 |
| 14 | Resource Management | Texture/Shader cache | 완료 |
| 15 | Audio | BGM / SFX | 구현 완료 / BGM 실행 확인 미보고 |
| 16 | UI / Score | HP / Score / Boss UI | 구현/리뷰 완료, 승패 실행 확인 대기 |
| 17 | Game Completion | Stage + Boss + 패턴 완성 | 진행 중 |
| 18 | Profiling / Optimization | FPS/frame time/메모리 비교 | 대기 |
| 19 | Refactoring | 책임/수명/const/구조 정리 | 대기 |
| 20 | Final Technical Verification | 랜덤 기술질문/코딩/디버깅 | 대기 |
| 21 | Portfolio Writing | Notion 포트폴리오 완성 | 대기 |
| 22 | Mock Interview | C++/DX11/게임개발 꼬리질문 | 대기 |

## 단계 운영 규칙

- 2026-10-04 진행 방향 합의: 탄막은 HP/무적, 보스 하나, 클리어/실패/재시작까지 작게 완성하고 핵심 검증/기술 기록 후 별도 DX11 3D 학습으로 이어간다. 포트폴리오 최종 문서/모의면접을 3D 진입의 선행 조건으로 삼지 않는다. 3D 핵심 구현은 사용자에게 맡기고 에이전트는 주변 코드/설명/리뷰를 지원한다.

- 2026-10-04 사용자 요청: 단순 호출/배선/반복 설정은 에이전트가 처리하고 빠르게 진행한다. 과제는 새로운 원리 적용이나 설계/디버깅 판단이 필요한 핵심에만 둔다.

- 2026-10-01 사용자 학습 우선순위 반영: Step 8 이후 바로 Object Pool로 넘어가지 않고 8A~8D에서 Graphics/DirectX 기반을 보강한다. Win32 부족한 부분은 실제 입력·resize 등 연결 지점에서 설명/이해 확인한다. 8A 책임 분리 구현과 이해 확인 완료, 다음은 8B.
- Object Pool은 vector만으로도 구현할 수 있으며 컨테이너 교체 자체가 목표가 아니다. 비활성 슬롯 재사용과 측정 가능한 비용 차이를 다루되 현재 게임에 반드시 필요한 최적화라고 전제하지 않는다.
- Step은 숫자 순서대로 진행한다.
- 사용자 이해 확인 없이 다음 Step으로 넘어가지 않는다.
- 필요하면 한 Step을 `0-1`, `0-2`처럼 여러 과제로 나눈다.
- 성능 관련 기능은 가능하면 최적화 전/후 수치를 모두 남긴다.
- 디버깅 이슈는 포트폴리오 소재가 될 수 있으므로 원인과 해결 방식을 기록한다.
- 최종 포트폴리오에서는 단순 기능 목록보다 설계 이유, 트레이드오프, 문제 해결, 성능 수치를 우선한다.
