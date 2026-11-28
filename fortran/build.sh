#!/usr/bin/bash

mkdir -p ./bin/

function build_programs {
    gfortran -J ./bin cmd.f95 -o bin/cmd
}

build_programs
