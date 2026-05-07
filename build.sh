#!/bin/sh

log_green() {
    echo -e "\033[0;32m[INFO] $1 \033[0m"
}

log_red() {
    echo -e "\033[0;31m[ERROR] $1 \033[0m"
}

log_result() {
    if [ $? -eq 0 ]; then
        log_green "SUCCESS"
    else
        log_red "FAILURE"
    fi
}

build_firmware() {
    echo "BUILDING FIRMWARE..."

    cmake --preset Debug firmware/
    cmake --build firmware/build/Debug -j

    log_result
}

build_gui() {
    echo "BUILDING GUI..."

    cmake -S gui/ -B gui/build
    cmake --build gui/build -j

    log_result
}

if [ "$1" == "clean" ]; then
    cmake --build firmware/build/Debug --target clean
    cmake --build gui/build --target clean
else
    build_firmware
    build_gui
fi
