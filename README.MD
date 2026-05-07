# c031-oscilloscope

## Overview

This is a repository for an STM32C031-based oscilloscope toy project. It is composed of two components: MCU firmware and a Qt6 GUI, written in C and C++ respectively.

## Building
This project is meant to be built on Linux with GCC. The MCU toolchain and Qt6, along with the rest of the dependencies, must be installed beforehand.

The provided basic shell script can be used to build the entire project:
```bash
./build.sh # Build firmware and GUI

# firmware build directory:
firmware/build/Debug/

# GUI build directory:
gui/build/
```

## Screenshots

![GUI screenshot](/assets/gui.jpg)
