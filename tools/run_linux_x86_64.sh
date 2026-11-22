#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_DIR="$ROOT_DIR/build"
BIN="$BUILD_DIR/bin/linux-x86_64/codefury"

if [ ! -x "$BIN" ]; then
	echo "[ERROR] Binary not found"

	exit 2
fi

echo "Running $BIN"
cd "$BUILD_DIR/bin/linux-x86_64"
"$BIN"