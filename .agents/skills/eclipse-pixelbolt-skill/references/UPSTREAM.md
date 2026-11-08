# Pixelbolt official sources and compatibility

- Repository: https://github.com/skulitom/Pixelbolt
- Official skill: https://github.com/skulitom/Pixelbolt/blob/main/skills/pixel-art/SKILL.md
- Quick start: https://github.com/skulitom/Pixelbolt/blob/main/docs/agent-quick-start.md
- Guide: https://github.com/skulitom/Pixelbolt/blob/main/docs/agent-guide.md
- Art review: https://github.com/skulitom/Pixelbolt/blob/main/docs/art-workflow.md
- Schema: https://github.com/skulitom/Pixelbolt/blob/main/schema.json
- Examples: https://github.com/skulitom/Pixelbolt/tree/main/examples

Expected support: validate, inspect, patch, render, gif, compile, preview, import and MCP interface (check installed version's `help`). CLI is Node >=20. Upstream README describes `npx -y pixelbolt@latest skill install` for Claude Code; this ECLIPSE adaptation is a separate portable folder and is not automatically installed into ChatGPT.

CLI may have changed; verify exact flags with local help before applying them. JSON recipes are source of truth; PNG is export. A successful `validate` is structural validation, not artistic acceptance.
