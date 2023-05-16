#!/bin/bash

fallback() {
        sudo rm -rf "$(pwd)"
        popd || exit
}

cmake_install() {
        if [ -z "$1" ]
        then
                return 1
        fi
        pushd "$(mktemp -d)" || exit
        git clone "$1"
        if [ $? -eq 128 ]
        then
                fallback
                return 1
        fi
        pushd "$(ls)" || exit
        mkdir build
        pushd build || exit
        sudo cmake .. && sudo make && sudo make install
        popd && popd && fallback
}
