#!/data/data/com.termux/files/usr/bin/bash
# Build the original game C++ into an Android ARM64 plugin WITHOUT GitHub Actions.
# Pass the Studio APK path once: ./studio-play.sh /sdcard/Download/app-debug.apk
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"
APK="${1:-${ECLIPSE_STUDIO_APK:-}}"
if ! command -v clang++ >/dev/null || ! command -v unzip >/dev/null || ! command -v readelf >/dev/null; then
  echo "Install tools in Termux: pkg install clang binutils unzip git" >&2; exit 2
fi
if [[ -z "$APK" || ! -f "$APK" ]]; then
  echo "Usage: ./studio-play.sh /sdcard/Download/ECLIPSE-Studio.apk" >&2
  echo "The Studio APK provides SDL3 shared libraries for local linking." >&2
  exit 2
fi
VENDOR="$ROOT/native-engine/android/vendor"
SDL="$VENDOR/SDL"
SDL_IMAGE="$VENDOR/SDL_image"
if [[ ! -f "$SDL/include/SDL3/SDL.h" ]]; then
  echo "Fetching official SDL 3.2.22 headers..."
  mkdir -p "$VENDOR"
  git clone --depth 1 --branch release-3.2.22 https://github.com/libsdl-org/SDL.git "$SDL"
fi
if [[ ! -f "$SDL_IMAGE/include/SDL3_image/SDL_image.h" ]]; then
  echo "Fetching SDL_image 3.2.4 headers..."
  mkdir -p "$VENDOR"
  git clone --depth 1 --branch release-3.2.4 https://github.com/libsdl-org/SDL_image.git "$SDL_IMAGE"
fi
OUT="$ROOT/studio/local-build"
LIBS="$OUT/link-libs"
mkdir -p "$LIBS"
for lib in libSDL3.so libSDL3_image.so; do
  unzip -p "$APK" "lib/arm64-v8a/$lib" > "$LIBS/$lib"
  if [[ ! -s "$LIBS/$lib" ]]; then echo "Studio APK missing $lib" >&2;exit 3;fi
done
echo "Compiling original native-engine/src/main.cpp with Termux Clang..."
clang++ -std=c++17 -O1 -g -fPIC -shared \
  -I"$SDL/include" -I"$SDL_IMAGE/include" \
  "$ROOT/native-engine/src/main.cpp" \
  -L"$LIBS" -lSDL3 -lSDL3_image \
  -Wl,-soname,libeclipse_game.so -Wl,--no-undefined \
  -static-libstdc++ \
  -o "$OUT/libeclipse_game.so"
echo "Checking ARM64 ELF and SDL_main symbol..."
readelf -h "$OUT/libeclipse_game.so" | grep -q 'AArch64' || { echo "ERROR: not AArch64" >&2;exit 4; }
if ! readelf -Ws "$OUT/libeclipse_game.so" | grep -Eq ' SDL_main$'; then
  echo "ERROR: SDL_main entry point not exported" >&2; exit 5
fi
if readelf -d "$OUT/libeclipse_game.so" | grep -E 'NEEDED' | grep -E '/data/data|libtermux|libc\+\+_shared' ; then
  echo "ERROR: plugin contains unsupported runtime dependency. Must link a standalone Android-compatible C++ runtime." >&2
  exit 6
fi
echo
echo "Build output: $OUT/libeclipse_game.so"
echo "Next: Studio → Termux .so 가져오기 → select the file → ▶ 플레이 시작"
echo "Android 16 linker/loader integration must still be verified on the tablet."
