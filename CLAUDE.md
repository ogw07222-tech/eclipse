# CLAUDE.md — ECLIPSE 게임 작업 가이드

아래 `AGENTS.md`가 프로젝트 필수 정책의 원본입니다. 작업 전에 반드시 따릅니다.

@AGENTS.md

## 핵심 금지 사항 (AGENTS.md 요약)

- GitHub Actions는 소유자의 **이번 실행에 대한 명시적 승인** 없이는 어떤 방식으로도 실행하지 않습니다 (`workflow_dispatch` 포함).
- `push`, `pull_request`, `schedule` 등 자동 트리거를 새로 추가하거나 켜지 않습니다.
- 커밋, 푸시, PR 전에 `.github/workflows/`의 트리거를 확인합니다. 현재 두 워크플로(`native-android-apk.yml`, `native-engine-checks.yml`)는 모두 `workflow_dispatch`만 있습니다.
- 빌드와 테스트는 소유자가 직접 합니다. 요청받지 않았다면 실행하지 않고, 결과는 "실행 안 함"으로 보고합니다.
- 대사, 내레이션, 자막, 말풍선 없이 이야기를 전달합니다 (`Docs/TUTORIAL_PROLOGUE_FLOW.md` 참고).
- 필수 에셋 로딩 실패는 `ECLIPSE ASSET ERROR`로 명시적으로 보고하고, 다른 장면으로 조용히 대체하지 않습니다.

## 명령 (참고용, 소유자 요청 시에만 실행)

| 목적 | 명령 |
| --- | --- |
| 데스크톱 빌드 | `cmake -S native-engine -B native-engine/build -DCMAKE_BUILD_TYPE=Release && cmake --build native-engine/build --parallel 4` |
| 헤드리스 테스트 | `cmake -S native-engine -B native-engine/build -DECLIPSE_HEADLESS=ON -DCMAKE_BUILD_TYPE=Release && ctest --test-dir native-engine/build --output-on-failure` |
| 코어 테스트 (SDL 없이) | `c++ -std=c++17 native-engine/tests/core_tests.cpp -o <scratch>/eclipse-tests && <scratch>/eclipse-tests` |
| Android APK (디버그) | `gradle -p native-engine/android --no-daemon assembleDebug` |
| APK 검증 | `python3 native-engine/tools/verify_apk.py native-engine/android/app/build/outputs/apk/debug/app-debug.apk` |

- 임시 파일은 `/tmp` 대신 세션 scratchpad 디렉터리에 둡니다.
- Android 빌드에는 JDK17, Gradle 8.11.1, SDK 35 / build-tools 35.0.0, NDK 27.2.12479018, CMake 3.31.6이 필요합니다 (`native-engine/android/README.md`).

## 구조

- `native-engine/` — C++17 + SDL3 런타임, 전투 로직, `android/` 프로젝트, `tools/` 검증 스크립트
- `Art/` — 편집 가능한 원본 아트와 키 포즈 (승인 전 작업물 포함)
- `Assets/Resources/Eclipse/` — 런타임에 들어가는 승인된 PNG만
- `Docs/` — 설계 문서. 캐릭터와 스킬은 `Docs/CHARACTER_SKILL_INDEX.md`부터 봅니다.
- `Tools/`, `tools/pixel-tools.sh` — 픽셀 아트 도구 연동
- `TASK_PROGRESS.md` — 작업 진행 기록. 작업이 중단될 수 있으면 여기에 단계, 수정 파일, 남은 일을 적습니다.

## 작업 방식

- 기존 PNG를 수정하기 전에 원본 바이너리를 먼저 확인합니다. 변경 전후 결과와 원본은 Git 히스토리로 보존합니다.
- 스크립트는 패킹, 포맷 변환, 메타데이터, 정적 검사에만 씁니다. 손으로 그린 애니메이션 프레임을 스크립트로 대신 만들어 "신규 작화"라고 보고하지 않습니다.
- 애니메이션 프레임 수와 재생 타이밍은 동작에 맞게 정합니다. 60 FPS를 맞추려고 프레임을 억지로 늘리지 않습니다.
- 작업 트리에는 소유자의 미커밋 변경이 많습니다 (`git status` 기준). 확인 없이 되돌리거나 덮어쓰지 않습니다.
- 커밋, 푸시, PR은 사용자가 요청할 때만 합니다.
