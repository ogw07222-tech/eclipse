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
            for (int layer = 0; layer < 3; layer++)
            {
                float parallax = Mathf.Round(offset * Factors[layer] * .25f / 6) * 6;
                for (int i = 0; i < 22; i++)
                {
                    float cx = i * Steps[layer] + layer * 47 - 100 - parallax;
                    Pine(cx, 584, Heights[layer] + (i * 19 + layer * 7) % 7 * 6, layer, i);
                }
            }
            WorldBlock(0, 578, 1600, 12, Moss);
            for (int i = 0; i < 210; i++)
            {
                float breeze = (int)(Mathf.Sin(Simulation.Clock * 1.7f + i) * 1.5f) * 6;
                WorldBlock(i * 47 % 1600 + breeze, 566 - i % 3 * 6, 6, 12, Grass);
            }
            WorldBlock(0, 585, 1600, 135, Floor); WorldBlock(0, 585, 1600, 8, Edge);
            for (int x = 0; x < 1600; x += 80) WorldBlock(x, 615, 55, 3, Stone);
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
        private void Pine(float cx, float bottom, float height, int layer, int seed)
        {
            WorldBlock(cx - 9, bottom - height * .64f, 18, height * .64f, Trunks[layer], 6);
            for (int tier = 0; tier < 4; tier++)
            {
                float width = 38 + (3 - tier) * 18, yy = bottom - height + tier * height * .19f;
                for (int row = 0; row < 5; row++)
                {
                    float rw = Mathf.Max(12, width - (4 - row) * 12);
                    WorldBlock(cx - rw * .5f, yy + row * 12, rw, 12, Leaves[layer], 6);
                }
                WorldBlock(cx - width * .23f, yy + 24, width * .3f, 6, Highlights[layer], 6);
            }
            WorldBlock(cx - 5, bottom - height + 4, 12, 6, Highlights[layer], 6);
            if (seed % 3 == 0) WorldBlock(cx + 16, bottom - height * .43f, 6, 6, Highlights[layer], 6);
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
