#!/usr/bin/env python3
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys


class CompileShaders:
    def __init__(self, project_root: str, extra_dirs: list[str]) -> None:
        self._project_root: Path = Path(project_root).resolve()
        self._extra_dirs: list[Path] = [
            Path(directory).resolve() for directory in extra_dirs
        ]
        self._shaders_dir: Path = self._project_root / "shaders"
        self._shaders_cg_dir: Path = self._project_root / "build" / "shaders" / "cg"
        self._shaders_glsl_dir: Path = self._project_root / "build" / "shaders" / "glsl"

    def compile_slang(self) -> bool:
        tools_slangc: Path = self._project_root / "tools" / "slang" / "slangc"

        slangc_path: str | None
        if tools_slangc.is_file():
            slangc_path = str(tools_slangc)
        else:
            slangc_path = shutil.which("slangc")

        if not slangc_path:
            print("[ERROR] slangc not found, cannot compile Slang shaders.")

            return False

        slang_env: dict[str, str] = os.environ.copy()
        if tools_slangc.is_file():
            tools_dir: str = str(tools_slangc.parent)
            existing: str = slang_env.get("LD_LIBRARY_PATH", "")

            if existing:
                slang_env["LD_LIBRARY_PATH"] = f"{tools_dir}:{existing}"
            else:
                slang_env["LD_LIBRARY_PATH"] = tools_dir

        slang_dir: Path = self._shaders_dir / "slang"
        if not slang_dir.exists():
            print(f"[ERROR] Slang shader directory not found: {slang_dir}")

            return False

        self._shaders_glsl_dir.mkdir(parents=True, exist_ok=True)

        slang_sources: list[tuple[Path, Path]] = [
            (slang_file, slang_dir) for slang_file in sorted(slang_dir.rglob("*.slang"))
        ]
        for extra_dir in self._extra_dirs:
            slang_sources += [
                (slang_file, extra_dir / "slang")
                for slang_file in sorted((extra_dir / "slang").rglob("*.slang"))
            ]

        for slang_file, source_dir in slang_sources:
            rel_path: Path = slang_file.relative_to(source_dir)
            base_rel: Path = rel_path.with_suffix("")
            name: str = slang_file.name

            print(f"[INFO] Compiling {rel_path}")

            vert_out: Path = self._shaders_glsl_dir / f"{base_rel}.vert"
            frag_out: Path = self._shaders_glsl_dir / f"{base_rel}.frag"

            vert_out.parent.mkdir(parents=True, exist_ok=True)
            frag_out.parent.mkdir(parents=True, exist_ok=True)

            for entry, stage, out_path in [
                ("vertMain", "vertex", vert_out),
                ("fragMain", "fragment", frag_out),
            ]:
                cmd: list[str] = [
                    slangc_path,
                    str(slang_file),
                    "-entry",
                    entry,
                    "-stage",
                    stage,
                    "-target",
                    "glsl",
                    "-I",
                    str(slang_dir),
                    "-matrix-layout-column-major",
                    "-o",
                    str(out_path),
                ]

                try:
                    result: subprocess.CompletedProcess[str] = subprocess.run(
                        cmd,
                        cwd=self._project_root,
                        capture_output=True,
                        text=True,
                        env=slang_env,
                    )

                    if result.returncode == 0:
                        glsl: str = out_path.read_text()
                        glsl = glsl.replace("layout(row_major) uniform;\n", "")
                        glsl = glsl.replace("layout(row_major) buffer;\n", "")
                        out_path.write_text(glsl)
                        print(f"[INFO] Slang {stage} OK -> {out_path}")
                    else:
                        out_path.unlink(missing_ok=True)
                        print(f"[WARNING] Failed to compile {stage} shader {name}")

                        if result.stderr:
                            print(result.stderr)
                except (OSError, UnicodeDecodeError) as error:
                    print(f"[ERROR] Failed to compile {stage}: {error}")

        return True

    def run(self) -> bool:
        self._shaders_cg_dir.mkdir(parents=True, exist_ok=True)
        self._shaders_glsl_dir.mkdir(parents=True, exist_ok=True)

        if not self.compile_slang():
            return False

        tools_cgcomp: Path = self._project_root / "tools" / "cgcomp" / "cgcomp"
        if tools_cgcomp.is_file():
            cgcomp_path: str = str(tools_cgcomp)
        else:
            cgcomp_path: str = "/usr/local/ps3dev/bin/cgcomp"

        if not Path(cgcomp_path).is_file() and not shutil.which(cgcomp_path):
            print("[ERROR] cgcomp not available.")

            return False

        cg_dirs: list[Path] = [self._shaders_dir / "cg"]
        cg_dirs += [extra_dir / "cg" for extra_dir in self._extra_dirs]
        shader_pairs: dict[str, dict[str, Path | None]] = {}

        for cg_dir in cg_dirs:
            for vcg_file in cg_dir.rglob("*.vcg"):
                if not vcg_file.is_file():
                    continue

                rel_path: Path = vcg_file.relative_to(cg_dir)
                base_rel: str = str(rel_path.with_suffix(""))
                shader_pairs[base_rel] = {"vcg": vcg_file, "fcg": None}

            for fcg_file in cg_dir.rglob("*.fcg"):
                if not fcg_file.is_file():
                    continue

                rel_path: Path = fcg_file.relative_to(cg_dir)
                base_rel: str = str(rel_path.with_suffix(""))
                if base_rel in shader_pairs:
                    shader_pairs[base_rel]["fcg"] = fcg_file
                else:
                    shader_pairs[base_rel] = {"vcg": None, "fcg": fcg_file}

        for shader_name, files in shader_pairs.items():
            vcg_file: Path | None = files["vcg"]
            fcg_file: Path | None = files["fcg"]

            print(f"[INFO] Building {shader_name}")

            vpo_path: Path = self._shaders_cg_dir / f"{shader_name}.vpo"
            fpo_path: Path = self._shaders_cg_dir / f"{shader_name}.fpo"

            vpo_path.parent.mkdir(parents=True, exist_ok=True)
            fpo_path.parent.mkdir(parents=True, exist_ok=True)

            if vcg_file:
                cmd: list[str] = [
                    cgcomp_path,
                    "-v",
                    "-Wcg",
                    str(vcg_file),
                    str(vpo_path),
                ]

                try:
                    result: subprocess.CompletedProcess[str] = subprocess.run(
                        cmd,
                        cwd=self._project_root,
                        capture_output=True,
                        text=True,
                    )

                    if result.returncode == 0:
                        print(f"[INFO] GCM vertex OK -> {vpo_path}")
                    else:
                        vpo_path.unlink(missing_ok=True)
                        print(f"[WARNING] Failed to compile vertex shader {vcg_file}")

                        # cgcomp reports its errors on stdout
                        if result.stdout:
                            print(result.stdout)
                        if result.stderr:
                            print(result.stderr)
                except (OSError, UnicodeDecodeError) as error:
                    print(f"[ERROR] Failed to compile vertex: {error}")

            if fcg_file:
                cmd = [
                    cgcomp_path,
                    "-f",
                    "-Wcg,-nofastprecision",
                    str(fcg_file),
                    str(fpo_path),
                ]

                try:
                    result = subprocess.run(
                        cmd,
                        cwd=self._project_root,
                        capture_output=True,
                        text=True,
                    )

                    if result.returncode == 0:
                        print(f"[INFO] GCM fragment OK -> {fpo_path}")
                    else:
                        fpo_path.unlink(missing_ok=True)
                        print(f"[WARNING] Failed to compile fragment shader {fcg_file}")

                        # cgcomp reports its errors on stdout
                        if result.stdout:
                            print(result.stdout)
                        if result.stderr:
                            print(result.stderr)
                except (OSError, UnicodeDecodeError) as error:
                    print(f"[ERROR] Failed to compile fragment: {error}")

        print("Shader compilation completed")

        return True


def main() -> int:
    parser: argparse.ArgumentParser = argparse.ArgumentParser()
    parser.add_argument("--shaders", action="append", default=[])
    args: argparse.Namespace = parser.parse_args()

    project_root: str = str(Path(__file__).parent.parent.resolve())
    compile_shaders: CompileShaders = CompileShaders(project_root, args.shaders)

    return 0 if compile_shaders.run() else 1


if __name__ == "__main__":
    sys.exit(main())