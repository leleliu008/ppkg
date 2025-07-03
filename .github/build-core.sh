#!/bin/sh

set -ex

run() {
  printf "\033[0;35m==>\033[0m \033[0;32m%b\n\033[0m" "$*"
  eval "$@"
}

if [ -z "$1" ] || [ -z "$2" ] ; then
    printf 'Usage: .github/build-core.sh <RELEASE-VERSION> <TARGET-PLATFORM-SPEC>\n' >&2
    exit 1
fi

######################################################

if [ "$2" = 'linux-musl-armhf' ] ; then
    TARGET_PLATFORM_SPEC='linux-musl-armv7l'
else
    TARGET_PLATFORM_SPEC="$2"
fi

######################################################

TARGET_PLATFORM_NAME="${TARGET_PLATFORM_SPEC%%-*}"
TARGET_PLATFORM_ARCH="${TARGET_PLATFORM_SPEC##*-}"

######################################################

if [ -f cacert.pem ] ; then
    run export SSL_CERT_FILE="$PWD/cacert.pem"
fi

######################################################

if [ "$TARGET_PLATFORM_NAME" = macos ] ; then
    run ./ppkg setup
else
    run ./ppkg setup --syspm
fi

run ./ppkg update
run ./ppkg install $TARGET_PLATFORM_SPEC/uppm@0.15.4 --static
run ./ppkg bundle  $TARGET_PLATFORM_SPEC/uppm@0.15.4 .tar.xz

######################################################

install -d bundle.d/

if [ "$TARGET_PLATFORM_NAME" = macos ] ; then
    TARGET_PLATFORM_XXXX="${2%-*}"
    TARGET_PLATFORM_VERS="${TARGET_PLATFORM_XXXX##*-}"

    CC="$(xcrun --sdk macosx --find clang)"
    SYSROOT="$(xcrun --sdk macosx --show-sdk-path)"
    CFLAGS="-isysroot $SYSROOT -mmacosx-version-min=$TARGET_PLATFORM_VERS -arch $TARGET_PLATFORM_ARCH -Qunused-arguments"

    for f in core/wrappers/*.c
    do
        x="${f##*/}"
        o="bundle.d/${x%.c}"
        run $CC $CFLAGS -std=c99 -Os -flto -o "$o" "$f"
        run strip "$o"
    done
else
    for f in core/wrappers/*.c core/elftools/*.c
    do
        x="${f##*/}"
        o="bundle.d/${x%.c}"
        run cc -std=gnu99 -Os -s -flto -static -o "$o" "$f"
    done
fi

######################################################

run curl -LO https://raw.githubusercontent.com/adobe-fonts/source-code-pro/release/OTF/SourceCodePro-Light.otf

######################################################

if [ "$TARGET_PLATFORM_NAME" = linux ] ; then
    BUNDLE_DIRNAME="ppkg-core-$1-$TARGET_PLATFORM_NAME-$TARGET_PLATFORM_ARCH"
else
    BUNDLE_DIRNAME="ppkg-core-$1-$TARGET_PLATFORM_SPEC"
fi

######################################################

run bsdtar vxf uppm*.tar.xz -C bundle.d --strip-components=1

mv bundle.d/bin/uppm *.otf core/fonts.conf bundle.d/

rm -rf bundle.d/bin/ bundle.d/share/ bundle.d/.ppkg/

mv bundle.d "$BUNDLE_DIRNAME"

run bsdtar cavf "$BUNDLE_DIRNAME.tar.xz" "$BUNDLE_DIRNAME"
