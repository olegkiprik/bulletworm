#!/bin/sh

# export SFML_SRC_PATH first
g++ \
-pipe \
-std=c++17 \
-s \
-DNDEBUG \
-O2 \
-march=native \
-fno-builtin \
-fwhole-program \
-flto \
-ffast-math \
src/*.c* \
src/engine/*.c* \
lib/src/bw_ext/*.c* \
lib/src/bw_ext/random/*.c* \
lib/src/bw_ext/stream/*.c* \
$SFML_SRC_PATH/SFML-2.6.1/src/SFML/Audio/*.c* \
$SFML_SRC_PATH/SFML-2.6.1/src/SFML/Graphics/*.c* \
$SFML_SRC_PATH/SFML-2.6.1/src/SFML/Window/*.c* \
$SFML_SRC_PATH/SFML-2.6.1/src/SFML/System/*.c* \
$SFML_SRC_PATH/SFML-2.6.1/src/SFML/System/Unix/*.c* \
$SFML_SRC_PATH/SFML-2.6.1/src/SFML/Window/Unix/*.c* \
-I lib/include/ \
-I $SFML_SRC_PATH/SFML-2.6.1/include/ \
-I $SFML_SRC_PATH/SFML-2.6.1/src/ \
-I $SFML_SRC_PATH/SFML-2.6.1/extlibs/headers/AL \
-I $SFML_SRC_PATH/SFML-2.6.1/extlibs/headers/glad/include/ \
-I $SFML_SRC_PATH/SFML-2.6.1/extlibs/headers/minimp3/ \
-I $SFML_SRC_PATH/SFML-2.6.1/extlibs/headers/freetype2/ \
-I $SFML_SRC_PATH/SFML-2.6.1/extlibs/headers/stb_image/ \
-I $SFML_SRC_PATH/SFML-2.6.1/extlibs/headers/vulkan/ \
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
