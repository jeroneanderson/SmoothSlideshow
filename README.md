# Smooth Slideshow C++

A native C++ Qt implementation of the Smooth Slideshow viewer, designed for efficiency on Raspberry Pi.

## Prerequisites

You need a C++ compiler and Qt5 development libraries.

On Raspberry Pi OS (Debian based):
```bash
sudo apt update
sudo apt install build-essential cmake qtbase5-dev qt5-default
```
*(Note: on newer Debian versions `qt5-default` might be replaced by specific packages, but `qtbase5-dev` is usually sufficient with CMake)*

## Build Instructions

1. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

2. Run CMake:
   ```bash
   cmake ..
   ```

3. Compile:
   ```bash
   make
   ```

4. Run:
   ```bash
   ./SmoothSlideshow
   ```

## Configuration

Settings are saved in `~/.config/Endless_Slides/config.json`.
Thumbnails are cached in `~/.config/Endless_Slides/thumbnails`.
