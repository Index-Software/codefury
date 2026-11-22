#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

LOG_DIR="$ROOT_DIR/build/logs"
LOG_FILE="$LOG_DIR/pull_submodules.log"
mkdir -p "$LOG_DIR"
: > "$LOG_FILE"
exec 3>&1 1>>"$LOG_FILE" 2>&1

PULL_GCMGL=0
PULL_MATHSFURY=0
PULL_WRENBIND=0
PULL_WREN=0
PULL_GLAD=0
PULL_SIMDE=0
for arg in "$@"; do
	case $arg in
		--pull-gcmgl) PULL_GCMGL=1 ;;
		--pull-mathsfury) PULL_MATHSFURY=1 ;;
		--pull-wrenbind) PULL_WRENBIND=1 ;;
		--pull-wren) PULL_WREN=1 ;;
		--pull-glad) PULL_GLAD=1 ;;
		--pull-simde) PULL_SIMDE=1 ;;
	esac
done

if ! command -v git >/dev/null 2>&1; then
	printf '[ERROR] git not found\n' >&3

	exit 1
fi

echo "Updating submodules..."

GCMGL_DIR="$ROOT_DIR/thirdparty/gcmgl"
MATHSFURY_DIR="$ROOT_DIR/thirdparty/mathsfury"
WRENBIND_DIR="$ROOT_DIR/thirdparty/wrenbind"
WREN_DIR="$ROOT_DIR/thirdparty/wren"
GLAD_DIR="$ROOT_DIR/thirdparty/glad"
SIMDE_DIR="$ROOT_DIR/thirdparty/simde"

GCMGL_BRANCH=origin/main
MATHSFURY_BRANCH=origin/main
WRENBIND_BRANCH=origin/master
WREN_BRANCH=origin/main
GLAD_BRANCH=origin/glad2
SIMDE_BRANCH=origin/master

update_submodule() {
	local name="$1"
	local dir="$2"
	local branch="$3"

	echo "Updating $name submodule..."

	if git -C "$dir" fetch && git -C "$dir" checkout "$branch"; then
		git add "$dir"
		printf '%s updated\n' "$name" >&3

		return 0
	fi

	printf '[ERROR] %s update failed\n' "$name" >&3

	return 1
}

git submodule sync --recursive
git submodule update --init --recursive
printf 'Submodules synced and initialized\n' >&3

if [ "$PULL_GCMGL" -eq 1 ]; then
	update_submodule gcmgl "$GCMGL_DIR" "$GCMGL_BRANCH"
fi

if [ "$PULL_MATHSFURY" -eq 1 ]; then
	update_submodule mathsfury "$MATHSFURY_DIR" "$MATHSFURY_BRANCH"
fi

if [ "$PULL_WRENBIND" -eq 1 ]; then
	update_submodule wrenbind "$WRENBIND_DIR" "$WRENBIND_BRANCH"
fi

if [ "$PULL_WREN" -eq 1 ]; then
	update_submodule wren "$WREN_DIR" "$WREN_BRANCH"
fi

if [ "$PULL_GLAD" -eq 1 ]; then
	update_submodule glad "$GLAD_DIR" "$GLAD_BRANCH"
fi

if [ "$PULL_SIMDE" -eq 1 ]; then
	update_submodule simde "$SIMDE_DIR" "$SIMDE_BRANCH"
fi

echo ""
echo "Committing updated submodules..."

if git commit -m "feat: update submodules"; then
	printf 'Submodules committed\n' >&3
else
	printf '[INFO] No submodule changes to commit\n' >&3
fi

printf 'Submodule update complete\n' >&3