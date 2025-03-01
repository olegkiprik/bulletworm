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

#include <bw_ext/ChallengeVisual.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <cmath>
#include <cassert>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
ChallengeVisual::ChallengeVisual(ChallengeVisual&& src) noexcept :
    m_vertices(std::move(src.m_vertices)),
    m_visibleCount(src.m_visibleCount),
    m_count(src.m_count),
    m_color(src.m_color),
    m_radius(src.m_radius) {
    src.m_color = sf::Color(255, 255, 255).toInteger();
    src.m_count = 0;
    src.m_visibleCount = 0;
    src.m_radius = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
ChallengeVisual& ChallengeVisual::operator=(ChallengeVisual&& src) noexcept {
    if (this == &src)
        return *this;
    m_vertices = std::move(src.m_vertices);
    m_color = src.m_color;
    m_radius = src.m_radius;
    m_count = src.m_count;
    m_visibleCount = src.m_visibleCount;
    src.m_color = sf::Color(255, 255, 255).toInteger();
    src.m_count = 0;
    src.m_radius = 0;
    src.m_visibleCount = 0;
    return *this;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
ChallengeVisual::ChallengeVisual() noexcept :
    m_vertices(),
    m_visibleCount(0),
    m_count(0),
    m_color(sf::Color(255, 255, 255).toInteger()),
    m_radius(0)
{}


////////////////////////////////////////////////////////////////////////////////////////////////////
ChallengeVisual::ChallengeVisual(float radius, std::size_t count) :
    ChallengeVisual(radius, count, count) {}


////////////////////////////////////////////////////////////////////////////////////////////////////
ChallengeVisual::ChallengeVisual(float radius, std::size_t count, std::size_t visibleCount) :
    m_vertices(),
    m_visibleCount(visibleCount),
    m_count(count),
    m_color(sf::Color(255, 255, 255).toInteger()),
    m_radius(radius) {
    assert(count >= 3);
    assert(visibleCount <= count);
    update();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void ChallengeVisual::setVisibleCount(std::size_t count) noexcept {
    assert(count <= m_count);
    m_visibleCount = count;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void ChallengeVisual::setRadius(float radius) noexcept {
    assert(radius >= 0);
    m_radius = radius;
    if (m_count)
        update();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void ChallengeVisual::setCount(std::size_t count) {
    assert(count >= 3);
    if (m_count != count) {
        m_count = count;
        m_visibleCount = std::min(m_visibleCount, m_count);
        update();
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void ChallengeVisual::setColor(std::uint32_t color) noexcept {
    m_color = color;
    updateColor();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void ChallengeVisual::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (m_visibleCount) {
        states.texture = nullptr;
        states.transform *= getTransform();
        target.draw(m_vertices.data(), m_visibleCount + 2, sf::TriangleFan, states);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void ChallengeVisual::update() {
    assert(m_count >= 3);
    m_vertices.resize(m_count + 2);
    m_vertices[0].position = sf::Vector2f(m_radius, m_radius);

    static constexpr float pi = 3.141592654f;

    for (std::size_t i = 0; i <= m_count; ++i) {
        float angle = i * 2 * pi / m_count - pi / 2;
        float x = std::cos(angle) * m_radius;
        float y = std::sin(angle) * m_radius;
        m_vertices[i + 1].position = sf::Vector2f(m_radius + x, m_radius + y);
    }

    updateColor();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void ChallengeVisual::updateColor() noexcept {
    std::for_each(m_vertices.begin(), m_vertices.end(),
                  [this](sf::Vertex& vx) {
                      vx.color = (sf::Color)m_color;
                  });
}

}
