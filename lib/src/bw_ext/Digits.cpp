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

#include <bw_ext/Digits.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <cassert>

namespace Bulletworm {

Digits::Digits() noexcept :
    m_vertices(),
    m_zeroRect(),
    m_count(0),
    m_texture(nullptr),
    m_isTextureVertical(false),
    m_isNumberVertical(false) {}

Digits::Digits(const sf::Texture& texture) noexcept :
    m_vertices(),
    m_zeroRect(),
    m_count(0),
    m_texture(&texture),
    m_isTextureVertical(false),
    m_isNumberVertical(false) {}

Digits::Digits(const sf::Texture& texture, const sf::IntRect& zeroRect, std::size_t count) :
    m_vertices(count * 6),
    m_zeroRect(zeroRect),
    m_count(),
    m_texture(&texture),
    m_isTextureVertical(false),
    m_isNumberVertical(false) {
    setDigitCount(count);
}

void Digits::setTexture(const sf::Texture& texture) noexcept {
    m_texture = &texture;
}
const sf::Texture* Digits::getTexture() const noexcept {
    return m_texture;
}
void Digits::setZeroDigitRect(const sf::IntRect& rect) noexcept {
    sf::IntRect currentRect = rect;

    for (std::size_t i = 0; i < m_count; ++i) {
        m_vertices[i * 6 + 0].position = (sf::Vector2f)sf::Vector2i(currentRect.left, currentRect.top);
        m_vertices[i * 6 + 1].position = (sf::Vector2f)sf::Vector2i(currentRect.left + currentRect.width, currentRect.top);
        m_vertices[i * 6 + 2].position = (sf::Vector2f)sf::Vector2i(currentRect.left + currentRect.width, currentRect.top + currentRect.height);
        m_vertices[i * 6 + 3].position = (sf::Vector2f)sf::Vector2i(currentRect.left + currentRect.width, currentRect.top + currentRect.height);
        m_vertices[i * 6 + 4].position = (sf::Vector2f)sf::Vector2i(currentRect.left, currentRect.top + currentRect.height);
        m_vertices[i * 6 + 5].position = (sf::Vector2f)sf::Vector2i(currentRect.left, currentRect.top);

        if (m_isNumberVertical)
            currentRect.top += currentRect.height;
        else
            currentRect.left += currentRect.width;
    }

    m_zeroRect = rect;
}
const sf::IntRect& Digits::getZeroDigitRect() const noexcept {
    return m_zeroRect;
}
bool Digits::isNumberVertical() const noexcept {
    return m_isNumberVertical;
}
void Digits::setNumberVerticalEnabled(bool enabled) noexcept {
    m_isNumberVertical = enabled;
    setZeroDigitRect(m_zeroRect);
}
bool Digits::isTextureVertical() const noexcept {
    return m_isTextureVertical;
}
void Digits::setTextureVerticalEnabled(bool enabled) noexcept {
    m_isTextureVertical = enabled;
}
std::size_t Digits::getDigitCount() const noexcept {
    return m_count;
}
void Digits::setDigitCount(std::size_t count) {
    m_count = count;
    m_vertices.resize(count * 6);
    setZeroDigitRect(m_zeroRect);
}
void Digits::setNumber(std::uintmax_t number, std::uintmax_t system) noexcept {
    assert(system > 1);

    for (std::size_t i = 0; i < m_count; ++i) {
        std::size_t revi = m_count - 1 - i;
        std::size_t modulo{ static_cast<std::size_t>(number % system) };
        number /= system;
        setDigit(revi, modulo);
    }
}
sf::FloatRect Digits::getLocalBounds() const noexcept {
    sf::IntRect rect = m_zeroRect;
    if (m_isNumberVertical)
        rect.height *= (int)m_count;
    else
        rect.width *= (int)m_count;
    return sf::FloatRect(rect);
}
sf::FloatRect Digits::getGlobalBounds() const noexcept {
    return getTransform().transformRect(getLocalBounds());
}
void Digits::setColor(std::uint32_t color, std::size_t digitIndex) noexcept {
    for (std::size_t i = 0; i < 6; ++i)
        m_vertices[digitIndex * 6 + i].color = (sf::Color)color;
}
std::uint32_t Digits::getColor(std::size_t digitIndex) const noexcept {
    assert(digitIndex * 6 < m_vertices.size());
    return m_vertices[digitIndex * 6].color.toInteger();
}
void Digits::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    states.texture = m_texture;
    target.draw(m_vertices.data(), m_vertices.size(), sf::Triangles, states);
}
void Digits::setDigit(std::size_t index, std::size_t digit) noexcept {
    sf::IntRect digitRect = m_zeroRect;
    if (m_isTextureVertical)
        digitRect.top += digitRect.height * (int)digit;
    else
        digitRect.left += digitRect.width * (int)digit;

    m_vertices[index * 6 + 0].texCoords = (sf::Vector2f)sf::Vector2i(digitRect.left, digitRect.top);
    m_vertices[index * 6 + 1].texCoords = (sf::Vector2f)sf::Vector2i(digitRect.left + digitRect.width, digitRect.top);
    m_vertices[index * 6 + 2].texCoords = (sf::Vector2f)sf::Vector2i(digitRect.left + digitRect.width, digitRect.top + digitRect.height);
    m_vertices[index * 6 + 3].texCoords = (sf::Vector2f)sf::Vector2i(digitRect.left + digitRect.width, digitRect.top + digitRect.height);
    m_vertices[index * 6 + 4].texCoords = (sf::Vector2f)sf::Vector2i(digitRect.left, digitRect.top + digitRect.height);
    m_vertices[index * 6 + 5].texCoords = (sf::Vector2f)sf::Vector2i(digitRect.left, digitRect.top);
}
}
