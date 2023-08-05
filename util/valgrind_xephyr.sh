#!/usr/bin/env bash

# modified from herbstluftwm/valgrind-xephyr.sh: https://github.com/herbstluftwm/herbstluftwm

die() {
    echo "$*" 1>&2 ; exit 1;
}
set -e

# process to run the wm in (e.g. valgrind)
runtime() {
    valgrind --leak-check=full "$@"
}

AWM_EXEC="./build/awm/awm" # relative to repository root directory

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
$(command -v Xephyr) :$DISPLAY_NUM -resizeable &
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

# run the window manager
DISPLAY=:$DISPLAY_NUM \
    runtime $AWM_EXEC \
    || echo "Non-zero exit code"

# clean up xephyr
kill $XEPHYR_PID
