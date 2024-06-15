#!/usr/bin/env bash

SPHINXEXEC=$1
SRC_DIR=$2
DST_DIR=$3
TOOLS_DIR=$4

CMD="$SPHINXEXEC -q -b html $SRC_DIR "$DST_DIR/html""

echo "conf_wrapper.sh: Running: \"$CMD\"" # for diagnostics (particularly in CD pipelines)

if [[ -z "$CONFPY_VERSION_OVERRIDE" ]]; then
    export CONFPY_VERSION=$($TOOLS_DIR/gversion.sh) --long
else
    export CONFPY_VERSION="$CONFPY_VERSION_OVERRIDE"
fi

# run sphinx-build with env vars set
$CMD
