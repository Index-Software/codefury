#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

LOG_DIR="$ROOT_DIR/build/logs"
mkdir -p "$LOG_DIR"
exec > >(tee "$LOG_DIR/build_msdf_atlas_gen.log") 2>&1

FREETYPE_TAG="VER-2-13-3"
LIBPNG_TAG="v1.6.43"
MSDF_ATLAS_GEN_COMMIT="6148900d59423059bafde2f51a0cb303184404bd"

OUT_DIR="$ROOT_DIR/tools/msdf-atlas-gen"

for tool in git cmake g++ make; do
	if ! command -v "$tool" >/dev/null 2>&1; then
		echo "[ERROR] $tool not found"

		exit 1
	fi
done

WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT
PREFIX="$WORK_DIR/prefix"
JOBS="$(nproc 2>/dev/null || echo 4)"

echo "Building FreeType $FREETYPE_TAG..."
git clone -q --depth 1 --branch "$FREETYPE_TAG" \
	https://github.com/freetype/freetype.git "$WORK_DIR/freetype"
cmake -S "$WORK_DIR/freetype" -B "$WORK_DIR/freetype-build" \
	-DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_BUILD_TYPE=Release \
	-DFT_DISABLE_BZIP2=ON -DFT_DISABLE_BROTLI=ON -DFT_DISABLE_HARFBUZZ=ON \
	-DFT_DISABLE_PNG=ON -DFT_DISABLE_ZLIB=ON >/dev/null
cmake --build "$WORK_DIR/freetype-build" -j"$JOBS" >/dev/null
cmake --install "$WORK_DIR/freetype-build" >/dev/null

echo "Building libpng $LIBPNG_TAG..."
git clone -q --depth 1 --branch "$LIBPNG_TAG" \
	https://github.com/pnggroup/libpng.git "$WORK_DIR/libpng"
cmake -S "$WORK_DIR/libpng" -B "$WORK_DIR/libpng-build" \
	-DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_BUILD_TYPE=Release \
	-DPNG_SHARED=OFF -DPNG_TESTS=OFF -DPNG_TOOLS=OFF >/dev/null
cmake --build "$WORK_DIR/libpng-build" -j"$JOBS" >/dev/null
cmake --install "$WORK_DIR/libpng-build" >/dev/null

echo "Building msdf-atlas-gen $MSDF_ATLAS_GEN_COMMIT..."
git clone -q --recurse-submodules \
	https://github.com/Chlumsky/msdf-atlas-gen.git "$WORK_DIR/msdf-atlas-gen"
git -C "$WORK_DIR/msdf-atlas-gen" checkout -q "$MSDF_ATLAS_GEN_COMMIT"
git -C "$WORK_DIR/msdf-atlas-gen" submodule update -q --init --recursive
cmake -S "$WORK_DIR/msdf-atlas-gen" -B "$WORK_DIR/msdf-atlas-gen-build" \
	-DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PREFIX" \
	-DMSDF_ATLAS_USE_VCPKG=OFF -DMSDF_ATLAS_USE_SKIA=OFF \
	-DMSDF_ATLAS_NO_ARTERY_FONT=ON -DMSDFGEN_DISABLE_SVG=ON \
	-DMSDFGEN_USE_SKIA=OFF >/dev/null
cmake --build "$WORK_DIR/msdf-atlas-gen-build" -j"$JOBS" >/dev/null

mkdir -p "$OUT_DIR"
cp "$WORK_DIR/msdf-atlas-gen-build/bin/msdf-atlas-gen" "$OUT_DIR/msdf-atlas-gen"

echo "Installed $OUT_DIR/msdf-atlas-gen"