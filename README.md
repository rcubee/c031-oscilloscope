# c031-oscilloscope

## Overview

This is a repository for an STM32C031-based oscilloscope toy project. It consists of two main components: MCU firmware and a Qt6 GUI, written in C11 and C++20 respectively. The firmware uses almost no HAL/LL code, interacting directly with the hardware registers.

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
