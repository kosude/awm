#!/usr/bin/env bash

SPHINXEXEC=$1
CURRENT_LIST_DIR=$2
CURRENT_BINARY_DIR=$3
TOOLS_DIR=$4

CMD="$SPHINXEXEC -q -b html $CURRENT_LIST_DIR "$CURRENT_BINARY_DIR/html""

echo "-- $CMD" # for diagnostics (particularly in CD pipelines)

# run sphinx-build with env vars set
CONFPY_VERSION="$($TOOLS_DIR/gversion.sh --long)" \
$CMD
