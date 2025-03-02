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

#ifndef TEXTURE_LOADER_HPP
#define TEXTURE_LOADER_HPP
#include <SFML/Graphics/Texture.hpp>
#include <memory>
#include <filesystem>

namespace Bulletworm {

// Loads the unit textures and compones them to the tile map
class TextureLoader {
public:
    struct Input {
        unsigned int count;     // unit texture count
        unsigned int unitWidth; // tile map element width
        unsigned int width;     // tile width
        unsigned int height;    // tile height
    };
    // if some errors, returns nothing. Move it to the UNIQUE POINTER.
    static std::unique_ptr<sf::Texture> load(const Input& data, const std::filesystem::path* filenames);
};

} // namespace Bulletworm

#endif // !TEXTURE_LOADER_HPP
