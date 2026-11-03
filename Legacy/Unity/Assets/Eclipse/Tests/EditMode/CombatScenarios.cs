using System;

namespace Eclipse.Tests
{
    // Shared assertions run both in Unity EditMode and standalone .NET, against production code.
    public static class CombatScenarios
    {
        public static readonly string[] Names = {
            "Initial wave", "Movement and walls", "Jump apex and landing", "Dash immunity and cooldown",
            "Three-hit combo and bleed", "Combo timeout", "Bleed-fire reaction", "Damage-over-time",
            "Swap entry attacks and cooldown", "Mage skill projectile", "Equipment stats and health",
            "Player damage and defeat", "Stage clear", "Restart cleanup", "Facing and vertical melee range"
        };
        private static void Check(bool value, string message) { if (!value) throw new Exception(message); }
        private static void Near(float actual, float expected, float tolerance = .02f) { Check(Math.Abs(actual - expected) <= tolerance, $"Expected {expected}, got {actual}"); }
        private static void Wait(CombatSimulation game, int frames) { for (int i = 0; i < frames; i++) game.Step(CombatSimulation.FixedStep, default); }
        private static CombatSimulation EmptyArena()
        {
            var g = new CombatSimulation();
            // Keep one distant enemy so timed scenarios do not automatically clear the stage.
            for (int i = 0; i < g.Enemies.Count; i++) { g.Enemies[i].X = 1500; g.Enemies[i].AttackTimer = 1000; }
            return g;
        }
        public static void Run(int scenario)
        {
            var g = EmptyArena(); var p = g.Player; var e = g.Enemies[0];
            switch (scenario)
            {
                case 0:
                    g = new CombatSimulation(); Check(g.Foes == 4 && g.Player.Health == 100 && !g.Ended, "Initial state");
                    Check(g.Enemies[0].Health == 55 && g.Enemies[1].Health == 85, "Enemy types"); Near(g.Enemies[3].X, 1285); break;
                case 1:
                    g.Step(CombatSimulation.FixedStep, new CombatInput { Axis = 1 }); Near(p.X, 140 + 280f / 60);
                    p.X = 1549; g.Step(CombatSimulation.FixedStep, new CombatInput { Axis = 1 }); Near(p.X, 1550);
                    p.X = 26; g.Step(CombatSimulation.FixedStep, new CombatInput { Axis = -1 }); Near(p.X, 25); Check(p.Facing == -1, "Facing"); break;
                case 2:
                    g.Step(CombatSimulation.FixedStep, new CombatInput { Jump = true }); Check(!p.Grounded && p.VY < 0, "Jump activation");
                    float highest = p.Y;
                    for (int i = 0; i < 90; i++) { g.Step(CombatSimulation.FixedStep, default); highest = Math.Min(highest, p.Y); }
                    Check(CombatSimulation.FloorY - highest > 160 && CombatSimulation.FloorY - highest < 180, "Jump height");
                    Check(p.Grounded && p.VY == 0, "Landing"); break;
                case 3:
                    g.Step(CombatSimulation.FixedStep, new CombatInput { Dash = true }); Near(p.VX, 780); g.DamagePlayer(15); Check(p.Health == 100, "Dash immunity");
                    Wait(g, 12); float cooldown = p.DashCooldown; g.Step(CombatSimulation.FixedStep, new CombatInput { Dash = true });
                    Check(p.DashLeft == 0 && p.DashCooldown < cooldown, "Dash cooldown"); Wait(g, 60); g.Step(CombatSimulation.FixedStep, new CombatInput { Dash = true }); Check(p.DashLeft > 0, "Dash reactivation"); break;
                case 4:
                    // Reset target position after each hit to isolate combo timing from recoil.
                    e.X = p.X + 60; g.Step(CombatSimulation.FixedStep, new CombatInput { Attack = true }); Check(e.Health == 43 && p.Combo == 1, "First hit");
                    Wait(g, 16); e.X = p.X + 60; g.Step(CombatSimulation.FixedStep, new CombatInput { Attack = true }); Check(e.Health == 28 && p.Combo == 2, "Second hit");
                    Wait(g, 17); e.X = p.X + 60; g.Step(CombatSimulation.FixedStep, new CombatInput { Attack = true });
                    Check(e.Health == 5 && p.Combo == 3 && e.Bleed > 4, "Finisher bleed"); break;
                case 5:
                    g.Step(CombatSimulation.FixedStep, new CombatInput { Attack = true }); Wait(g, 55); g.Step(CombatSimulation.FixedStep, new CombatInput { Attack = true }); Check(p.Combo == 1, "Combo reset"); break;
                case 6:
                    g.ReceiveHit(e, 1, false, true); g.ReceiveHit(e, 11, true); Check(e.Health == 21 && e.Bleed == 0 && e.Burn > 0 && e.ReactionFlash > 0, "Consume bleed for +22");
                    g.ReceiveHit(e, 1, true); Check(e.Health == 20, "Reaction must not repeat without bleed"); break;
                case 7:
                    g.ReceiveHit(e, 1, false, true); Wait(g, 46); Check(e.Health == 51, "Bleed tick");
                    var fireTarget = g.Enemies[1]; g.ReceiveHit(fireTarget, 1, true); Wait(g, 37); Check(fireTarget.Health == 82, "Burn tick"); break;
                case 8:
                    g.Step(CombatSimulation.FixedStep, new CombatInput { Swap = true }); Check(p.Form == 1 && g.Projectiles.Count == 1 && g.Projectiles[0].Damage == 14, "Mage entry");
                    g.Step(CombatSimulation.FixedStep, new CombatInput { Swap = true }); Check(p.Form == 1 && g.Projectiles.Count == 1, "Swap cooldown");
                    Wait(g, 61); e.X = p.X + 60; g.Step(CombatSimulation.FixedStep, new CombatInput { Swap = true }); Check(p.Form == 0 && e.Health == 42, "Sword entry"); break;
                case 9:
                    p.Form = 1; g.Step(CombatSimulation.FixedStep, new CombatInput { Skill = true }); Check(g.Projectiles.Count == 1 && g.Projectiles[0].Empowered, "Skill spawn");
                    Near(p.SkillCooldown, 4.1f); e.X = p.X + 90; Wait(g, 12); Check(e.Health == 30 && e.Burn > 0, "Fire projectile damage"); break;
                case 10:
                    p.CycleEquipment(0); p.CycleEquipment(1); Check(p.Gear.Attack == 6 && p.Gear.Defense == 4 && p.MaxHealth == 130 && p.Health == 130, "Weapon and armor"); Near(p.Gear.Speed, .792f); Near(p.Gear.Range, 1.16f);
                    p.CycleEquipment(2); Check(p.Gear.Attack == 9 && p.Gear.Id(0) == "blood_blade", "Charm and stable IDs");
                    p.CycleEquipment(1); Check(p.Health == 110 && p.MaxHealth == 110, "Max health adjustment");
                    p.CycleEquipment(2); Check(p.Gear.Defense == 3 && p.MaxHealth == 125, "Guardian charm"); break;
                case 11:
                    g.DamagePlayer(9); g.DamagePlayer(9); Check(p.Health == 91, "Invulnerability"); Wait(g, 45); g.DamagePlayer(999); Check(g.Ended && !g.Won && p.Health == 0, "Defeat");
                    float x = p.X; g.Step(CombatSimulation.FixedStep, new CombatInput { Axis = 1, Attack = true }); Near(p.X, x); break;
                case 12:
                    foreach (var enemy in g.Enemies) g.ReceiveHit(enemy, 1000, false);
                    g.Step(CombatSimulation.FixedStep, default); Check(g.Ended && g.Won && g.Foes == 0, "Clear"); break;
                case 13:
                    p.Form = 1; p.CycleEquipment(0); g.Step(CombatSimulation.FixedStep, new CombatInput { Attack = true }); g.ReceiveHit(e, 1, true);
                    g.Restart(); Check(g.Foes == 4 && g.Projectiles.Count == 0 && g.Popups.Count == 0 && !g.Ended, "Cleanup");
                    Check(g.Player.Form == 0 && g.Player.Health == 100 && g.Player.Gear.Id(0) == "rusty_sword", "Player reset"); break;
                case 14:
                    e.X = p.X - 60; g.Step(CombatSimulation.FixedStep, new CombatInput { Attack = true }); Check(e.Health == 55, "No rear hit");
                    Wait(g, 18); p.Y = 400; p.Grounded = false; e.X = p.X + 60; g.Step(CombatSimulation.FixedStep, new CombatInput { Skill = true }); Check(e.Health == 55, "No vertical out-of-range hit"); break;
                default: throw new ArgumentOutOfRangeException(nameof(scenario));
            }
        }
    }
}
