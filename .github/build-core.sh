#!/bin/sh

set -ex

run() {
  printf "\033[0;35m==>\033[0m \033[0;32m%b\n\033[0m" "$*"
  eval "$@"
}

if [ -f cacert.pem ] ; then
    run export SSL_CERT_FILE="$PWD/cacert.pem"
fi

run ./ppkg setup --syspm
run ./ppkg update
run ./ppkg install uppm@0.15.4 --static
run ./ppkg bundle  uppm@0.15.4 .tar.xz

install -d out/

for f in wrappers/*.c elftools/*.c
do
    x="${f#*/}"
    o="out/${x%.c}"
    run cc -flto -Os -std=gnu99 -o "$o" "$f"
    run strip "$o"
done

run export SSL_CERT_FILE="$HOME/.ppkg/core/cacert.pem"

curl -LO https://raw.githubusercontent.com/adobe-fonts/source-code-pro/release/OTF/SourceCodePro-Light.otf

bsdtar vxf uppm*.tar.xz -C out --strip-components=1

mv out/bin/uppm *.otf core/fonts.conf out/

rm -rf out/bin/ out/share/ out/.ppkg/

NATIVE_OS_KIND="$(uname -s | tr A-Z a-z)"
NATIVE_OS_ARCH="$(uname -m)"

case $NATIVE_OS_KIND in
    linux)
        DIRNAME="ppkg-core-$1-$NATIVE_OS_KIND-$NATIVE_OS_ARCH"
        ;;
    *)  NATIVE_OS_VERS="$(uname -r)"
        NATIVE_OS_VERS="${NATIVE_OS_VERS%%-*}"
        DIRNAME="ppkg-core-$1-$NATIVE_OS_KIND-$NATIVE_OS_VERS-$NATIVE_OS_ARCH"
esac

mv out "$DIRNAME"

bsdtar cavf "$DIRNAME.tar.xz" "$DIRNAME"
