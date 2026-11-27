"""Pack the original cinematic plates and transparent character cels."""
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw

art = Path(__file__).resolve().parent
output = art.parents[1] / "Assets/Resources/Eclipse/title"
for shot in ("altar", "gauntlet"):
    with Image.open(art / f"eclipse_{shot}.png") as plate:
        plate.convert("RGB").resize((1280, 720), Image.Resampling.LANCZOS).save(
            output / f"opening_{shot}.png", optimize=True
        )

source = Image.open(art / "eclipse_character_cels.png").convert("RGBA")
# The tiny nonzero values away from the cels are export noise. Keep the authored
# alpha, including the antialiased contour, across all three animation layers.
source.putalpha(source.getchannel("A").point(lambda alpha: 0 if alpha < 5 else alpha))
atlas = Image.new("RGBA", (1536, 1408))


def pack_cel(box, destination, waist_mask=None):
    cel = source.crop(box)
    if waist_mask is not None:
        opacity = cel.getchannel("A")
        cel.putalpha(ImageChops.multiply(opacity, waist_mask.crop(box)))
    atlas.paste(cel, destination)


# Masks overlap beneath the belt and tassets. The exterior silhouette and
# transparent gaps between the hand, armor, and sword come from the cel itself.
legs = Image.new("L", source.size)
ImageDraw.Draw(legs).polygon(
    [(149, 522), (185, 464), (259, 454), (312, 475), (353, 508),
     (376, 555), (468, 836), (468, 958), (81, 958), (81, 572)],
    fill=255,
)
upper = Image.new("L", source.size, 255)
ImageDraw.Draw(upper).polygon(
    [(151, 561), (191, 499), (262, 491), (321, 511), (351, 548),
     (368, 582), (410, 723), (442, 956), (81, 958), (81, 568)],
    fill=0,
)

pack_cel((28, 8, 614, 854), (0, 0), upper)
pack_cel((638, 124, 1508, 996), (620, 0))
pack_cel((80, 450, 478, 954), (0, 884), legs)
atlas.save(output / "opening_knight.png", optimize=True)
