using UnityEngine;

namespace Eclipse
{
    public sealed class PixelHudView : PixelGraphic
    {
        public CombatSimulation Simulation { get; set; }
        private static readonly Color HeartOn = Hex("f56b78"), HeartOff = Hex("364451"), Slot = Hex("667482"), Empty = Hex("222c3d"), MeterOff = Hex("353a43");
        private static readonly string[] Heart = { "..XX.XX..", ".XXXXXXX.", "XXXXXXXXX", "XXXXXXXXX", ".XXXXXXX.", "..XXXXX..", "...XXX...", "....X...." };
        protected override void DrawPixels()
        {
            if (Simulation == null) return;
            var p = Simulation.Player; float ratio = (float)p.Health / p.MaxHealth;
            for (int i = 0; i < 7; i++) for (int y = 0; y < Heart.Length; y++) for (int x = 0; x < Heart[y].Length; x++)
                if (Heart[y][x] == 'X') Block(360 + i * 55 + x * 5, 18 + y * 5, 6, 6, ratio >= (i + .5f) / 7 ? HeartOn : HeartOff);
            for (int i = 0; i < 5; i++)
            {
                Block(365 + i * 48, 76, 42, 16, Ink);
                Block(369 + i * 48, 80, 34, 8, Mathf.Clamp01(1 - p.SkillCooldown / 4) > i / 5f ? Gold : MeterOff);
            }
            for (int i = 0; i < 4; i++)
            {
                Block(765 + i * 74, 21, 66, 61, Ink); Block(771 + i * 74, 27, 54, 49, Slot); Block(777 + i * 74, 33, 42, 37, Empty);
            }
            Block(1130, 30, 30, 26, Light); Block(1124, 48, 42, 12, Light);
            Block(1136, 60, 6, 12, Light); Block(1148, 60, 6, 12, Light);
            Block(1130, 40, 8, 8, Ink); Block(1148, 40, 8, 8, Ink);
            Number(Simulation.Foes, 1180, 43, Light, 5);
        }
    }
}
