# 기사 / 백야의 집행자 달리기 상체 후속 모션

- 소스: `Tools/add_upper_body_followthrough.py`
- 기사: `knight_motion.png`의 432~455 (24장, 0.7초)
- 백야: `ui/white_night_motion.png`의 88~119 (32장)
- 골반 경계 아래 픽셀 및 다른 동작 슬롯은 유지한다.
- 상체는 연속된 픽셀 형태로 위상에 따라 움직이고, 왼쪽으로 흘러가는 망토/머리카락에는 지연 반동을 준다.
- 양쪽 파일을 로컬에서 생성·확인하려면 저장소 루트에서 `python3 Tools/add_upper_body_followthrough.py --dry-run` 을 실행한다.
- 생성 결과 `Art/movement-upper-review/knight_run.gif`, `white_night_run.gif`, 각 contact PNG를 먼저 정상 속도와 느린 속도로 확인한다.
- 육안 검토 후 `python3 Tools/add_upper_body_followthrough.py` 를 실행해야 **실제 게임 PNG**가 변경된다.
- 기존 `.pre-upper-motion.png` 백업이 있으면 항상 이를 기준으로 생성하므로 중복 변형하지 않는다.
- 전체 기기/SDL 플레이, 실시간 자연스러움은 검증되지 않았다. GitHub Actions 및 APK 빌드는 사용하지 않았다.
- `--dry-run`은 검토 이미지와 GIF만 생성하며 게임 PNG를 수정하지 않는다.
