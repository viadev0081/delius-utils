#!/usr/bin/bash

CXX_FLAGS=(
    -std=c++17
    -O2
    -Wall
    -Wextra
)

function build {
    cd ~/dev/fastfetch-logos || exit 1
    mkdir -p bin/
    clang++ "${CXX_FLAGS[@]}" src/main.cpp -o bin/ff-logos -lncursesw || exit 1
    bin/ff-logos
}

build
