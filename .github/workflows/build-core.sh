#!/bin/sh

set -ex

for item in *.c
do
    file="${item%.c}"
    clang -flto -Os -std=c99 -o "$file" "$item"
    mv "$file" ~/.xcpkg/core/
done
