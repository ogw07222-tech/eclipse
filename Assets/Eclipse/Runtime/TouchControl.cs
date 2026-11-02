using System;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

namespace Eclipse
{
    public sealed class TouchControl : MonoBehaviour, IPointerDownHandler, IPointerUpHandler, IDragHandler
    {
        public Action<bool> PressChanged;
        public Action<Vector2> StickChanged;
        public bool IsStick;
        public RawImage Artwork;
        public Rect RestUv = new Rect(0, 0, 1, 1), ActiveUv = new Rect(0, 0, 1, 1);
        private int? pointer;
        public void OnPointerDown(PointerEventData e)
        {
            if (pointer.HasValue) return;
            pointer = e.pointerId;
            if (Artwork != null) Artwork.uvRect = ActiveUv;
            PressChanged?.Invoke(true);
            if (IsStick) Move(e);
        }
        public void OnDrag(PointerEventData e) { if (IsStick && pointer == e.pointerId) Move(e); }
        public void OnPointerUp(PointerEventData e) { if (pointer == e.pointerId) Release(); }
        private void Move(PointerEventData e)
        {
            var rect = (RectTransform)transform;
            if (!RectTransformUtility.ScreenPointToLocalPointInRectangle(rect, e.position, e.pressEventCamera, out var point)) return;
            // Rect pivot is top-left, so local centre is (width/2, -height/2).
            Vector2 centre = rect.rect.center;
            var delta = Vector2.ClampMagnitude((point - centre) / 63, 1);
            StickChanged?.Invoke(delta);
        }
        public void Release()
        {
            pointer = null;
            if (Artwork != null) Artwork.uvRect = RestUv;
            PressChanged?.Invoke(false); if (IsStick) StickChanged?.Invoke(Vector2.zero);
        }
        private void OnDisable() { Release(); }
    }
}
