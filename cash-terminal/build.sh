#!/usr/bin/bash

function build {
    exec "$(dirname "$0")/build/build.sh"
}

build
