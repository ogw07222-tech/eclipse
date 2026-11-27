# ECLIPSE GitHub workspace

[Game repository](https://github.com/ogw07222-tech/eclipse) · [Private resource archive](https://github.com/ogw07222-tech/eclipse-resources)

Use `ogw07222-tech/eclipse` on `main` as the canonical game workspace. It contains the SDL3 game, its Android project, gameplay documentation, and the assets needed for normal checkout and local builds.

| Location | Contents |
| --- | --- |
| `eclipse/Assets/Resources/Eclipse/` | Integrated runtime images, atlases, and other packaged game assets |
| `eclipse/Art/` | Editable source art, reviewed sprite work, and the associated mappings |
| `eclipse-resources/` | Large reference models, alternate environment art, review renders, and third-party source textures |

Clone the resource archive beside the game when working on source art. Its catalog identifies the exact file and SHA-256 for each archived asset.

```sh
git clone https://github.com/ogw07222-tech/eclipse.git
git clone https://github.com/ogw07222-tech/eclipse-resources.git
```

When promoting archive artwork to the game, add the game-ready asset to its appropriate runtime path and keep the mapping, packaging, and art notes with the game change. Reference the archived path and commit when they help reproduce the result.

See [AGENTS.md](../AGENTS.md) for the owner-only testing and workflow policy.
