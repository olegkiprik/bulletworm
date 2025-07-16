<img src="Resources/Textures/icon.png" alt="logo" width="60"/>

# BulletWorm

## Requirements

- [ ] Moderate system specifications. Emulation should be OK.
- [ ] GNU/Linux operating system on x86_64 architecture. With some tweaks, it should be possible to run the game on any other desktop OS.

## Installation

Unpack the .tar.xz archive downloaded from Releases. Try to run the executable. If it fails, then consider compiling from source, see the instructions below.

## Compiling from source on Linux

Execute the following commands inside the repo directory. Make sure you have [SFML](http://sfml-dev.org) 2.6.1 installed. If this version is not available on your distribution, read the next section.

<kbd>time g++ src/\*.cpp src/engine/\*.c\* lib/src/bw_ext/\*.c\* lib/src/bw_ext/random/\*.c\* lib/src/bw_ext/stream/\*.c\* -o bulletworm -O2 -flto -DNDEBUG -march=native -pipe -W -std=c++17 -I lib/include/ -lsfml-system -lsfml-window -lsfml-graphics -lsfml-audio
</kbd> to compile and link

<kbd>./bulletworm</kbd> to play

## Compiling from source on Linux (advanced, should work on any distribution)

- [ ] [List of dependencies to install](https://www.sfml-dev.org/tutorials/2.6/compile-with-cmake.php#installing-dependencies)

- [ ] Set the variable SFML_SRC_PATH to your location of [SFML sources](https://www.sfml-dev.org/files/SFML-2.6.1-sources.zip) directory (without *SFML-...* at the end). For example:

<kbd>export SFML_SRC_PATH=~/Downloads/</kbd>

- [ ] Run the script:

<kbd>time ./compile.sh</kbd>

It takes some time.

## Screenshots

![Image 1](demo/screenshot_01.jpg)

![Image 3](demo/screenshot_03.jpg)

![Image 4](demo/screenshot_04.jpg)
