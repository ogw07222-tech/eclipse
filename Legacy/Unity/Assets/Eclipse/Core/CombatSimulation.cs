using System;
using System.Collections.Generic;

namespace Eclipse
{
    // Godot's pixel coordinates and 60 Hz step are retained deliberately.
    // This model has no Unity dependency; rendering and input are adapters.
    public struct CombatInput
    {
        public float Axis;
        public bool Jump, Dash, Attack, Skill, Swap;
    }

    public sealed class Equipment
    {
        public static readonly string[][] Ids = {
            new[] { "rusty_sword", "blood_blade", "quick_blade" },
            new[] { "cloth_armor", "iron_armor", "scout_armor" },
            new[] { "empty_charm", "flame_charm", "guardian_charm" }
        };
        public static readonly string[][] Names = {
            new[] { "낡은 장검", "핏빛 검", "질풍검" },
            new[] { "천 갑옷", "철 갑옷", "정찰 갑옷" },
            new[] { "장신구 없음", "불꽃 부적", "수호 부적" }
        };
        private readonly int[] selected = new int[3];
        public int Attack => (selected[0] == 1 ? 6 : selected[0] == 2 ? -2 : 0) + (selected[2] == 1 ? 3 : 0);
        public int Defense => (selected[1] == 1 ? 4 : selected[1] == 2 ? 1 : 0) + (selected[2] == 2 ? 2 : 0);
        public int BonusHealth => (selected[1] == 1 ? 30 : selected[1] == 2 ? 10 : 0) + (selected[2] == 2 ? 15 : 0);
        public float Speed => (selected[0] == 1 ? .88f : selected[0] == 2 ? 1.3f : 1f) * (selected[1] == 1 ? .9f : selected[1] == 2 ? 1.1f : 1f);
        public float Range => selected[0] == 1 ? 1.16f : selected[0] == 2 ? 1.08f : 1f;
        public string Name(int slot) => Names[slot][selected[slot]];
        public string Id(int slot) => Ids[slot][selected[slot]];
        public void Cycle(int slot) { if (slot >= 0 && slot < 3) selected[slot] = (selected[slot] + 1) % 3; }
    }

    public sealed class PlayerState
    {
        public readonly Equipment Gear = new Equipment();
        public float X = 140, Y = CombatSimulation.FloorY, VX, VY;
        public int Health = 100, Form, Facing = 1, Combo, AttackSerial, HurtSerial;
        public int MaxHealth => 100 + Gear.BonusHealth;
        public bool Grounded = true;
        public float DashLeft, DashCooldown, AttackCooldown, SkillCooldown, SwapCooldown;
        public float Invulnerable, Coyote, JumpBuffer, AttackFlash, ComboWindow, HitStop, SwapFlash;
        public void CycleEquipment(int slot)
        {
            int oldMax = MaxHealth;
            Gear.Cycle(slot);
            Health = Math.Max(1, Math.Min(MaxHealth, Health + MaxHealth - oldMax));
        }
    }

    public sealed class EnemyState
    {
        public readonly int Kind, MaxHealth, AttackDamage;
        public readonly float Speed;
        public float X, Y = CombatSimulation.FloorY, AttackTimer, Bleed, BleedTick, Burn, BurnTick;
        public float HitFlash, ReactionFlash, Knockback;
        public int Health;
        public bool Dead => Health <= 0;
        public EnemyState(int kind, float x)
        {
            Kind = kind; X = x;
            Health = MaxHealth = kind == 0 ? 55 : 85;
            Speed = kind == 0 ? 95 : 63;
            AttackDamage = kind == 0 ? 9 : 15;
        }
    }

    public sealed class ProjectileState
    {
        public float X, Y, Life = 1.6f;
        public int Facing, Damage;
        public bool Empowered;
    }

    public sealed class DamagePopup
    {
        public float X, Y, Age;
        public int Amount;
        public string Kind;
    }

    public sealed class CombatSimulation
    {
        public const float FloorY = 585, WorldWidth = 1600, FixedStep = 1f / 60;
        public PlayerState Player { get; private set; }
        public readonly List<EnemyState> Enemies = new List<EnemyState>(4);
        public readonly List<ProjectileState> Projectiles = new List<ProjectileState>(16);
        public readonly List<DamagePopup> Popups = new List<DamagePopup>(32);
        public bool Ended { get; private set; }
        public bool Won { get; private set; }
        public int Foes { get { int n = 0; foreach (var e in Enemies) if (!e.Dead) n++; return n; } }
        public float Clock { get; private set; }
        public CombatSimulation() { Restart(); }
        public void Restart()
        {
            Player = new PlayerState();
            Enemies.Clear(); Projectiles.Clear(); Popups.Clear();
            float[] positions = { 575, 800, 1020, 1285 };
            for (int i = 0; i < positions.Length; i++) Enemies.Add(new EnemyState(i % 2, positions[i]));
            Ended = Won = false; Clock = 0;
        }
        private static float Down(float value, float dt) => Math.Max(0, value - dt);
        private static float Toward(float value, float amount) => value > 0 ? Math.Max(0, value - amount) : Math.Min(0, value + amount);
        public void Step(float dt, CombatInput input)
        {
            if (dt <= 0 || dt > .1f) throw new ArgumentOutOfRangeException(nameof(dt));
            Clock += dt;
            for (int i = Popups.Count - 1; i >= 0; i--)
            {
                Popups[i].Age += dt; Popups[i].Y -= 27 * dt;
                if (Popups[i].Age >= .6f) Popups.RemoveAt(i);
            }
            if (Ended) return;
            StepPlayer(dt, input);
            foreach (var enemy in Enemies) { StepEnemy(enemy, dt); if (Ended) break; }
            if (!Ended) StepProjectiles(dt);
            if (!Ended && Foes == 0) { Won = true; Ended = true; }
        }
        private void StepPlayer(float dt, CombatInput input)
        {
            var p = Player;
            p.DashLeft = Down(p.DashLeft, dt); p.DashCooldown = Down(p.DashCooldown, dt);
            p.AttackCooldown = Down(p.AttackCooldown, dt); p.SkillCooldown = Down(p.SkillCooldown, dt);
            p.SwapCooldown = Down(p.SwapCooldown, dt); p.Invulnerable = Down(p.Invulnerable, dt);
            p.AttackFlash = Down(p.AttackFlash, dt); p.ComboWindow = Down(p.ComboWindow, dt);
            p.SwapFlash = Down(p.SwapFlash, dt);
            if (p.HitStop > 0) { p.HitStop = Down(p.HitStop, dt); return; }
            if (p.ComboWindow <= 0) p.Combo = 0;
            p.Coyote = p.Grounded ? .11f : Down(p.Coyote, dt);
            p.JumpBuffer = input.Jump ? .13f : Down(p.JumpBuffer, dt);
            if (Math.Abs(input.Axis) > .1f) p.Facing = input.Axis > 0 ? 1 : -1;
            if (input.Swap && p.SwapCooldown <= 0)
            {
                p.Form = 1 - p.Form; p.SwapCooldown = 1; p.AttackFlash = .22f;
                p.Combo = 0; p.ComboWindow = 0; p.SwapFlash = .24f; p.AttackSerial++;
                if (p.Form == 0) Melee(13, false, 112); else SpawnProjectile(14, false);
            }
            if (input.Dash && p.DashCooldown <= 0)
            {
                p.DashLeft = .15f; p.DashCooldown = .85f; p.Invulnerable = Math.Max(p.Invulnerable, .15f);
            }
            if (p.DashLeft > 0) { p.VX = 780 * p.Facing * p.Gear.Speed; p.VY = 0; }
            else
            {
                p.VX = input.Axis * 280 * p.Gear.Speed; p.VY += 1250 * dt;
                if (p.JumpBuffer > 0 && p.Coyote > 0) { p.VY = -650; p.Coyote = p.JumpBuffer = 0; }
            }
            if (input.Attack && p.AttackCooldown <= 0) BasicAttack();
            if (input.Skill && p.SkillCooldown <= 0) UseSkill();
            p.X = Math.Max(25, Math.Min(1550, p.X + p.VX * dt)); p.Y += p.VY * dt;
            p.Grounded = p.Y >= FloorY;
            if (p.Grounded) { p.Y = FloorY; p.VY = 0; }
        }
        private void BasicAttack()
        {
            var p = Player; p.AttackFlash = .16f; p.AttackSerial++;
            if (p.Form == 0)
            {
                p.Combo = p.ComboWindow > 0 ? p.Combo % 3 + 1 : 1; p.ComboWindow = .8f;
                p.AttackCooldown = (p.Combo == 1 ? .23f : p.Combo == 2 ? .25f : .42f) / p.Gear.Speed;
                int power = p.Combo == 1 ? 12 : p.Combo == 2 ? 15 : 23;
                Melee(power + p.Gear.Attack, p.Combo == 3, (p.Combo == 3 ? 115 : 96) * p.Gear.Range);
            }
            else { p.Combo = 0; p.AttackCooldown = .47f; SpawnProjectile(11 + p.Gear.Attack, false); }
        }
        private void UseSkill()
        {
            var p = Player; p.SkillCooldown = p.Form == 0 ? 3.4f : 4.1f; p.AttackFlash = .28f; p.AttackSerial++;
            if (p.Form == 0) Melee(23 + p.Gear.Attack, true, 96 * p.Gear.Range);
            else SpawnProjectile(25 + p.Gear.Attack, true);
        }
        private void Melee(int amount, bool bleed, float reach)
        {
            var p = Player;
            foreach (var e in Enemies)
            {
                float dx = (e.X - p.X) * p.Facing;
                if (e.Dead || dx < -9 || dx >= reach || Math.Abs(e.Y - p.Y) >= 67) continue;
                ReceiveHit(e, amount, false, bleed);
                p.HitStop = Math.Max(p.HitStop, amount < 20 ? .045f : .075f);
            }
        }
        private void SpawnProjectile(int power, bool empowered)
        {
            Projectiles.Add(new ProjectileState { X = Player.X + Player.Facing * 27, Y = Player.Y - 33,
                Facing = Player.Facing, Damage = power, Empowered = empowered });
        }
        public void ReceiveHit(EnemyState e, int amount, bool fire, bool bleed = false)
        {
            if (e.Dead || Ended) return;
            if (fire)
            {
                e.Burn = 3; e.BurnTick = 0;
                if (e.Bleed > 0) { amount += 22; e.Bleed = 0; e.ReactionFlash = .5f; }
            }
            if (bleed) { e.Bleed = 4.5f; e.BleedTick = 0; }
            e.Knockback = (e.X >= Player.X ? 1 : -1) * (e.ReactionFlash > 0 ? 210 : 135);
            DamageEnemy(e, amount, fire ? "fire" : "normal", false);
        }
        private void DamageEnemy(EnemyState e, int amount, string kind, bool dot)
        {
            if (e.Dead) return;
            e.Health = Math.Max(0, e.Health - amount);
            if (!dot) e.HitFlash = .18f;
            Popups.Add(new DamagePopup { X = e.X, Y = e.Y - 86, Amount = amount, Kind = kind });
        }
        public void DamagePlayer(int amount)
        {
            var p = Player;
            if (Ended || p.Invulnerable > 0 || p.Health <= 0) return;
            p.Health = Math.Max(0, p.Health - Math.Max(1, amount - p.Gear.Defense));
            p.Invulnerable = .7f; p.AttackFlash = .28f; p.HurtSerial++; p.Combo = 0; p.ComboWindow = 0;
            if (p.Health == 0) { Ended = true; Won = false; }
        }
        private void StepEnemy(EnemyState e, float dt)
        {
            if (e.Dead) return;
            e.AttackTimer = Down(e.AttackTimer, dt); e.HitFlash = Down(e.HitFlash, dt);
            e.ReactionFlash = Down(e.ReactionFlash, dt); e.Knockback = Toward(e.Knockback, 650 * dt);
            if (e.Bleed > 0)
            {
                e.Bleed = Down(e.Bleed, dt); e.BleedTick += dt;
                if (e.BleedTick >= .75f) { e.BleedTick = 0; DamageEnemy(e, 3, "bleed", true); }
            }
            if (e.Burn > 0)
            {
                e.Burn = Down(e.Burn, dt); e.BurnTick += dt;
                if (e.BurnTick >= .6f) { e.BurnTick -= .6f; DamageEnemy(e, 2, "fire", true); }
            }
            if (e.Dead) return;
            float dx = Player.X - e.X, vx = 0;
            if (Math.Abs(dx) > 46) vx = Math.Sign(dx) * e.Speed;
            else if (e.AttackTimer <= 0 && Math.Abs(Player.Y - e.Y) < 70)
            {
                DamagePlayer(e.AttackDamage); e.AttackTimer = e.Kind == 0 ? 1.3f : 1.8f;
            }
            e.X = Math.Max(e.Kind == 0 ? 18 : 23, Math.Min(WorldWidth - (e.Kind == 0 ? 18 : 23), e.X + (vx + e.Knockback) * dt));
        }
        private void StepProjectiles(float dt)
        {
            for (int i = Projectiles.Count - 1; i >= 0; i--)
            {
                var b = Projectiles[i]; float oldX = b.X;
                b.X += b.Facing * (b.Empowered ? 440 : 540) * dt; b.Life -= dt;
                bool remove = b.Life <= 0;
                foreach (var e in Enemies)
                {
                    if (remove || e.Dead) continue;
                    // Swept test avoids tunnelling when the render frame rate drops.
                    float nearX = Math.Max(Math.Min(oldX, b.X), Math.Min(Math.Max(oldX, b.X), e.X));
                    float dx = nearX - e.X, dy = b.Y - (e.Y - 30), radius = b.Empowered ? 34 : 25;
                    if (dx * dx + dy * dy < radius * radius) { ReceiveHit(e, b.Damage, true); remove = true; }
                }
                if (remove) Projectiles.RemoveAt(i);
            }
        }
    }
}
