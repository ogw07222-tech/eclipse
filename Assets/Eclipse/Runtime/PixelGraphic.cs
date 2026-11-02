using UnityEngine;
using UnityEngine.UI;

namespace Eclipse
{
    public abstract class PixelGraphic : MaskableGraphic
    {
        protected VertexHelper Mesh;
        public static Color Hex(string value) { ColorUtility.TryParseHtmlString("#" + value, out var c); return c; }
        protected static readonly Color Ink = Hex("101923"), Panel = Hex("1c2a34"), Light = Hex("d4e0d2"), Gold = Hex("e3bd72");
        private static readonly string[] Digits = {
            "111101101101111", "010110010010111", "111001111100111", "111001111001111", "101101111001001",
            "111100111001111", "111100111101111", "111001010010010", "111101111101111", "111101111001111" };
        protected override void OnPopulateMesh(VertexHelper vh)
        {
            Mesh = vh; vh.Clear(); DrawPixels(); Mesh = null;
        }
        protected abstract void DrawPixels();
        protected void Block(float x, float y, float w, float h, Color color, float grid = 1)
        {
            if (w <= 0 || h <= 0 || color.a <= 0) return;
            x = Mathf.Round(x / grid) * grid; y = Mathf.Round(y / grid) * grid;
            w = Mathf.Max(grid, Mathf.Round(w / grid) * grid); h = Mathf.Max(grid, Mathf.Round(h / grid) * grid);
            if (x + w < 0 || x > rectTransform.rect.width || y + h < 0 || y > rectTransform.rect.height) return;
            float left = rectTransform.rect.xMin + x, top = rectTransform.rect.yMax - y;
            int i = Mesh.currentVertCount;
            Mesh.AddVert(new Vector3(left, top), color, Vector2.zero);
            Mesh.AddVert(new Vector3(left + w, top), color, Vector2.zero);
            Mesh.AddVert(new Vector3(left + w, top - h), color, Vector2.zero);
            Mesh.AddVert(new Vector3(left, top - h), color, Vector2.zero);
            Mesh.AddTriangle(i, i + 1, i + 2); Mesh.AddTriangle(i + 2, i + 3, i);
        }
        protected void Disk(float x, float y, float r, Color color, int cell = 4)
        {
            for (int yy = -(int)r; yy <= r; yy += cell)
                for (int xx = -(int)r; xx <= r; xx += cell)
                    if (xx * xx + yy * yy <= r * r) Block(x + xx, y + yy, cell, cell, color);
        }
        protected void Ring(float x, float y, float r, Color color, int cell = 4)
        {
            for (int yy = -(int)r; yy <= r; yy += cell)
                for (int xx = -(int)r; xx <= r; xx += cell)
                {
                    int d = xx * xx + yy * yy;
                    if (d <= r * r && d >= (r - cell * 2) * (r - cell * 2)) Block(x + xx, y + yy, cell, cell, color);
                }
        }
        protected void Number(int value, float x, float y, Color tint, int scale = 3)
        {
            // Avoid per-frame numeric string allocation in the mesh builder.
            int divisor = 1; while (value / divisor >= 10 && divisor <= 10000000) divisor *= 10;
            do
            {
                int digit = value / divisor % 10; string glyph = Digits[digit];
                for (int j = 0; j < 15; j++) if (glyph[j] == '1')
                {
                    float px = x + j % 3 * scale, py = y + j / 3 * scale;
                    Block(px + 2, py + 2, scale, scale, Ink); Block(px, py, scale, scale, tint);
                }
                x += 4 * scale; divisor /= 10;
            } while (divisor > 0);
        }
    }
}
