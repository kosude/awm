#!/usr/bin/env bash

SCRIPT_DIR="$(dirname -- "$(readlink -f "$BASH_SOURCE")")"
BUILD_DIR="$(dirname -- "$SCRIPT_DIR")/build/plugins"

PLUGINS=(
    "test.so"
)

CP="$(command -v cp)"

function noop {
    return
}
alias echo=echo

for arg in "$@"; do
    case "$arg" in
        -q|--quiet)
            alias echo=noop
            ;;
    esac
done

dst="$SCRIPT_DIR/config/plugins"

srcs=()
for p in "${PLUGINS[@]}"; do
    srcs+=("$BUILD_DIR/$p")
done

len="$(wc -w <<< "$srcs")"
echo "Copying $len plugin(s) to test config directory..."

printf -v srcsstr ' %s' "${srcs[@]}"
srcsstr=${srcsstr:1}

cmd="$CP $srcsstr $dst"
echo -en "\n$cmd\n\n"
$cmd
