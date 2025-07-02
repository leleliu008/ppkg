#!/bin/sh

set -ex

NATIVE_OS_VERS="$(sw_vers -productVersion)"
NATIVE_OS_ARCH="$(uname -m)"

SYSROOT="$(xcrun --sdk macosx --show-sdk-path)"

export PROXIED_TARGET_CC="$(xcrun --sdk macosx --find clang)"
export WRAPPER_TARGET_CCFLAGS="-isysroot $SYSROOT     -mmacosx-version-min=$NATIVE_OS_VERS -arch $NATIVE_OS_ARCH -Qunused-arguments -fno-common -ldl"

export PPKG_VERBOSE=1

export PACKAGE_CREATE_MOSTLY_STATICALLY_LINKED_EXECUTABLE=1

clang -flto -Os -o wrapper-target-cc wrapper-target-cc.c

./wrapper-target-cc -flto -Os -o sed-in-place ../sed-in-place.c
