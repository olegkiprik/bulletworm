<img src="Resources/Textures/icon.png" alt="logo" width="60"/>

# BulletWorm

## Requirements

- [ ] Sane system specifications. Emulation should be OK.
- [ ] GNU/Linux on x86_64

## Installation

Unpack the .tar.xz archive downloaded from Releases. Run the executable. If failure occurs, then consider compiling from source.

## Compiling from source on Linux

Execute the following commands with <kbd>$PWD</kbd> set to your repo clone. Make sure you have [SFML](http://sfml-dev.org) 2.6.1 installed. If this version is not available, see the next section.

<kbd>time g++ src/\*.cpp src/engine/\*.c\* lib/src/bw_ext/\*.c\* lib/src/bw_ext/random/\*.c\* lib/src/bw_ext/stream/\*.c\* -o bulletworm -O2 -flto -DNDEBUG -march=native -pipe -W -std=c++17 -I lib/include/ -lsfml-system -lsfml-window -lsfml-graphics -lsfml-audio
</kbd> to compile and link

<kbd>./bulletworm</kbd> to play

## Compiling from source on Linux (advanced, should work on any distribution)

- [ ] [List of dependencies to install](https://www.sfml-dev.org/tutorials/2.6/compile-with-cmake.php#installing-dependencies)

Use the script *compile.sh*. Set the variable SFML_SRC_PATH to your location of [SFML sources](https://www.sfml-dev.org/files/SFML-2.6.1-sources.zip) directory (without *SFML-...* at the end).

## Screenshots

![Image 0](demo/screenshot_00.jpg)

![Image 1](demo/screenshot_01.jpg)

![Image 3](demo/screenshot_03.jpg)

![Image 4](demo/screenshot_04.jpg)

![Image 5](demo/screenshot_05.png)
