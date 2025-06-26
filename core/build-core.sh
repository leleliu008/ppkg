#!/bin/sh

set -ex

for f in wrappers/*.c
do
    o="${f%.c}"
    cc -flto -Os -std=gnu99 -o "$o" "$f"
    strip "$o"
    mv "$o" ~/.ppkg/core/
done
