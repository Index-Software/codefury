#!/usr/bin/env python3
import json
from pathlib import Path
import sys


def format_float(value: float) -> str:
    return f"{value:.9g}"


def convert(json_path: Path, out_path: Path) -> int:
    data = json.loads(json_path.read_text())

    atlas = data["atlas"]
    metrics = data["metrics"]

    atlas_width = int(atlas["width"])
    atlas_height = int(atlas["height"])
    em_size = float(atlas["size"])

    distance_range = float(atlas["distanceRange"])

    ascent = -float(metrics["ascender"]) * em_size
    descent = float(metrics["descender"]) * em_size
    line_height = float(metrics["lineHeight"]) * em_size

    glyphs = []
    for glyph in data["glyphs"]:
        codepoint = int(glyph["unicode"])
        advance = float(glyph["advance"]) * em_size

        bounds = glyph.get("atlasBounds")
        plane = glyph.get("planeBounds")
        if not bounds or not plane:
            glyphs.append((codepoint, advance, 0.0, 0.0, 0, 0, 0, 0))

            continue

        left = int(bounds["left"])
        top = int(bounds["top"])
        width = int(bounds["right"]) - left
        height = int(bounds["bottom"]) - top

        offset_x = float(plane["left"]) * em_size

        offset_y = height + float(plane["top"]) * em_size

        glyphs.append(
            (codepoint, advance, offset_x, offset_y, left, top, width, height),
        )

    glyphs.sort(key=lambda glyph: glyph[0])

    lines = [
        f"font em={format_float(em_size)} ascent={format_float(ascent)} "
        f"descent={format_float(descent)} "
        f"lineHeight={format_float(line_height)} "
        f"distanceRange={format_float(distance_range)} "
        f"atlasWidth={atlas_width} atlasHeight={atlas_height}",
    ]
    for codepoint, advance, offset_x, offset_y, left, top, width, height in glyphs:
        lines.append(
            f"glyph codepoint={codepoint} advance={format_float(advance)} "
            f"offsetX={format_float(offset_x)} offsetY={format_float(offset_y)} "
            f"atlasX={left} atlasY={top} width={width} height={height}",
        )

    out_path.write_text("\n".join(lines) + "\n")

    print(f"Wrote {out_path} ({len(glyphs)} glyphs, em {em_size}px)")

    return 0


def main() -> int:
    if len(sys.argv) != 3:
        print("Usage: convert_font_atlas.py font_atlas.json font_atlas.metrics")

        return 2

    return convert(Path(sys.argv[1]), Path(sys.argv[2]))


if __name__ == "__main__":
    sys.exit(main())