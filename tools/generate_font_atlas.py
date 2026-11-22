#!/usr/bin/env python3
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

from convert_font_atlas import convert

PROJECT_ROOT = Path(__file__).resolve().parents[1]
FONTS_DIR = PROJECT_ROOT / "content" / "engine" / "assets" / "fonts"

FONT_NAME = "Roboto-Regular.ttf"
ATLAS_NAME = "font_atlas"

ATLAS_SIZE = 48
ATLAS_DIMENSIONS = (512, 512)
DISTANCE_RANGE = 12
OUTER_PADDING = 4


def find_generator() -> str | None:
    from_env = os.environ.get("MSDF_ATLAS_GEN")
    if from_env:
        return from_env

    tools_generator = PROJECT_ROOT / "tools" / "msdf-atlas-gen" / "msdf-atlas-gen"
    if tools_generator.is_file():
        return str(tools_generator)

    return shutil.which("msdf-atlas-gen")


def generate() -> int:
    generator = find_generator()
    if not generator:
        print(
            "[ERROR] msdf-atlas-gen not found, run tools/build_msdf_atlas_gen.sh "
            "or set MSDF_ATLAS_GEN.",
        )

        return 1

    font_path = FONTS_DIR / FONT_NAME
    atlas_path = FONTS_DIR / f"{ATLAS_NAME}.png"
    metrics_path = FONTS_DIR / f"{ATLAS_NAME}.metrics"

    with tempfile.TemporaryDirectory() as temp_dir:
        json_path = Path(temp_dir) / f"{ATLAS_NAME}.json"

        command = [
            generator,
            "-font", str(font_path),
            "-type", "mtsdf",
            "-format", "png",
            "-dimensions", str(ATLAS_DIMENSIONS[0]), str(ATLAS_DIMENSIONS[1]),
            "-size", str(ATLAS_SIZE),
            "-pxrange", str(DISTANCE_RANGE),
            "-outerpxpadding", str(OUTER_PADDING),
            "-yorigin", "top",
            "-imageout", str(atlas_path),
            "-json", str(json_path),
        ]

        print(" ".join(command))
        result = subprocess.run(command)
        if result.returncode != 0:
            print("[ERROR] The atlas generator failed.")

            return result.returncode

        return convert(json_path, metrics_path)


if __name__ == "__main__":
    sys.exit(generate())