#!/usr/bin/env python3
import os
from pathlib import Path
import shutil
import subprocess
import sys

DFG_SIZE = 128
ENVIRONMENT_FACE_SIZES = (256, 512)
IMAGE_SUFFIXES = (".png", ".jpg", ".jpeg")


def is_outdated(outputs: list[Path], inputs: list[Path]) -> bool:
    if not all(output.is_file() for output in outputs):
        return True

    output_time = min(output.stat().st_mtime for output in outputs)

    return any(path.stat().st_mtime > output_time for path in inputs)


class GenerateEnvironment:
    def __init__(self, project_root: Path) -> None:
        self._project_root: Path = project_root
        self._environment_dir: Path = (
            project_root / "content" / "engine" / "assets" / "textures" / "environment"
        )
        self._environment_output_dir: Path = (
            project_root / "build" / "assets" / "textures" / "environment"
        )
        self._dfg_path: Path = (
            project_root / "build" / "assets" / "textures" / "ibl" / "dfg.png"
        )
        self._ibl_dir: Path = project_root / "src" / "engine" / "renderer" / "ibl"
        self._texture_dir: Path = (
            project_root / "src" / "engine" / "renderer" / "texture"
        )
        self._mathsfury_dir: Path = project_root / "thirdparty" / "mathsfury"
        self._generator_path: Path = project_root / "build" / "tools" / "iblgen"

    def _get_generator_sources(self) -> list[Path]:
        return [
            self._project_root / "tools" / "iblgen" / "EnvironmentGenerator.cpp",
            self._ibl_dir / "Cubemap.cpp",
            self._ibl_dir / "EnvironmentFaces.cpp",
            self._ibl_dir / "CubemapUtils.cpp",
            self._ibl_dir / "CubemapIBL.cpp",
            self._ibl_dir / "CubemapSH.cpp",
            self._ibl_dir / "EnvironmentPrefilter.cpp",
            self._ibl_dir / "EnvironmentSerializer.cpp",
            self._texture_dir / "ColorSpace.cpp",
            *sorted(self._mathsfury_dir.glob("*.cpp")),
        ]

    def _get_generator_inputs(self) -> list[Path]:
        return [
            *self._get_generator_sources(),
            *sorted(self._ibl_dir.glob("*.h")),
            self._texture_dir / "ColorSpace.h",
            *sorted(self._mathsfury_dir.glob("*.h")),
            self._project_root / "shaders" / "ShaderConstants.h",
        ]

    def _build_generator(self) -> bool:
        compiler = (
            os.environ.get("HOST_CXX") or shutil.which("c++") or shutil.which("g++")
        )
        if not compiler:
            print("[ERROR] No host C++ compiler found, set HOST_CXX")

            return False

        include_dirs = [
            ".",
            "public",
            "src",
            "src/engine",
            "thirdparty",
            "thirdparty/mathsfury",
            "thirdparty/simde",
            "thirdparty/gcmgl/public",
            "thirdparty/gcmgl/src",
        ]
        command = [
            compiler,
            "-std=gnu++98",
            "-O2",
            *(f"-I{self._project_root / include_dir}" for include_dir in include_dirs),
            *(str(source) for source in self._get_generator_sources()),
            "-o",
            str(self._generator_path),
            "-lm",
        ]

        self._generator_path.parent.mkdir(parents=True, exist_ok=True)
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode != 0:
            print("[ERROR] Failed to build the environment generator")
            print(result.stdout + result.stderr)

            return False

        return True

    def _run_generator(self, arguments: list[str], output: Path) -> bool:
        result = subprocess.run(
            [str(self._generator_path), *arguments],
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            output.unlink(missing_ok=True)
            print(f"[ERROR] Failed to generate {output.name}")
            print(result.stdout + result.stderr)

            return False

        return True

    def _generate_dfg(self) -> bool:
        if not is_outdated([self._dfg_path], [self._generator_path]):
            return True

        print(f"Generating {self._dfg_path.name}...")
        self._dfg_path.parent.mkdir(parents=True, exist_ok=True)

        return self._run_generator(
            ["dfg", str(DFG_SIZE), str(self._dfg_path)],
            self._dfg_path,
        )

    def _generate_environments(self) -> bool:
        if not self._environment_dir.is_dir():
            return True

        images = sorted(
            path
            for path in self._environment_dir.iterdir()
            if path.suffix.lower() in IMAGE_SUFFIXES
        )

        self._environment_output_dir.mkdir(parents=True, exist_ok=True)
        for image in images:
            for face_size in ENVIRONMENT_FACE_SIZES:
                output = (
                    self._environment_output_dir
                    / f"{image.stem}.{face_size}.ibl"
                )
                if not is_outdated([output], [image, self._generator_path]):
                    continue

                print(f"Generating {image.name} ({face_size})...")
                arguments = [
                    "environment",
                    str(image),
                    str(face_size),
                    str(output),
                ]
                if not self._run_generator(arguments, output):
                    return False

        return True

    def run(self) -> bool:
        if is_outdated([self._generator_path], self._get_generator_inputs()):
            print("Building the environment generator...")
            if not self._build_generator():
                return False

        if not self._generate_dfg() or not self._generate_environments():
            return False

        print("Environment generation completed")

        return True


def main() -> int:
    project_root = Path(__file__).resolve().parent.parent
    generate_environment = GenerateEnvironment(project_root)

    return 0 if generate_environment.run() else 1


if __name__ == "__main__":
    sys.exit(main())