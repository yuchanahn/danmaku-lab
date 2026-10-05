# Art Direction — Step 17A

- 사용자 확정 방향: 캐릭터 중심 판타지.
- 첫 플레이어 시안: `assets/concepts/player_mage_v1.png`. 기존 `player_test.png`는 유지하고 게임에는 아직 연결하지 않았다.
- 생성 방식: built-in imagegen, transparent_background=true. 원본을 프로젝트에 복사해 보관했다.
- 검토 결과: 모자/망토를 갖춘 마법사 후면 전신 이미지. 원화 장식이 많고 체형이 길며, 게임용 탑다운 시점과 작은 표시 크기에 맞춘 보완이 필요하다. 현재 Player는 48×48로 표시한다. 실제 크기에서의 가독성/투명 경계는 적용 시 추가 확인한다.
- 현재 과제: 캐릭터 분위기에서 유지할 점과 바꿀 점을 정한다. 합의 후 비율/실루엣을 보완하고 적용한다.
- 표시 크기와 충돌 반경을 분리한다. 이미지의 투명 여백도 Quad 크기에 포함된다. 작은 게임 화면에서 읽히는 실루엣과 일관된 시점이 중요하다.

## 첫 시안 생성 프롬프트

```text
Use case: stylized-concept
Asset type: single playable character sprite for a 2D fantasy bullet-hell game, prototype art direction preview.
Primary request: a polished original fantasy mage character game sprite, full body, stylized hand-painted anime game illustration with readable bold shapes. Floating forward toward the top of the screen, viewed from above and slightly behind, so hat, shoulders and flowing short cloak form a clear silhouette. A compact mage hat and restrained magical details, charming but battle-ready. Consistent overhead lighting. The body center must align with the center of the canvas. Designed to remain readable at about 48-64 pixels on screen; emphasize clean outer contour over tiny details.
Composition: one isolated character only, centered, square canvas, entire silhouette visible including hat and cloak with a small transparent margin. Orthographic sprite-like presentation, no perspective environment.
Constraints: genuinely transparent background, no background scenery, no ground, no shadows outside the character, no text, no watermark, no UI, no sprite sheet, no extra characters, no huge glowing aura or particle trail. Single neutral flying pose. This is a new fantasy asset, not an edit of an existing spaceship.
```
