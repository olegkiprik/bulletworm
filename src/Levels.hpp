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

#ifndef LEVELS_HPP
#define LEVELS_HPP
#include "engine/const/EatableItem.hpp"
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <cstdint>
#include <array>

namespace {
    template<class T>
    constexpr unsigned int fwkBitCeilLogLvl(T num) noexcept {
        unsigned int nr = 0;
        while (num) {
            num >>= 1;
            ++nr;
        }
        return nr;
    }

    template<class T, class U>
    constexpr T fwkGetRealSizeLvl(U size) noexcept {
        return 1 + (size ? (T(1) << fwkBitCeilLogLvl(size - 1)) : T(0));
    }
}

namespace sf {
class InputStream;
}

namespace Bulletworm {

enum class LevelCountMap;
enum class EatableItem;

class Levels {
public:

    [[nodiscard]] bool loadFromStream(unsigned int diffCount,
                                      unsigned int levelCount, sf::InputStream& stream,
                                      bool endiannessRequired);

    unsigned int getDifficultyCount() const noexcept {
        return m_diffCount;
    }
    unsigned int getLevelCount() const noexcept {
        return m_levelCount;
    }

    const std::uint32_t* getLevelAttribPtr(unsigned int diffIndex, 
                                           unsigned int levelIndex) const noexcept;
    const std::uint32_t* getLevelPlotDataPtr(unsigned int diffIndex,
                                             unsigned int levelIndex) const noexcept;
    const std::uint32_t* getEffectDurationPtr(unsigned int diffIndex, 
                                              unsigned int levelIndex) const noexcept;
    const std::array<std::uintmax_t, fwkGetRealSizeLvl<std::size_t,int>(PowerupCount)>&
        getPowerupProbs(unsigned int diffIndex, unsigned int levelIndex) const noexcept;
    const sf::Vector2u& getMapSize(unsigned int diffIndex,
                                   unsigned int levelIndex) const noexcept;

    const std::uint32_t*
        getLevelCountMap(LevelCountMap what, unsigned int diffIndex,
                         unsigned int levelIndex) const noexcept;
    const std::uint32_t*
        getItemProbCountMap(EatableItem what, unsigned int diffIndex, 
                            unsigned int levelIndex) const noexcept;

private:

    std::vector<std::uint32_t> m_levelAttributes;
    std::vector<std::uint32_t> m_levelPlotData;
    std::vector<std::uint32_t> m_effectDurations;
    
    std::vector<std::array<std::uintmax_t,
        fwkGetRealSizeLvl<std::size_t, int>(PowerupCount)>> m_powerupProbs;

    std::vector<sf::Vector2u> m_mapSizes;
    std::vector<std::vector<std::uint32_t>> m_levelCountMaps;
    std::vector<std::vector<std::uint32_t>> m_itemProbCountMaps;

    unsigned int m_diffCount = 0;
    unsigned int m_levelCount = 0;
};

}

#endif // !LEVELS_HPP
