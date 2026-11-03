#!/usr/bin/bash

SOURCES=(
    src/chscript.cpp
    src/lib/lib.cpp
)

CXX_FLAGS=(
    -std=c++17
    -O2
    -Wall 
    -Wextra
)

function build {
    cd ~/delius-utils/change-script || exit 1
    mkdir -p bin/
    clang++ "${CXX_FLAGS[@]}" "${SOURCES[@]}" -o bin/cs -lncursesw || exit 1
    bin/cs
}

build
