#!/usr/bin/env bash

# This script is used to set environment variables before configuring the Sphinx documentation

SPHINXEXEC=$1
CURRENT_LIST_DIR=$2
CURRENT_BINARY_DIR=$3
TOOLS_DIR=$4

CONFPY_VERSION="$($TOOLS_DIR/gversion.sh --long)" \
$SPHINXEXEC -q -b html $CURRENT_LIST_DIR "$CURRENT_BINARY_DIR/html"
