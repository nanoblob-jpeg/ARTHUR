#!/bin/sh

SCRIPT_PATH=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_PATH")
LIB_PRELOAD_PATH=$(readlink -f "$SCRIPT_DIR/libmalloc.so")

LD_PRELOAD="$LIB_PRELOAD_PATH" $@
