# ECLIPSE × Pixelbolt Skill

A reusable, **portable instruction package** for agents with filesystem/terminal access. It does not contain or bundle Pixelbolt itself, does not auto-install a ChatGPT skill, and does not update GitHub.

## Usage

1. Place `eclipse-pixelbolt-skill/` in your agent's skills directory or ECLIPSE repository as appropriate for the agent runtime.
2. In a Pixelbolt checkout, run `PIXELBOLT_CLI=/absolute/path/Pixelbolt/bin/pixelbolt.js bash eclipse-pixelbolt-skill/scripts/pixelbolt.sh help`.
3. Read `SKILL.md` and comply with the project `AGENTS.md` before touching game files.
4. Optional npx use: `PIXELBOLT_ALLOW_NPX=1 bash scripts/pixelbolt.sh help` (downloads from npm and needs network).

Official Pixelbolt can also install its upstream skill in Claude Code with `npx -y pixelbolt@latest skill install`.

## Verification state

- Skill documents, shell syntax and error handling can be checked without Pixelbolt installed.
- The actual Pixelbolt renderer must still be exercised in an environment with a Pixelbolt checkout or npm/network access.
