#!/bin/sh

set -ex

for f in wrappers/*.c elftools/*.c
do
    o="${f%.c}"
    cc -std=gnu99 -Os -s -flto -o "$o" "$f"
    mv "$o" ~/.ppkg/core/
done
