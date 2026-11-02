using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

namespace Eclipse
{
    // Composition root: the scene needs one EclipseGame component, no Inspector wiring.
    public sealed class EclipseGame : MonoBehaviour
    {
        public CombatSimulation Simulation { get; private set; }
        private RectTransform viewport;
        private PixelWorldView world;
        private PixelHudView hud;
        private RawImage knight;
        private GameObject controls, endPanel, inventory;
        private Text endTitle, inventoryText;
        private Font uiFont;
        private readonly List<TouchControl> touchControls = new List<TouchControl>();
        private readonly bool[] held = new bool[5]; // jump, dash, attack, skill, swap
        private CombatInput pending;
        private float stickAxis, accumulator, cameraX = 640, animationClock, attackTime, hurtTime;
        private int attackSerial, hurtSerial, attackCombo = 1, animation = -1;
        private Rect previousSafeArea;
        private Vector2Int previousSize;
        private bool lastEnded;
        private static readonly int[] FrameCounts = { 6, 8, 4, 3, 6, 6, 8, 5, 3, 6 };
        private static readonly int[] FrameStarts = { 0, 6, 14, 18, 21, 27, 33, 41, 46, 49 };
        private void Awake()
        {
            Application.targetFrameRate = 60;
            Simulation = new CombatSimulation();
            BuildPresentation();
            RefreshInventory(); FitViewport();
        }
        private void BuildPresentation()
        {
            var cameraObject = new GameObject("Letterbox Camera", typeof(Camera)); cameraObject.transform.SetParent(transform, false);
            var camera = cameraObject.GetComponent<Camera>(); camera.clearFlags = CameraClearFlags.SolidColor; camera.backgroundColor = Color.black; camera.cullingMask = 0;
            var canvasObject = new GameObject("ECLIPSE Canvas", typeof(RectTransform), typeof(Canvas), typeof(GraphicRaycaster));
            canvasObject.transform.SetParent(transform, false); canvasObject.GetComponent<Canvas>().renderMode = RenderMode.ScreenSpaceOverlay;
            var eventObject = new GameObject("ECLIPSE EventSystem", typeof(EventSystem), typeof(StandaloneInputModule)); eventObject.transform.SetParent(transform, false);
            viewport = MakeRect("1280 x 720 Viewport", canvasObject.transform, 0, 0, 1280, 720);
            viewport.anchorMin = viewport.anchorMax = viewport.pivot = new Vector2(.5f, .5f);
            var worldRect = MakeRect("Pixel Forest and Combat", viewport, 0, 0, 1280, 720);
            world = worldRect.gameObject.AddComponent<PixelWorldView>(); world.raycastTarget = false; world.Simulation = Simulation;
            knight = MakeImage("Original Knight Atlas", viewport, 0, 0, 96, 128, LoadTexture("knight_sheet_48x64_v2")); knight.raycastTarget = false;
            var hudRect = MakeRect("Pixel HUD", viewport, 0, 0, 1280, 125);
            hud = hudRect.gameObject.AddComponent<PixelHudView>(); hud.raycastTarget = false; hud.Simulation = Simulation;
            controls = MakeRect("Touch Controls", viewport, 0, 0, 1280, 720).gameObject;
            var stickImage = MakeImage("Move and Jump", controls.transform, 35, 518, 160, 160, LoadTexture("ui/joystick_arrows_new"));
            var stick = stickImage.gameObject.AddComponent<TouchControl>(); stick.IsStick = true; stick.Artwork = stickImage;
            stick.StickChanged = value => { stickAxis = Mathf.Abs(value.x) < .23f ? 0 : Mathf.Sign(value.x); SetHeld(0, value.y > .52f); };
            touchControls.Add(stick);
            ActionButton("Attack", 1110, 540, 116, "ui/attack_states", new Rect(0, 0, .5f, 1), new Rect(.5f, 0, .5f, 1), 2);
            ActionButton("Dash", 985, 560, 100, "ui/dash_states", new Rect(0, 0, .5f, 1), new Rect(.5f, 0, .5f, 1), 1);
            ActionButton("Swap", 995, 445, 90, "ui/swap_skill_states", new Rect(0, .5f, .5f, .5f), new Rect(.5f, .5f, .5f, .5f), 4);
            ActionButton("Skill", 1115, 420, 94, "ui/swap_skill_states", new Rect(0, 0, .5f, .5f), new Rect(.5f, 0, .5f, .5f), 3);
            // Optional inventory uses system Korean fonts, as the source has no licensed pixel font.
            uiFont = Font.CreateDynamicFontFromOSFont(new[] { "Noto Sans CJK KR", "Malgun Gothic", "Apple SD Gothic Neo", "sans-serif" }, 20);
            if (uiFont == null) uiFont = Resources.GetBuiltinResource<Font>("LegacyRuntime.ttf");
            inventory = MakeRect("Equipment Inventory", viewport, 46, 135, 460, 340).gameObject;
            Panel(inventory, PixelGraphic.Hex("1c2a34"));
            inventoryText = Label("Equipment", inventory.transform, 20, 15, 425, 245, 19, TextAnchor.UpperLeft);
            for (int i = 0; i < 3; i++)
            {
                int slot = i;
                ClickButton(new[] { "무기", "방어구", "장신구" }[i], inventory.transform, 20 + i * 143, 278, 132, 45,
                    () => CycleEquipment(slot));
            }
            inventory.SetActive(false);
            // Invisible art-free hot area keeps the original top-right slot layout unchanged.
            var inventoryHit = MakeRect("Inventory Slots Hit Area", viewport, 765, 21, 288, 61);
            var hit = inventoryHit.gameObject.AddComponent<Image>(); hit.color = Color.clear;
            inventoryHit.gameObject.AddComponent<Button>().onClick.AddListener(ToggleInventory);
            endPanel = MakeRect("Round Result", viewport, 0, 0, 1280, 720).gameObject;
            Panel(endPanel, new Color(.067f, .102f, .157f, .94f));
            endTitle = Label("Result", endPanel.transform, 250, 250, 780, 80, 54, TextAnchor.MiddleCenter);
            var hint = Label("Retry Hint", endPanel.transform, 250, 340, 780, 50, 23, TextAnchor.MiddleCenter);
            hint.text = "R 키 또는 아래 버튼으로 다시 시작";
            ClickButton("다시 플레이", endPanel.transform, 510, 412, 260, 65, Restart);
            endPanel.SetActive(false);
        }
        private Texture2D LoadTexture(string path)
        {
            var texture = Resources.Load<Texture2D>("Eclipse/" + path);
            if (texture == null) throw new InvalidOperationException("ECLIPSE texture missing: " + path);
            texture.filterMode = FilterMode.Point; texture.wrapMode = TextureWrapMode.Clamp; return texture;
        }
        private void ActionButton(string name, float x, float y, float size, string resource, Rect normal, Rect pressed, int action)
        {
            var image = MakeImage(name, controls.transform, x, y, size, size, LoadTexture(resource)); image.uvRect = normal;
            var control = image.gameObject.AddComponent<TouchControl>(); control.Artwork = image; control.RestUv = normal; control.ActiveUv = pressed;
            control.PressChanged = value => SetHeld(action, value); touchControls.Add(control);
        }
        private static RectTransform MakeRect(string name, Transform parent, float x, float y, float w, float h)
        {
            var rect = new GameObject(name, typeof(RectTransform)).GetComponent<RectTransform>(); rect.SetParent(parent, false);
            rect.anchorMin = rect.anchorMax = rect.pivot = new Vector2(0, 1); rect.sizeDelta = new Vector2(w, h); rect.anchoredPosition = new Vector2(x, -y); return rect;
        }
        private static RawImage MakeImage(string name, Transform parent, float x, float y, float w, float h, Texture texture)
        {
            var rect = MakeRect(name, parent, x, y, w, h); var image = rect.gameObject.AddComponent<RawImage>(); image.texture = texture; return image;
        }
        private static void Panel(GameObject owner, Color tint) { owner.AddComponent<Image>().color = tint; }
        private Text Label(string name, Transform parent, float x, float y, float w, float h, int size, TextAnchor alignment)
        {
            var text = MakeRect(name, parent, x, y, w, h).gameObject.AddComponent<Text>(); text.font = uiFont; text.fontSize = size; text.alignment = alignment;
            text.color = PixelGraphic.Hex("d4e0d2"); text.raycastTarget = false; return text;
        }
        private void ClickButton(string caption, Transform parent, float x, float y, float w, float h, UnityEngine.Events.UnityAction action)
        {
            var rect = MakeRect(caption, parent, x, y, w, h); var image = rect.gameObject.AddComponent<Image>(); image.color = PixelGraphic.Hex("304039");
            var button = rect.gameObject.AddComponent<Button>(); button.targetGraphic = image; button.onClick.AddListener(action);
            var label = Label(caption, rect, 4, 4, w - 8, h - 8, 22, TextAnchor.MiddleCenter); label.text = caption;
        }
        private void SetHeld(int action, bool value)
        {
            if (value && !held[action] && !Simulation.Ended)
            {
                switch (action) { case 0: pending.Jump = true; break; case 1: pending.Dash = true; break; case 2: pending.Attack = true; break; case 3: pending.Skill = true; break; case 4: pending.Swap = true; break; }
            }
            held[action] = value;
        }
        private void Update()
        {
            FitViewport();
            if (Input.GetKeyDown(KeyCode.R)) Restart();
            if (Input.GetKeyDown(KeyCode.I)) ToggleInventory();
            for (int i = 0; i < 3; i++) if (Input.GetKeyDown((KeyCode)((int)KeyCode.Alpha1 + i))) CycleEquipment(i);
            if (!Simulation.Ended)
            {
                pending.Jump |= Input.GetKeyDown(KeyCode.Space) || Input.GetKeyDown(KeyCode.W) || Input.GetKeyDown(KeyCode.UpArrow);
                pending.Dash |= Input.GetKeyDown(KeyCode.LeftShift) || Input.GetKeyDown(KeyCode.RightShift);
                pending.Attack |= Input.GetKeyDown(KeyCode.J) || Input.GetKeyDown(KeyCode.Z);
                pending.Skill |= Input.GetKeyDown(KeyCode.K) || Input.GetKeyDown(KeyCode.X);
                pending.Swap |= Input.GetKeyDown(KeyCode.L) || Input.GetKeyDown(KeyCode.C) || Input.GetKeyDown(KeyCode.Tab);
            }
            float axis = (Input.GetKey(KeyCode.D) || Input.GetKey(KeyCode.RightArrow) ? 1 : 0) - (Input.GetKey(KeyCode.A) || Input.GetKey(KeyCode.LeftArrow) ? 1 : 0);
            pending.Axis = Mathf.Abs(axis) > .1f ? axis : stickAxis;
            accumulator += Mathf.Min(Time.unscaledDeltaTime, .1f);
            while (accumulator >= CombatSimulation.FixedStep)
            {
                Simulation.Step(CombatSimulation.FixedStep, pending);
                pending = new CombatInput { Axis = pending.Axis }; accumulator -= CombatSimulation.FixedStep;
            }
            cameraX = Mathf.Lerp(cameraX, Mathf.Clamp(Simulation.Player.X, 640, 960), 1 - Mathf.Exp(-5 * Time.unscaledDeltaTime));
            world.CameraX = Mathf.Round(cameraX); world.SetVerticesDirty(); hud.SetVerticesDirty();
            AnimateKnight(Time.unscaledDeltaTime);
            if (Simulation.Ended != lastEnded)
            {
                lastEnded = Simulation.Ended; endPanel.SetActive(lastEnded); controls.SetActive(!lastEnded);
                if (lastEnded) { inventory.SetActive(false); ClearInput(); endTitle.text = Simulation.Won ? "STAGE CLEAR!" : "GAME OVER"; endTitle.color = PixelGraphic.Hex(Simulation.Won ? "8be4b5" : "ff8d99"); }
            }
        }
        private void AnimateKnight(float dt)
        {
            var p = Simulation.Player;
            if (p.AttackSerial != attackSerial) { attackSerial = p.AttackSerial; attackTime = .36f; attackCombo = Mathf.Clamp(p.Combo, 1, 3); animation = -1; }
            if (p.HurtSerial != hurtSerial) { hurtSerial = p.HurtSerial; hurtTime = .23f; }
            attackTime = Mathf.Max(0, attackTime - dt); hurtTime = Mathf.Max(0, hurtTime - dt);
            int next = p.Health <= 0 ? 9 : hurtTime > 0 ? 8 : attackTime > 0 ? 3 + attackCombo : p.DashLeft > 0 ? 7 : !p.Grounded ? (p.VY < 0 ? 2 : 3) : Mathf.Abs(p.VX) > 20 ? 1 : 0;
            if (next != animation) { animation = next; animationClock = 0; } else animationClock += dt;
            bool loop = next == 0 || next == 1 || next == 3;
            int frame = loop ? (int)(animationClock * 12) % FrameCounts[next] : Mathf.Min((int)(animationClock * 12), FrameCounts[next] - 1);
            int index = FrameStarts[next] + frame;
            float u = index % 8 / 8f, v = 1 - (index / 8 + 1) / 7f;
            knight.uvRect = p.Facing > 0 ? new Rect(u, v, 1f / 8, 1f / 7) : new Rect(u + 1f / 8, v, -1f / 8, 1f / 7);
            knight.enabled = p.Form == 0; knight.rectTransform.anchoredPosition = new Vector2(Mathf.Round(p.X - cameraX + 640 - 48), -Mathf.Round(p.Y - 128));
        }
        private void FitViewport()
        {
            Rect safe = Screen.safeArea; var size = new Vector2Int(Screen.width, Screen.height);
            if (safe == previousSafeArea && size == previousSize) return;
            previousSafeArea = safe; previousSize = size;
            float scale = Mathf.Min(safe.width / 1280, safe.height / 720);
            viewport.localScale = new Vector3(scale, scale, 1);
            viewport.anchoredPosition = safe.center - new Vector2(Screen.width * .5f, Screen.height * .5f);
        }
        private void CycleEquipment(int slot) { if (Simulation.Ended) return; Simulation.Player.CycleEquipment(slot); RefreshInventory(); }
        private void RefreshInventory()
        {
            var p = Simulation.Player;
            inventoryText.text = $"장비 현황\n\n무기: {p.Gear.Name(0)}\n방어구: {p.Gear.Name(1)}\n장신구: {p.Gear.Name(2)}\n\n공격력 {p.Gear.Attack:+0;-0;0} | 방어력 {p.Gear.Defense}\n최대 체력 {p.MaxHealth} | 속도 {p.Gear.Speed:0.00}x\n1 / 2 / 3 키 또는 아래 버튼으로 교체";
        }
        private void ToggleInventory() { if (!Simulation.Ended) inventory.SetActive(!inventory.activeSelf); }
        public void Restart()
        {
            ClearInput(); Simulation.Restart(); cameraX = 640; accumulator = 0; animation = -1; attackTime = hurtTime = 0; attackSerial = hurtSerial = 0;
            lastEnded = false; endPanel.SetActive(false); controls.SetActive(true); inventory.SetActive(false); RefreshInventory();
        }
        private void ClearInput() { foreach (var control in touchControls) control.Release(); Array.Clear(held, 0, held.Length); stickAxis = 0; pending = default; accumulator = 0; }
        private void OnApplicationFocus(bool focus) { if (!focus && Simulation != null) ClearInput(); }
        private void OnApplicationPause(bool pause) { if (pause && Simulation != null) ClearInput(); }
        private void OnDisable() { if (Simulation != null) ClearInput(); }
    }
}
