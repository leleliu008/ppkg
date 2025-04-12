#!/bin/sh

case $1 in
    dot)
        export FONTCONFIG_FILE=/etc/fonts/fonts.conf
        ~/.ppkg/uppm/installed/dot_static/bin/dot_static -Tsvg -o phases.svg phases.dot
        ;;
    d2) ~/.ppkg/uppm/installed/d2/bin/d2 phases.d2 phases.svg
    *)  printf 'unrecognized argument: $1 , Usage: $0 dot|d2\n'
esac
