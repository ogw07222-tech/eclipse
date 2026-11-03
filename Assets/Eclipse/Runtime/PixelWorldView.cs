using UnityEngine;

namespace Eclipse
{
    public sealed class PixelWorldView : PixelGraphic
    {
        public CombatSimulation Simulation { get; set; }
        public float CameraX { get; set; } = 640;
        private static readonly Color Sky = Hex("152334"), Moon = Hex("aebda3"), Stars = Hex("718d8c");
        private static readonly Color[] Trunks = { Hex("243c45"), Hex("29433b"), Hex("3b4934") };
        private static readonly Color[] Leaves = { Hex("244b48"), Hex("2c5c49"), Hex("3b744c") };
        private static readonly Color[] Highlights = { Hex("33575a"), Hex("39705a"), Hex("538859") };
        private static readonly float[] Factors = { .12f, .28f, .48f };
        private static readonly int[] Steps = { 124, 110, 104 }, Heights = { 172, 210, 242 };
        private static readonly Color Moss = Hex("527b45"), Grass = Hex("83a65a"), Floor = Hex("2b4034"), Edge = Hex("73995d"), Stone = Hex("334661");
        private static readonly Color SmallEnemy = Hex("9d7abf"), BigEnemy = Hex("ba6c83"), Eye = Hex("ffed9c"), HpBack = Hex("262435"), Hp = Hex("f47777");
        private static readonly Color Blood = Hex("e34359"), BloodDark = Hex("8f183a"), Fire = Hex("f16b32"), FireDark = Hex("bb3839"), FireLight = Hex("ffb74b"), FireCore = Hex("ffe7a1"), Reaction = Hex("ffbc57");
        private static readonly Color Mage = Hex("f69a5c"), MageShade = Hex("5a2837"), Skin = Hex("f6d3b3"), Staff = Hex("8f6ad4"), NormalDamage = Hex("f2d48c"), FireDamage = Hex("ff9155"), BleedDamage = Hex("e76878");
        private float offset;
        protected override void DrawPixels()
        {
            if (Simulation == null) return;
            offset = CameraX - 640;
            Block(0, 0, 1280, 720, Sky);
            for (int row = 0; row < 11; row++)
            {
                int half = 2 + Mathf.Min(row, 10 - row);
                WorldBlock(1190 - half * 6, 105 + row * 6, (half * 2 + 1) * 6, 6, Moon);
            }
            for (int i = 0; i < 62; i++) WorldBlock((i * 233 + 49) % 1600, (i * 97 + 59) % 315, 6, 6, Stars);
            // Deterministic, pixel-snapped parallax: uneven woodland rather than repeated pines.
            for (int layer = 0; layer < 3; layer++)
            {
                float shift = Mathf.Round(offset * Factors[layer] / 6f) * 6f;
                for (int i = -3; i < 22; i++)
                {
                    int seed = i + layer * 173;
                    float cx = i * Steps[layer] + (Hash(seed, 1) % 65 - 32) - shift;
                    float height = Heights[layer] + (Hash(seed, 2) % 12 - 5) * 6;
                    ForestTree(cx, 584, height, layer, seed);
                }
                for (int i = -2; i < 35; i++)
                {
                    int seed = i + layer * 391;
                    float bx = i * 61 + Hash(seed, 9) % 42 - shift;
                    Bush(bx, 579 - layer * 3, 21 + Hash(seed, 8) % 23, layer, seed);
                }
            }
            WorldBlock(0, 578, 1600, 12, Moss);
            WorldBlock(0, 585, 1600, 135, Floor);
            WorldBlock(0, 585, 1600, 8, Edge);
            // Ground stays in world space; vegetation remains decorative and never affects collision.
            for (int i = 0; i < 115; i++)
            {
                int seed = i + 861;
                float x = i * 15 + Hash(seed, 3) % 12;
                if (x > 1600) break;
                float sway = Mathf.Round(Mathf.Sin(Simulation.Clock * 1.5f + seed) * 1f) * 3f;
                WorldBlock(x + sway, 574 - Hash(seed, 4) % 3 * 6, 6, 6 + Hash(seed, 5) % 3 * 6,
                    i % 7 == 0 ? Grass : i % 3 == 0 ? Moss : Edge, 3);
                if (i % 11 == 0)
                {
                    WorldBlock(x - 6, 581, 18, 6, Stone, 3);
                    WorldBlock(x, 578, 6, 3, Hex("62736b"), 3);
                }
                if (i % 19 == 0)
                {
                    WorldBlock(x, 562, 3, 15, Moss, 3);
                    WorldBlock(x - 3, 556, 9, 6, Hex("b6aa88"), 3);
                }
            }
            for (int i = 0; i < 45; i++)
            {
                int seed = 1171 + i;
                float x = i * 37 + Hash(seed, 2) % 20;
                WorldBlock(x, 604 + Hash(seed, 3) % 15 * 6, 6 + Hash(seed, 4) % 4 * 6, 6,
                    i % 4 == 0 ? Stone : Hex("3a5141"), 6);
            }
            foreach (var e in Simulation.Enemies) if (!e.Dead) Enemy(e);
            foreach (var b in Simulation.Projectiles)
            {
                Disk(b.X - offset, b.Y, b.Empowered ? 14 : 8, Fire);
                Disk(b.X - offset, b.Y, b.Empowered ? 7 : 4, FireCore);
            }
            var p = Simulation.Player;
            if (p.Form == 1) DrawMage(p);
            if (p.SwapFlash > 0) Ring(p.X - offset, p.Y - 36, 44, Gold);
            foreach (var popup in Simulation.Popups)
                Number(popup.Amount, popup.X - offset - 12, popup.Y, popup.Kind == "fire" ? FireDamage : popup.Kind == "bleed" ? BleedDamage : NormalDamage);
        }
        private void WorldBlock(float x, float y, float w, float h, Color tint, float grid = 1) => Block(x - offset, y, w, h, tint, grid);
        // A stable integer hash keeps trees unchanged between frames and play sessions.
        private static int Hash(int seed, int salt)
        {
            unchecked
            {
                uint h = (uint)(seed * 374761393 + salt * 668265263);
                h = (h ^ (h >> 13)) * 1274126177u;
                return (int)((h ^ (h >> 16)) & 0x7fffffff);
            }
        }
        private void ForestTree(float cx, float bottom, float height, int layer, int seed)
        {
            float trunk = 12 + Hash(seed, 6) % 3 * 6;
            float crownY = bottom - height * (.62f + (Hash(seed, 7) % 9) * .025f);
            Color bark = Trunks[layer], leaf = Leaves[layer], bright = Highlights[layer];
            // Broadleaf trees and occasional uneven conifers, not identical tiered cones.
            Block(cx - trunk * .5f, crownY, trunk, bottom - crownY, bark, 6);
            if (seed % 4 == 0)
            {
                Block(cx - 24, crownY + 24, 30, 6, bark, 6);
                Block(cx + 3, crownY + 12, 30, 6, bark, 6);
            }
            else
            {
                Block(cx - 30, crownY + 36, 33, 6, bark, 6);
                Block(cx + 3, crownY + 24, 33, 6, bark, 6);
            }
            float radius = 38 + Hash(seed, 10) % 5 * 6 + layer * 5;
            if (seed % 7 == 0)
            {
                // Irregular needle clusters for a small proportion of forest trees.
                for (int tier = 0; tier < 3; tier++)
                {
                    float y = bottom - height + tier * 39;
                    float w = radius * (1.1f + tier * .15f);
                    for (int row = 0; row < 5; row++)
                        Block(cx - w * .5f + row * 5, y + row * 9, w - row * 10, 12, leaf, 6);
                }
            }
            else
            {
                // Offset voxel clusters create overlapping, asymmetric crowns.
                for (int clump = 0; clump < 7; clump++)
                {
                    int h = Hash(seed, 20 + clump);
                    float x = cx + (h % 17 - 8) * 6;
                    float y = bottom - height + ((h / 19) % 9) * 6 + clump % 3 * 15;
                    float r = radius * (.42f + (h / 37 % 5) * .08f);
                    for (int row = -3; row <= 3; row++)
                    {
                        float width = r * (1f - Mathf.Abs(row) * .16f);
                        Block(x - width, y + row * 9, width * 2, 12, leaf, 6);
                    }
                }
            }
            for (int k = 0; k < 6; k++)
            {
                int h = Hash(seed, 51 + k);
                float xx = cx + (h % 17 - 8) * 6;
                float yy = bottom - height + (h / 23 % 11) * 6;
                Block(xx, yy, 12 + h % 3 * 6, 6, bright, 6);
            }
            if (layer == 2 && seed % 5 == 0)
            {
                Block(cx - 3, bottom - 38, 6, 30, Hex("43604a"), 6);
                Block(cx + 6, bottom - 24, 6, 12, Moss, 6);
            }
        }
        private void Bush(float x, float bottom, float radius, int layer, int seed)
        {
            Color foliage = Leaves[layer], glint = Highlights[layer];
            for (int n = 0; n < 4; n++)
            {
                int h = Hash(seed, 70 + n);
                float bx = x + (n - 2) * 12 + h % 9;
                float top = bottom - radius - h % 13;
                Block(bx - 12, top + 9, 30, radius * .65f, foliage, 6);
                Block(bx - 6, top, 18, 12, foliage, 6);
                if (n % 2 == 0) Block(bx, top + 6, 12, 6, glint, 6);
            }
        }
        private void Enemy(EnemyState e)
        {
            float x = e.X, y = e.Y, half = e.Kind == 0 ? 16 : 21, h = e.Kind == 0 ? 48 : 62;
            WorldBlock(x - half, y - h, half * 2, h, e.HitFlash > 0 ? Color.white : e.Kind == 0 ? SmallEnemy : BigEnemy);
            Disk(x - 7 - offset, y - h + 16, 4, Eye, 2); Disk(x + 7 - offset, y - h + 16, 4, Eye, 2);
            WorldBlock(x - 21, y - h - 15, 42, 5, HpBack); WorldBlock(x - 21, y - h - 15, 42f * e.Health / e.MaxHealth, 5, Hp);
            if (e.Bleed > 0) for (int i = 0; i < 6; i++)
            {
                float phase = Mathf.Repeat(Simulation.Clock * (17 + i % 3 * 4) + i * 11, h);
                WorldBlock(x + (i * 13 % 29 - 14), y - h + phase, 3, i % 2 == 0 ? 6 : 3, i % 2 == 0 ? Blood : BloodDark, 3);
            }
            if (e.Burn > 0) for (int i = 0; i < 9; i++)
            {
                float rise = Mathf.Repeat(Simulation.Clock * (29 + i % 4 * 6) + i * 9, h + 18);
                WorldBlock(x + i * 17 % 35 - 17 + Mathf.Sin(Simulation.Clock * 5 + i) * 3, y + 4 - rise,
                    i % 3 == 0 ? 6 : 3, i % 2 == 0 ? 9 : 6, i % 3 == 0 ? FireLight : i % 3 == 1 ? Fire : FireDark, 3);
            }
            if (e.ReactionFlash > 0) for (int i = 0; i < 8; i++)
                WorldBlock(x + i % 4 * 17 - 26, y - h * .5f + i / 4 * 18 - 9, 9, 6, Reaction, 3);
        }
        private void DrawMage(PlayerState p)
        {
            float x = p.X, y = p.Y;
            Color core = Mage; if (p.Invulnerable > 0 && (int)(Simulation.Clock * 60) % 6 < 3) core.a = .4f;
            WorldBlock(x - 15, y - 55, 30, 40, MageShade); WorldBlock(x - 12, y - 52, 24, 30, core);
            Disk(x - offset, y - 60, 15, Skin, 3); WorldBlock(x - 12, y - 70, 24, 10, MageShade);
            WorldBlock(x + p.Facing * 7 - 2, y - 62, 4, 4, Ink);
            WorldBlock(x - 12, y - 15, 10, 15, MageShade); WorldBlock(x + 2, y - 15, 10, 15, MageShade);
            for (int i = 0; i < 9; i++) WorldBlock(x + p.Facing * (15 + i), y - 35 - i * 3, 6, 6, Staff, 3);
            Disk(x + p.Facing * 22 - offset, y - 64, 7, FireLight, 3);
            if (p.AttackFlash > 0) Ring(x + p.Facing * 16 - offset, y - 36, 40, FireLight, 4);
        }
    }
}
