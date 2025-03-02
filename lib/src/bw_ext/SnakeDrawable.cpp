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

#include <bw_ext/SnakeDrawable.hpp>
#include <bw_ext/const/ObjectParameterEnums.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <cmath>

namespace Bulletworm {

void SnakeDrawable::clear() noexcept {
    m_vertices.clear();
}

void SnakeDrawable::push(const sf::Vector2i& position, Direction ptdentry, Direction ptdexit, unsigned int texSz,
                         std::uint32_t snakeFillColor, std::uint32_t snakeOutlineColor) {
    constexpr std::size_t CirclePrecision = 30;
    constexpr float pi = 3.141592654f;
    constexpr unsigned outlineRatioNumerator = 1;
    constexpr unsigned outlineRatioDenominator = 10;

    constexpr float outlineRatioNum1 = float(outlineRatioNumerator + outlineRatioDenominator);

    float radius = float(texSz) / 4;

    float radius_gr = float(texSz) * 3 * 2 / 4 / 4;
    float radius_ls = float(texSz) * 1 * 2 / 4 / 4;

    std::swap(radius_gr, radius_ls);

    m_vertices.resize(m_vertices.size() + CirclePrecision * 18);
    std::size_t vsize = m_vertices.size();

    sf::Vector2f firstCenterPos;
    sf::Vector2f secondCenterPos;

    sf::Vector2i biasedPos = position;
    ++biasedPos.x;
    ++biasedPos.y;

    switch (ptdentry) {
    case Direction::Up:
        firstCenterPos.x = float(biasedPos.x * texSz * 2 + texSz) / 2;
        firstCenterPos.y = float(biasedPos.y * texSz * 4 + texSz * 3) / 4;
        break;
    case Direction::Down:
        firstCenterPos.x = float(biasedPos.x * texSz * 2 + texSz) / 2;
        firstCenterPos.y = float(biasedPos.y * texSz * 4 + texSz) / 4;
        break;
    case Direction::Left:
        firstCenterPos.y = float(biasedPos.y * texSz * 2 + texSz) / 2;
        firstCenterPos.x = float(biasedPos.x * texSz * 4 + texSz * 3) / 4;
        break;
    case Direction::Right:
        firstCenterPos.y = float(biasedPos.y * texSz * 2 + texSz) / 2;
        firstCenterPos.x = float(biasedPos.x * texSz * 4 + texSz) / 4;
        break;
    default:
        break;
    }

    switch (ptdexit) {
    case Direction::Up:
        secondCenterPos.x = float(biasedPos.x * texSz * 2 + texSz) / 2;
        secondCenterPos.y = float(biasedPos.y * texSz * 4 + texSz) / 4;
        break;
    case Direction::Down:
        secondCenterPos.x = float(biasedPos.x * texSz * 2 + texSz) / 2;
        secondCenterPos.y = float(biasedPos.y * texSz * 4 + texSz * 3) / 4;
        break;
    case Direction::Left:
        secondCenterPos.y = float(biasedPos.y * texSz * 2 + texSz) / 2;
        secondCenterPos.x = float(biasedPos.x * texSz * 4 + texSz) / 4;
        break;
    case Direction::Right:
        secondCenterPos.y = float(biasedPos.y * texSz * 2 + texSz) / 2;
        secondCenterPos.x = float(biasedPos.x * texSz * 4 + texSz * 3) / 4;
        break;
    default:
        break;
    }

    for (std::size_t i = 0; i < CirclePrecision; ++i) {
        m_vertices[vsize - CirclePrecision * 18 + i * 3].position = firstCenterPos;
        m_vertices[vsize - CirclePrecision * 9 + i * 3].position = secondCenterPos;
    }

    for (std::size_t i = 0; i < CirclePrecision; ++i) {
        float angle = pi * 2 * i / CirclePrecision;
        float x = std::cos(angle) * radius; // getCurrentRadius(angle + pi / 2, radius_gr, radius_ls);
        float y = std::sin(angle) * radius; // getCurrentRadius(angle + pi / 2, radius_gr, radius_ls);
        sf::Vector2f pos(x, y);

        m_vertices[vsize - CirclePrecision * 18 + i * 3 + 1].position =
            firstCenterPos + pos;
        m_vertices[vsize - CirclePrecision * 9 + i * 3 + 1].position =
            secondCenterPos + pos;

        // outline
        m_vertices[vsize - CirclePrecision * 15 + i * 6 + 0].position =
            firstCenterPos + pos;
        m_vertices[vsize - CirclePrecision * 15 + i * 6 + 1].position =
            firstCenterPos + pos * outlineRatioNum1 / float(outlineRatioDenominator);
        m_vertices[vsize - CirclePrecision * 15 + i * 6 + 5].position =
            firstCenterPos + pos;

        m_vertices[vsize - CirclePrecision * 6 + i * 6 + 0].position =
            secondCenterPos + pos;
        m_vertices[vsize - CirclePrecision * 6 + i * 6 + 1].position =
            secondCenterPos + pos * outlineRatioNum1 / float(outlineRatioDenominator);
        m_vertices[vsize - CirclePrecision * 6 + i * 6 + 5].position =
            secondCenterPos + pos;
    }

    for (std::size_t i = 0; i < CirclePrecision; ++i) {
        float angle = pi * 2 * (i + 1) / CirclePrecision;
        float x = std::cos(angle) * radius; // getCurrentRadius(angle + pi / 2, radius_gr, radius_ls);
        float y = std::sin(angle) * radius; // getCurrentRadius(angle + pi / 2, radius_gr, radius_ls);
        sf::Vector2f pos(x, y);

        m_vertices[vsize - CirclePrecision * 18 + i * 3 + 2].position =
            firstCenterPos + pos;
        m_vertices[vsize - CirclePrecision * 9 + i * 3 + 2].position =
            secondCenterPos + pos;

        // outline
        m_vertices[vsize - CirclePrecision * 15 + i * 6 + 2].position =
            firstCenterPos + pos * outlineRatioNum1 / float(outlineRatioDenominator);
        m_vertices[vsize - CirclePrecision * 15 + i * 6 + 3].position =
            firstCenterPos + pos * outlineRatioNum1 / float(outlineRatioDenominator);
        m_vertices[vsize - CirclePrecision * 15 + i * 6 + 4].position =
            firstCenterPos + pos;

        m_vertices[vsize - CirclePrecision * 6 + i * 6 + 2].position =
            secondCenterPos + pos * outlineRatioNum1 / float(outlineRatioDenominator);
        m_vertices[vsize - CirclePrecision * 6 + i * 6 + 3].position =
            secondCenterPos + pos * outlineRatioNum1 / float(outlineRatioDenominator);
        m_vertices[vsize - CirclePrecision * 6 + i * 6 + 4].position =
            secondCenterPos + pos;
    }

    for (std::size_t i = 0; i < CirclePrecision * 3; ++i) {
        m_vertices[vsize - CirclePrecision * 18 + i].color = sf::Color(snakeFillColor);
        m_vertices[vsize - CirclePrecision * 9 + i].color = sf::Color(snakeFillColor);
    }

    for (std::size_t i = 0; i < CirclePrecision * 6; ++i) {
        m_vertices[vsize - CirclePrecision * 15 + i].color = sf::Color(snakeOutlineColor);
        m_vertices[vsize - CirclePrecision * 6 + i].color = sf::Color(snakeOutlineColor);
    }
}

void SnakeDrawable::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(m_vertices.data(), m_vertices.size(), sf::Triangles, states);
}

}
