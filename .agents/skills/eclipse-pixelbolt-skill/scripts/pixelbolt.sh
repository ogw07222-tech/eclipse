#!/usr/bin/env bash
set -euo pipefail
# The adapter does not install software, alter PATH, or trigger GitHub Actions.
if ! command -v node >/dev/null 2>&1; then
  echo 'ERROR: Node.js >=20 is required' >&2; exit 10
fi
NODE_MAJOR="$(node -p 'process.versions.node.split(".")[0]')"
if (( NODE_MAJOR < 20 )); then
  echo "ERROR: Node.js >=20 required; found $(node --version)" >&2; exit 10
fi
if [[ -n "${PIXELBOLT_CLI:-}" ]]; then
  if [[ ! -f "$PIXELBOLT_CLI" ]]; then echo "ERROR: no Pixelbolt CLI at $PIXELBOLT_CLI" >&2; exit 11; fi
  exec node "$PIXELBOLT_CLI" "$@"
fi
for cli in ./bin/pixelbolt.js ./Pixelbolt/bin/pixelbolt.js ../Pixelbolt/bin/pixelbolt.js; do
  if [[ -f "$cli" ]]; then exec node "$cli" "$@"; fi
done
if [[ "${PIXELBOLT_ALLOW_NPX:-0}" == 1 ]]; then
  if ! command -v npx >/dev/null 2>&1; then echo 'ERROR: npx not found' >&2; exit 12; fi
  exec npx -y pixelbolt@latest "$@"
fi
printf '%s\n' 'ERROR: Pixelbolt CLI not found. Set PIXELBOLT_CLI=/path/to/Pixelbolt/bin/pixelbolt.js' 'or explicitly set PIXELBOLT_ALLOW_NPX=1 to allow npx/network installation.' >&2
exit 12
