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

#ifndef DIGITS_HPP
#define DIGITS_HPP
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <vector>
#include <cstdint>

namespace Bulletworm {
class Digits : public sf::Drawable, public sf::Transformable {
public:

    Digits() noexcept;

    explicit Digits(const sf::Texture& texture) noexcept;

    Digits(const sf::Texture& texture, const sf::IntRect& zeroRect, std::size_t count);

    void setTexture(const sf::Texture& texture) noexcept;
    const sf::Texture* getTexture() const noexcept;

    void setZeroDigitRect(const sf::IntRect& rect) noexcept;
    const sf::IntRect& getZeroDigitRect() const noexcept;

    bool isNumberVertical() const noexcept;
    void setNumberVerticalEnabled(bool enabled = true) noexcept;

    bool isTextureVertical() const noexcept;
    void setTextureVerticalEnabled(bool enabled = true) noexcept;

    std::size_t getDigitCount() const noexcept;
    void setDigitCount(std::size_t count);

    void setNumber(std::uintmax_t number, std::uintmax_t system = 10) noexcept;

    sf::FloatRect getLocalBounds() const noexcept;
    sf::FloatRect getGlobalBounds() const noexcept;

    void setColor(std::uint32_t color, std::size_t digitIndex) noexcept;
    std::uint32_t getColor(std::size_t digitIndex) const noexcept;

private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:

    void setDigit(std::size_t index, std::size_t digit) noexcept;

    std::vector<sf::Vertex> m_vertices;
    sf::IntRect m_zeroRect;
    std::size_t m_count;
    const sf::Texture* m_texture;
    bool m_isTextureVertical;
    bool m_isNumberVertical;
};
}

#endif // !DIGITS_HPP
