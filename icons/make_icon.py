#!/usr/bin/env python3
"""Generate an alternative app icon as a 1024x1024 PNG preview.

Design: rounded-square tile with the app's grass-green checker pattern,
a matte-black mine with radial spikes and a small highlight centered, and
a red flag planted in the lower-right corner.
"""

from __future__ import annotations

import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter

SIZE = 1024
OUTPUT = Path(__file__).parent / "preview_alt_icon.png"


def build_icon() -> Image.Image:
    img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    radius = int(SIZE * 0.22)
    mask = Image.new("L", (SIZE, SIZE), 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, SIZE, SIZE), radius=radius, fill=255)

    # 2x2 checker in the app's two green shades.
    cell = SIZE // 2
    green_a = (162, 209, 73, 255)
    green_b = (140, 188, 54, 255)
    for row in range(2):
        for col in range(2):
            colour = green_a if (row + col) % 2 == 0 else green_b
            draw.rectangle(
                (col * cell, row * cell, (col + 1) * cell, (row + 1) * cell),
                fill=colour,
            )

    # Mine body.
    cx, cy = int(SIZE * 0.46), int(SIZE * 0.54)
    mine_r = int(SIZE * 0.24)

    # Radial spikes (8-pointed).
    spike_len = int(SIZE * 0.11)
    spike_w = int(SIZE * 0.04)
    for angle_deg in range(0, 360, 45):
        angle = math.radians(angle_deg)
        x1 = cx + math.cos(angle) * (mine_r - spike_w * 0.3)
        y1 = cy + math.sin(angle) * (mine_r - spike_w * 0.3)
        x2 = cx + math.cos(angle) * (mine_r + spike_len)
        y2 = cy + math.sin(angle) * (mine_r + spike_len)
        draw.line((x1, y1, x2, y2), fill=(18, 18, 18, 255), width=spike_w)

    # Soft drop-shadow under the mine.
    shadow_layer = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    sd = ImageDraw.Draw(shadow_layer)
    sd.ellipse(
        (cx - mine_r, cy - mine_r + int(SIZE * 0.02),
         cx + mine_r, cy + mine_r + int(SIZE * 0.02)),
        fill=(0, 0, 0, 130),
    )
    shadow_layer = shadow_layer.filter(ImageFilter.GaussianBlur(radius=SIZE // 50))
    img = Image.alpha_composite(img, shadow_layer)

    draw = ImageDraw.Draw(img)
    draw.ellipse(
        (cx - mine_r, cy - mine_r, cx + mine_r, cy + mine_r),
        fill=(24, 24, 24, 255),
    )
    # Mine highlight (upper-left).
    h_r = int(mine_r * 0.32)
    h_off = int(mine_r * 0.42)
    draw.ellipse(
        (cx - h_off - h_r, cy - h_off - h_r,
         cx - h_off + h_r, cy - h_off + h_r),
        fill=(235, 235, 235, 255),
    )

    # Red flag planted upper-right.
    pole_x = int(SIZE * 0.74)
    pole_top = int(SIZE * 0.22)
    pole_bot = int(SIZE * 0.78)
    pole_w = int(SIZE * 0.022)
    # Flag base mound.
    draw.polygon(
        [
            (pole_x - int(SIZE * 0.06), pole_bot),
            (pole_x + int(SIZE * 0.06), pole_bot),
            (pole_x, pole_bot - int(SIZE * 0.035)),
        ],
        fill=(30, 30, 30, 255),
    )
    # Pole.
    draw.rectangle(
        (pole_x - pole_w // 2, pole_top, pole_x + pole_w // 2, pole_bot),
        fill=(30, 30, 30, 255),
    )
    # Flag cloth (triangle flowing left).
    flag_tip_x = pole_x - int(SIZE * 0.20)
    flag_tip_y = pole_top + int(SIZE * 0.08)
    draw.polygon(
        [
            (pole_x, pole_top),
            (flag_tip_x, flag_tip_y),
            (pole_x, pole_top + int(SIZE * 0.16)),
        ],
        fill=(226, 48, 39, 255),
    )
    # Subtle flag highlight.
    draw.polygon(
        [
            (pole_x, pole_top),
            (flag_tip_x + int(SIZE * 0.05), flag_tip_y - int(SIZE * 0.005)),
            (pole_x, pole_top + int(SIZE * 0.03)),
        ],
        fill=(250, 90, 80, 255),
    )

    out = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    out.paste(img, (0, 0), mask=mask)
    return out


if __name__ == "__main__":
    icon = build_icon()
    icon.save(OUTPUT)
    print(f"wrote {OUTPUT}")
