# Native foundation validation — 2026-11-02

Baseline: main `1a5b3d7b461e12cf691628004408dd8df7def333` (native prototype PR #20).

## Executed

- C++17 core test compiled with `g++ -std=c++17 -Wall -Wextra -Werror` and passed.
- CMake Release build compiled actual SDL3 3.2.22, SDL3_image 3.2.4 and ECLIPSE, not mocked APIs.
- CTest: **2/2 passed** (`eclipse_core`, `eclipse_png_render_smoke`). The PNG smoke test uses the real SDL dummy video/software renderer, loads the original knight texture, validates 384×448 dimensions and presents a frame.
- Extra execution from `/tmp` passed, proving assets do not depend on the shell's working directory.
- Copy of executable without packaged PNG returned failure with the expected diagnostic.
- Packaged knight is byte-identical to source: 104,154 compressed PNG bytes.
- CMake reports image backends enabled: `stb png`; other image decoders and encoders are off. SDL audio/camera/dialog/GPU/haptics/joystick/sensors/Vulkan/test-library are off.
- Native Actions YAML passed actionlint 1.7.7; Git diff whitespace check passed.

For local compilation, dependency source trees were downloaded at the pinned release tags and supplied through CMake's FETCHCONTENT_SOURCE_DIR overrides. The workflow uses clean FetchContent downloads with submodules explicitly disabled, then the same compile/test targets.

## Size snapshot

Linux x86_64 **headless** Release, stripped copies measured with `strip --strip-unneeded`:

| Component | Bytes |
| --- | ---: |
| ECLIPSE executable | 26,896 |
| SDL3 shared library | 2,413,312 |
| SDL3_image PNG library | 121,368 |
| Original compressed knight PNG | 104,154 |

These are native file sizes for this CI-like build. They exclude OS C/C++ libraries, graphics backends disabled in headless mode, Android bootstrap, packaging/signing and runtime memory. This is **not an APK size or device performance estimate**. No claim of improvement percentage is possible because a comparable baseline binary was not measured.

## Pending

Interactive desktop rendering/resize/focus/device multitouch, Android NDK/Gradle packaging, APK installation, full ECLIPSE gameplay parity, on-device frame time and memory. No Android build or visual screenshot is provided by these tests.
