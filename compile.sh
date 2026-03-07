#!/bin/sh

wget https://www.sfml-dev.org/files/SFML-2.6.2-sources.zip
unzip SFML-2.6.2*
SFML_SRC_PATH=$PWD

g++ \
-std=c++17 \
-s \
-Os \
-DNDEBUG \
-ffast-math \
-flto \
src/*.c* \
src/engine/*.c* \
lib/src/bw_ext/*.c* \
lib/src/bw_ext/random/*.c* \
lib/src/bw_ext/stream/*.c* \
$SFML_SRC_PATH/SFML-2.6.2/src/SFML/Audio/*.c* \
$SFML_SRC_PATH/SFML-2.6.2/src/SFML/Graphics/*.c* \
$SFML_SRC_PATH/SFML-2.6.2/src/SFML/Window/*.c* \
$SFML_SRC_PATH/SFML-2.6.2/src/SFML/System/*.c* \
$SFML_SRC_PATH/SFML-2.6.2/src/SFML/System/Unix/*.c* \
$SFML_SRC_PATH/SFML-2.6.2/src/SFML/Window/Unix/*.c* \
-I lib/include/ \
-I $SFML_SRC_PATH/SFML-2.6.2/include/ \
-I $SFML_SRC_PATH/SFML-2.6.2/src/ \
-I $SFML_SRC_PATH/SFML-2.6.2/extlibs/headers/AL \
-I $SFML_SRC_PATH/SFML-2.6.2/extlibs/headers/glad/include/ \
-I $SFML_SRC_PATH/SFML-2.6.2/extlibs/headers/minimp3/ \
-I $SFML_SRC_PATH/SFML-2.6.2/extlibs/headers/freetype2/ \
-I $SFML_SRC_PATH/SFML-2.6.2/extlibs/headers/stb_image/ \
-I $SFML_SRC_PATH/SFML-2.6.2/extlibs/headers/vulkan/ \
-o bulletworm \
-l openal \
-l GL \
-l freetype \
-l X11 \
-l pthread \
-l m \
-l vulkan \
-l udev \
-l vorbis \
-l ogg \
-l FLAC \
-l Xrandr \
-l Xcursor \
-l vorbisfile \
-l vorbisenc

rm -rf SFML-2.6.2*
SFML_SRC_PATH=
