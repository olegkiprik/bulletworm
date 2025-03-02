<img src="Resources/Textures/icon.png" alt="logo" width="60"/>

# BulletWorm

## Installation on Linux (the straightforward way)

To install the app on Linux via terminal, execute the following commands with <kbd>$PWD</kbd> set to your repo clone. Make sure you have [SFML](http://sfml-dev.org) 2.6.1 installed. If this version is not available, see the next section.

<kbd>time g++ src/\*.cpp src/engine/\*.c\* lib/src/bw_ext/\*.c\* lib/src/bw_ext/random/\*.c\* lib/src/bw_ext/stream/\*.c\* -o bulletworm -O2 -flto -DNDEBUG -march=native -pipe -W -std=c++17 -I lib/include/ -lsfml-system -lsfml-window -lsfml-graphics -lsfml-audio
</kbd> to compile and link

<kbd>./bulletworm</kbd> to play

## Installation on Linux (for hackers)

Use the script *compile.sh*. [SFML sources](https://www.sfml-dev.org/files/SFML-2.6.1-sources.zip)

## Screenshots

![Image 0](demo/screenshot_00.jpg)

![Image 1](demo/screenshot_01.jpg)

![Image 3](demo/screenshot_03.jpg)

![Image 4](demo/screenshot_04.jpg)

![Image 5](demo/screenshot_05.png)
