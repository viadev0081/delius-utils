#!/usr/bin/bash

SOURCES=(
    src/cash_term.cpp
    src/lib/lib.cpp
)

CXX_FLAGS=(
    -std=c++17
    -O2
    -Wall
    -Wextra
)

function build {
    local root
    root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
    cd "$root" || exit 1

    mkdir -p bin/
    clang++ "${CXX_FLAGS[@]}" "${SOURCES[@]}" -o bin/cash_term -lncursesw || exit 1
    bin/cash_term
}

build
