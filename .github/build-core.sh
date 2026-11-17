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

unset SYSPM
unset STATIC

case $TARGET_PLATFORM_NAME in
    macos)
        ;;
    dragonflybsd)
        run pkg install -y curl libnghttp2 gcc
        STATIC=--static
        ;;
    *)  STATIC=--static
        SYSPM=--syspm
esac

run ./ppkg about
run ./ppkg setup $SYSPM
run ./ppkg update

run ./ppkg install $TARGET_PLATFORM_SPEC/uppm@0.15.4 $STATIC
run ./ppkg bundle  $TARGET_PLATFORM_SPEC/uppm@0.15.4 .tar.xz

run ./ppkg install $TARGET_PLATFORM_SPEC/elftool $STATIC
run ./ppkg bundle  $TARGET_PLATFORM_SPEC/elftool .tar.xz

run ./ppkg install $TARGET_PLATFORM_SPEC/ppkg-core $STATIC
run ./ppkg bundle  $TARGET_PLATFORM_SPEC/ppkg-core .tar.xz

######################################################

run curl -LO https://raw.githubusercontent.com/adobe-fonts/source-code-pro/release/OTF/SourceCodePro-Light.otf

######################################################

cat > fonts.conf <<EOF
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "urn:fontconfig:fonts.dtd">
<fontconfig>
	<description>custom configuration file</description>

	<dir>PPKG_CORE_DIR</dir>

	<match target="pattern">
		<test qual="any" name="family">
			<string>mono</string>
		</test>
		<edit name="family" mode="assign" binding="same">
			<string>monospace</string>
		</edit>
	</match>

	<match target="pattern">
		<test qual="any" name="family">
			<string>sans serif</string>
		</test>
		<edit name="family" mode="assign" binding="same">
			<string>sans-serif</string>
		</edit>
	</match>

	<match target="pattern">
		<test qual="any" name="family">
			<string>sans</string>
		</test>
		<edit name="family" mode="assign" binding="same">
			<string>sans-serif</string>
		</edit>
	</match>

	<match target="pattern">
		<test qual="any" name="family">
			<string>system ui</string>
		</test>
		<edit name="family" mode="assign" binding="same">
			<string>system-ui</string>
		</edit>
	</match>

</fontconfig>
EOF

######################################################

if [ "$TARGET_PLATFORM_NAME" = linux ] ; then
    BUNDLE_DIRNAME="ppkg-core-$1-$TARGET_PLATFORM_NAME-$TARGET_PLATFORM_ARCH"
else
    BUNDLE_DIRNAME="ppkg-core-$1-$TARGET_PLATFORM_SPEC"
fi

######################################################

install -d bundle.d/

for f in *.tar.xz
do
    run bsdtar vxf "$f" -C bundle.d --strip-components=1
done

mv bundle.d/bin/* *.otf fonts.conf bundle.d/

rm -rf bundle.d/bin/ bundle.d/share/ bundle.d/.ppkg/

mv bundle.d "$BUNDLE_DIRNAME"

run bsdtar cavf "$BUNDLE_DIRNAME.tar.xz" "$BUNDLE_DIRNAME"
