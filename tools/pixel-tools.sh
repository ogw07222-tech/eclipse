#!/usr/bin/env bash
# ECLIPSE pixel-art tool adapter. No GitHub Actions, no game asset overwrites.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CACHE="${ECLIPSE_PIXEL_TOOLS_HOME:-${HOME}/.cache/eclipse-pixel-tools}"
BOLT="$CACHE/Pixelbolt"
FX="$CACHE/pixelfx"
OUT="$ROOT/Art/tool-previews"
usage() {
  cat <<'HELP'
Usage: bash tools/pixel-tools.sh COMMAND [args...]
  install                 Clone Pixelbolt & PixelFX locally (does not modify game assets)
  versions                Show installed Git revision IDs
  bolt-preview [recipe]   Open Pixelbolt Studio (Ctrl+C to stop)
  bolt-inspect RECIPE     Produce contact sheet/inspection output
  bolt-render RECIPE NAME Render PNGs under Art/tool-previews/pixelbolt/NAME
  fx-gallery              Start local PixelFX gallery (Ctrl+C to stop)
  fx-sheet EFFECT_ID      Export PixelFX sprite sheet + JSON (e.g. melee/sword-slash)
  fx-check EFFECT_ID      Check an effect
No game-runtime asset is replaced automatically; artist approval is required.
HELP
}
require_node() {
  command -v node >/dev/null || { echo "Node.js 20+ is required" >&2; exit 2; }
  node -e 'if(+process.versions.node.split(".")[0]<20)process.exit(1)' || { echo "Node.js 20+ is required" >&2; exit 2; }
}
require_repo() { [[ -f "$1/package.json" ]] || { echo "Not installed: $1. Run: bash tools/pixel-tools.sh install" >&2; exit 2; }; }
safe_name() { [[ "$1" =~ ^[a-zA-Z0-9_-]+$ ]] || { echo "Name must contain only letters, numbers, _ or -" >&2; exit 2; }; }
cmd="${1:-help}"; shift || true
case "$cmd" in
  install)
    require_node
    command -v git >/dev/null || { echo "git is required" >&2; exit 2; }
    mkdir -p "$CACHE"
    if [[ ! -d "$BOLT/.git" ]]; then git clone --depth 1 https://github.com/skulitom/Pixelbolt.git "$BOLT"; fi
    if [[ ! -d "$FX/.git" ]]; then git clone --depth 1 https://github.com/patch-gg/pixelfx.git "$FX"; fi
    # Pixelbolt has zero runtime deps; PixelFX dependencies are installed locally.
    (cd "$FX" && npm ci --ignore-scripts)
    echo "Pinned to current local clones. No automatic git pull."
    "$0" versions
    ;;
  versions)
    for folder in "$BOLT" "$FX"; do
      if [[ -d "$folder/.git" ]]; then printf '%s: ' "$folder"; git -C "$folder" rev-parse HEAD; else echo "$folder: not installed"; fi
    done
    ;;
  bolt-preview)
    require_node; require_repo "$BOLT"
    (cd "$BOLT" && node bin/pixelbolt.js preview "$@")
    ;;
  bolt-inspect)
    require_node; require_repo "$BOLT"
    [[ "$#" -eq 1 ]] || { usage; exit 2; }
    # Keep the input location absolute before changing directory.
    recipe="$(realpath "$1")"
    mkdir -p "$OUT/pixelbolt"
    (cd "$BOLT" && node bin/pixelbolt.js inspect "$recipe" --out "$OUT/pixelbolt/contact.png")
    ;;
  bolt-render)
    require_node; require_repo "$BOLT"
    [[ "$#" -eq 2 ]] || { usage; exit 2; }
    recipe="$(realpath "$1")"; safe_name "$2"
    target="$OUT/pixelbolt/$2"
    [[ ! -e "$target" ]] || { echo "Output exists: $target; choose another name" >&2; exit 2; }
    mkdir -p "$OUT/pixelbolt"
    (cd "$BOLT" && node bin/pixelbolt.js render "$recipe" --out "$target")
    ;;
  fx-gallery)
    require_node; require_repo "$FX"
    (cd "$FX" && node tools/serve.mjs)
    ;;
  fx-sheet)
    require_node; require_repo "$FX"
    [[ "$#" -eq 1 && "$1" =~ ^[a-zA-Z0-9_-]+/[a-zA-Z0-9_-]+$ ]] || { usage; exit 2; }
    # PixelFX's renderer owns its export directory. Never copy to runtime automatically.
    (cd "$FX" && node tools/render.mjs "$1" --sheet)
    echo "Check generated sheet and JSON under PixelFX local checkout; copy approved output into Art/ first."
    ;;
  fx-check)
    require_node; require_repo "$FX"
    [[ "$#" -eq 1 && "$1" =~ ^[a-zA-Z0-9_-]+/[a-zA-Z0-9_-]+$ ]] || { usage; exit 2; }
    (cd "$FX" && node tools/check.mjs "$1")
    ;;
  help|-h|--help) usage ;;
  *) usage; exit 2 ;;
esac
