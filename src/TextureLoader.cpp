////////////////////////////////////////////////////////////
//
// Bulletworm - Advanced Snake Game
// Copyright (c) 2024 Oleh Kiprik (oleg.kiprik@proton.me)
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

#include "TextureLoader.hpp"
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <cassert>
#include <climits>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<sf::Texture> TextureLoader::load(const Input& data, const std::filesystem::path* filenames) {
    
    // Requirements
    assert(data.count > 0);
    assert(data.height > 0);
    assert(data.width > 0);
    assert(data.unitWidth > 0);
    assert(data.width <= INT_MAX);
    assert(data.height <= INT_MAX);
    assert(data.count != UINT_MAX);
    assert(UINT_MAX / data.width >= data.unitWidth);
    assert(UINT_MAX - data.count + 1 >= data.unitWidth);
    assert(UINT_MAX / data.height >= (data.count - 1 + data.unitWidth) / data.unitWidth);

    unsigned texWidth{ data.width * data.unitWidth };
    unsigned texHeight{ data.height * ((data.count - 1 + data.unitWidth) / data.unitWidth) };
    sf::RenderTexture renderTexture;
    if (!renderTexture.create(texWidth, texHeight)) {
        return {};
    }
    renderTexture.clear();
    sf::Image imageForPreproc;
    sf::Texture unitTex;
    if (!unitTex.create(data.width, data.height)) {
        return {};
    }
    sf::Sprite sprite(unitTex, sf::IntRect(0, 0, data.width, data.height));
    for (unsigned i = 0; i < data.count; ++i) {
        if (!imageForPreproc.loadFromFile(filenames[i].string())) {
            return {};
        }
        unitTex.update(imageForPreproc);
        unsigned x{ i % data.unitWidth };
        unsigned y{ i / data.unitWidth };
        sprite.setPosition((float)(x * data.width), (float)(y * data.height));
        renderTexture.draw(sprite, sf::BlendNone);
    }
    renderTexture.display();
    return std::make_unique<sf::Texture>(renderTexture.getTexture());
}

} // namespace Bulletworm
