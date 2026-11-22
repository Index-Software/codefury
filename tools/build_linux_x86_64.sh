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
CMAKE_DIR="$BUILD_DIR/cmake"

echo "Cleaning previous build..."
rm -rf "$BUILD_DIR/bin/linux-x86_64" "$BUILD_DIR/obj/linux-x86_64" "$CMAKE_DIR" || true

export BUILD_TYPE=Release

mkdir -p "$CMAKE_DIR"

if ! command -v cmake >/dev/null 2>&1; then
	echo "[ERROR] cmake not found"

	exit 1
fi

cmake -DAPP="$APP" -DBUILD_PS3=OFF -B "$CMAKE_DIR" -S "$ROOT_DIR"

echo "Compiling..."

if ! command -v make >/dev/null 2>&1; then
	echo "[ERROR] make not found"

	exit 1
fi

make -j"$(nproc)" -C "$CMAKE_DIR"

echo "Build completed"