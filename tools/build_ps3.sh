#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "1 - Template"
echo "2 - Demo"
read -p "Select an example to build: " choice
choice=${choice:-1}

case $choice in
	1)
		APP="Template"
		;;
	2)
		APP="Demo"
		;;
	*)
		echo "Invalid choice"
		APP="Template"
		;;
esac

echo "Building example: $APP"

BUILD_DIR="$ROOT_DIR/build"
mkdir -p "$BUILD_DIR"

export PS3DEV="${PS3DEV:-/usr/local/ps3dev}"
export PSL1GHT="${PSL1GHT:-/usr/local/ps3dev}"
export APP="$APP"
export BUILD_TYPE=Release

echo "Cleaning previous build..."
rm -rf "$BUILD_DIR/bin/ps3" "$BUILD_DIR/obj/ps3" || true

echo "Building content..."
if ! command -v python3 >/dev/null 2>&1; then
	echo "[ERROR] python3 not found"

	exit 1
fi

python3 "$ROOT_DIR/tools/build_content.py" --platform ps3 --output "$BUILD_DIR/bin/ps3/USRDIR"

echo "Compiling..."
if ! command -v make >/dev/null 2>&1; then
	echo "[ERROR] make not found"

	exit 1
fi

make -C "$ROOT_DIR" -f "$ROOT_DIR/Makefile.ps3"

if [ -f "$BUILD_DIR/bin/ps3/codefury.self" ]; then
	cp "$BUILD_DIR/bin/ps3/codefury.self" "$BUILD_DIR/bin/ps3/USRDIR/EBOOT.BIN"

	if [ -f "$ROOT_DIR/PARAM.SFO.xml" ]; then
		if command -v python3 >/dev/null 2>&1 && [ -f "$PS3DEV/bin/sfo.py" ]; then
			python3 "$PS3DEV/bin/sfo.py" --fromxml "$ROOT_DIR/PARAM.SFO.xml" "$BUILD_DIR/bin/ps3/PARAM.SFO" || {
				echo "[ERROR] Build failed, sfo.py failed"

				exit 1
			}
		else
			echo "[ERROR] Build failed, sfo.py not available"

			exit 1
		fi
	fi

	if command -v convert >/dev/null 2>&1; then
		convert -size 320x176 xc:blue -fill white -gravity center -pointsize 24 -annotate +0+0 "codefury" "$BUILD_DIR/bin/ps3/ICON0.PNG" || {
			echo "[WARNING] convert failed, writing placeholder ICON0.PNG"
			touch "$BUILD_DIR/bin/ps3/ICON0.PNG"
		}
	else
		echo "[WARNING] convert not found, writing placeholder ICON0.PNG"
		touch "$BUILD_DIR/bin/ps3/ICON0.PNG"
	fi

	mkdir -p "$BUILD_DIR/obj/ps3"
	mv "$BUILD_DIR/bin/ps3/codefury.elf" "$BUILD_DIR/bin/ps3/codefury.elf.map" "$BUILD_DIR/bin/ps3/codefury.fake.self" "$BUILD_DIR/obj/ps3/" 2>/dev/null || true
else
	echo "[ERROR] Build failed"

	exit 1
fi

echo "Build completed"