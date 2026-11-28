#!/usr/bin/bash

function install-deps {
    echo "change your distro group: "
    echo "1.arch  2.debian  3.void"
    read group

    if [ "$group" == "1" ]; then
        sudo pacman -S clang cmake ncurses gcc-fortran base-devel
    elif [ "$group" == "2" ]; then
        sudo apt install clang cmake libncurses-dev gfortran
    elif [ "$group" == "3" ]; then
        sudo xbps-install clang cmake ncurses base-devel gcc-fortran ncurses-devel
    else
        echo "unknown distro group"
    fi
}

install-deps
