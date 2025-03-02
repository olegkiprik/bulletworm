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

#ifndef CHALLENGE_VISUAL_HPP
#define CHALLENGE_VISUAL_HPP
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <cstdint>
#include <vector>

namespace Bulletworm {

// just the convex shape with partiality
class ChallengeVisual : public sf::Drawable, public sf::Transformable {
public:

    ChallengeVisual(const ChallengeVisual&) = default;
    ChallengeVisual(ChallengeVisual&&) noexcept;

    ChallengeVisual& operator=(const ChallengeVisual&) = default;
    ChallengeVisual& operator=(ChallengeVisual&&) noexcept;

    // creates nothing
    ChallengeVisual() noexcept;

    // creates full
    ChallengeVisual(float radius, std::size_t count);

    // creates partial
    ChallengeVisual(float radius, std::size_t count, std::size_t visibleCount);

    // </ctors>

    // count to view
    void setVisibleCount(std::size_t count) noexcept;

    // how big is the shape
    void setRadius(float radius) noexcept;

    // of all points
    void setCount(std::size_t count);

    // fill color, I don't have the outline :/
    void setColor(std::uint32_t color) noexcept;

    // how big is the shape
    float getRadius() const noexcept {
        return m_radius;
    }

    // all points
    std::size_t getCount() const noexcept {
        return m_count;
    }

    // only visible
    std::size_t getVisibleCount() const noexcept {
        return m_visibleCount;
    }

    // fill color
    std::uint32_t getColor() const noexcept {
        return m_color;
    }

private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:

    // everything
    void update();

    // color only
    void updateColor() noexcept;

    std::vector<sf::Vertex> m_vertices;
    std::size_t m_visibleCount;
    std::size_t m_count;
    std::uint32_t m_color;
    float m_radius;
};

}

#endif // CHALLENGE_VISUAL_HPP
