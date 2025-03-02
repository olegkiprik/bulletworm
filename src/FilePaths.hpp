////////////////////////////////////////////////////////////
//
// Bulletworm - Advanced Snake Game
// Copyright (c) 2024-2025 Oleh Kiprik (oleg.kiprik@proton.me)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
////////////////////////////////////////////////////////////

#ifndef FILE_PATHS_HPP
#define FILE_PATHS_HPP

namespace Bulletworm {

using ResourcePath = const char*;

const ResourcePath CURSOR_PATH = "Resources/Textures/cursor.png";
const ResourcePath ICON_PATH = "Resources/Textures/icon.png";
const ResourcePath DIGITS_PATH = "Resources/Textures/digits.png";

const ResourcePath SOUND_PATH = "Resources/Sounds/";
const ResourcePath MUSIC_PATH = "Resources/Music/";
const ResourcePath TEXTURE_PATH = "Resources/Textures/";
const ResourcePath WALLPAPER_PATH = "Resources/Wallpapers/";
const ResourcePath FONT_PATH = "Resources/Fonts/";
const ResourcePath LANGUAGE_PATH = "Resources/Languages/";
const ResourcePath SHADER_PATH = "Resources/Shaders/";

const ResourcePath MUSIC_LIST_PATH = "Resources/Lists/music.txt";
const ResourcePath SOUND_LIST_PATH = "Resources/Lists/sounds.txt";
const ResourcePath SHADER_LIST_PATH = "Resources/Lists/shaders.txt";
const ResourcePath TEXTURE_LIST_PATH = "Resources/Lists/textures.txt";
const ResourcePath WALLPAPER_LIST_PATH = "Resources/Lists/wallpapers.txt";
const ResourcePath LANGUAGE_LIST_PATH = "Resources/Lists/languages.txt";
const ResourcePath FONT_LIST_PATH = "Resources/Lists/fonts.txt";

const ResourcePath DATA_PATH = "Resources/data.bin";
const ResourcePath STATUS_PATH = "Resources/status.bin";

const ResourcePath LOG_PATH = "logs.log";

}

#endif // FILE_PATHS_HPP