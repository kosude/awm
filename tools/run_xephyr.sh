#!/usr/bin/env bash

# modified from herbstluftwm/valgrind-xephyr.sh: https://github.com/herbstluftwm/herbstluftwm

die() {
    echo "$*" 1>&2 ; exit 1;
}
set -e

# print usage info
usage() {
    cat <<EOF
Usage: $0 FLAGS

Valgrind is used by default. Note that starting the window manager is
intentionally delayed when using this script!

    --valgrind          Run in valgrind (default)
    --none              Run directly
    -h --help           Print this help
EOF
}

# process to run the wm in (e.g. valgrind)
runtime() {
    valgrind --leak-check=full --track-origins=yes --suppressions="../tools/vg.supp" "$@"
}

# arg parsing
for arg in "$@" ; do
    case "$arg" in
        --valgrind)
            ;;
        --none)
            runtime() {
                "$@"
            }
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument $arg"
            usage >&2
            exit 1
            ;;
    esac
done

AWM_EXEC="./awm/awm" # relative to build directory

# get available display number
DISPLAY_NUM=
for DN in {1..99} ; do
    if [[ ! -e /tmp/.X11-unix/X$DN ]] ; then
        DISPLAY_NUM="$DN"
        break
    fi
done
if [[ -z "$DISPLAY_NUM" ]] ; then
    die "No free display index found"
fi

# start xephyr environment
$(command -v Xephyr) :$DISPLAY_NUM -screen 640x480 &
XEPHYR_PID=$!

# wait for the xephyr environment X server to be available
while sleep 0.3; do
    echo "Waiting for display :$DISPLAY_NUM to appear"
    if [[ -e /tmp/.X11-unix/X$DISPLAY_NUM ]] ; then
        break
    fi
done

# test programs
DISPLAY=:$DISPLAY_NUM $(command -v xterm) &

sleep 0.4

cd "./build/"

# run the window manager
DISPLAY=:$DISPLAY_NUM \
    runtime $AWM_EXEC \
    || echo "Non-zero exit code"

# clean up xephyr
kill $XEPHYR_PID
