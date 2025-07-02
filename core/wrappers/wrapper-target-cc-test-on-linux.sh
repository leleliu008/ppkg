#!/bin/sh

set -ex

export PROXIED_TARGET_CC="$(command -v cc)"
export WRAPPER_TARGET_CCFLAGS="-s    -Wl,-v"
export WRAPPER_TARGET_LDFLAGS="  -lpthread  -ldl -lc   -lstdc++"

export PPKG_VERBOSE=1

export PACKAGE_CREATE_MOSTLY_STATICALLY_LINKED_EXECUTABLE=1

cc -flto -Os -o wrapper-target-cc wrapper-target-cc.c

./wrapper-target-cc -c -Os -o ../sed-in-place.c

true "=================================="

./wrapper-target-cc -flto -Os -o sed-in-place ../sed-in-place.c /usr/lib/x86_64-linux-gnu/libm-2.39.a

true "=================================="

./wrapper-target-cc -static -flto -Os -o sed-in-place ../sed-in-place.c /usr/lib/x86_64-linux-gnu/libm-2.39.a
