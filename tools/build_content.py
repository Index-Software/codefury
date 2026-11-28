#!/usr/bin/env python3
import argparse
from pathlib import Path
import shutil
import subprocess
import sys

from generate_font_atlas import find_generator, generate
from generate_ibl import GenerateEnvironment, is_outdated

PLATFORM_SHADER_DIRS = {"linux": "glsl", "ps3": "cg"}
PLATFORM_OUTPUT_DIRS = {"linux": "bin/linux-x86_64", "ps3": "bin/ps3/USRDIR"}
SHADER_SOURCE_SUFFIXES = (".slang", ".slangh", ".cginc", ".vcg", ".fcg", ".h", ".py")


class BuildContent:
    def __init__(
        self,
        project_root: Path,
        platform: str,
        output_dir: Path,
        extra_shader_dirs: list[Path],
    ) -> None:
        self._project_root: Path = project_root
        self._platform: str = platform
        self._output_dir: Path = output_dir
        self._extra_shader_dirs: list[Path] = extra_shader_dirs
        self._build_dir: Path = project_root / "build"

    def _compile_shaders(self) -> bool:
        shaders_dir = self._project_root / "shaders"
        stamp = self._build_dir / "shaders" / "shaders.stamp"
        sources = [
            path
            for directory in [shaders_dir, *self._extra_shader_dirs]
            for path in directory.rglob("*")
            if path.suffix in SHADER_SOURCE_SUFFIXES
        ]

        extra_dirs = "\n".join(str(directory) for directory in self._extra_shader_dirs)
        is_same_dirs = stamp.is_file() and stamp.read_text() == extra_dirs
        if is_same_dirs and not is_outdated([stamp], sources):
            return True

        if stamp.is_file() and not is_same_dirs:
            shutil.rmtree(self._build_dir / "shaders" / "cg", ignore_errors=True)
            shutil.rmtree(self._build_dir / "shaders" / "glsl", ignore_errors=True)

        print("Compiling shaders...", flush=True)
        command = [sys.executable, str(shaders_dir / "compile_shaders.py")]
        for directory in self._extra_shader_dirs:
            command += ["--shaders", str(directory)]

        result = subprocess.run(command, cwd=self._project_root)
        if result.returncode != 0:
            print("[ERROR] Shader compilation failed")

            return False

        stamp.write_text(extra_dirs)

        return True

    def _generate_fonts(self) -> bool:
        if find_generator() is None:
            return True

        tools_dir = self._project_root / "tools"
        fonts_dir = self._project_root / "content" / "engine" / "assets" / "fonts"
        outputs = [fonts_dir / "font_atlas.png", fonts_dir / "font_atlas.metrics"]
        inputs = [
            fonts_dir / "Roboto-Regular.ttf",
            tools_dir / "generate_font_atlas.py",
            tools_dir / "convert_font_atlas.py",
        ]

        if not is_outdated(outputs, inputs):
            return True

        print("Generating the font atlas...")

        return generate() == 0

    def _bake_environment(self) -> bool:
        return GenerateEnvironment(self._project_root).run()

    def _stage(self) -> None:
        print("Staging content...")
        shader_dir = PLATFORM_SHADER_DIRS[self._platform]
        content_dir = self._project_root / "content"
        copies = [
            (self._build_dir / "shaders" / shader_dir, Path("shaders") / shader_dir),
            (content_dir / "engine" / "assets", Path("assets")),
            (self._build_dir / "assets", Path("assets")),
            (content_dir / "engine" / "scripts", Path("scripts") / "engine"),
            (content_dir / "scripts", Path("scripts")),
        ]

        for src, dst in copies:
            if src.is_dir():
                shutil.copytree(src, self._output_dir / dst, dirs_exist_ok=True)

    def run(self) -> bool:
        if not self._compile_shaders():
            return False

        if not self._generate_fonts():
            return False

        if not self._bake_environment():
            return False

        self._stage()
        print("Content build completed")

        return True


def main() -> int:
    project_root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--platform",
        choices=sorted(PLATFORM_SHADER_DIRS),
        required=True,
    )
    parser.add_argument("--output", type=Path)
    parser.add_argument("--shaders", type=Path, action="append", default=[])
    args = parser.parse_args()

    default_output_dir = project_root / "build" / PLATFORM_OUTPUT_DIRS[args.platform]
    output_dir = args.output or default_output_dir
    build_content = BuildContent(
        project_root,
        args.platform,
        output_dir.resolve(),
        [directory.resolve() for directory in args.shaders],
    )

    return 0 if build_content.run() else 1


if __name__ == "__main__":
    sys.exit(main())
