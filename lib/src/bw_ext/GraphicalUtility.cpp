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

#include <bw_ext/GraphicalUtility.hpp>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
std::uint32_t scaleColor(float ratio) noexcept {
	return sf::Color(
		std::uint8_t(std::min(255.f * 2 * (1.f - ratio), 255.f)),
		std::uint8_t(std::min(255.f * ratio * 2, 255.f)),
		0).toInteger();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
sf::IntRect createTexRect(int left, int top, int width, int height, unsigned int texSz) noexcept {
	return sf::IntRect(left * texSz, top * texSz, width * texSz, height * texSz);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
sf::IntRect createTexRect(int left, int top, unsigned int texSz) noexcept {
	return createTexRect(left, top, 1, 1, texSz);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
sf::IntRect getTextureUnitRect(int unit, unsigned int texSz, unsigned int texUnitWidth) noexcept {
	int x = unit % texUnitWidth;
	int y = unit / texUnitWidth;
	return createTexRect(x, y, texSz);
}

}
