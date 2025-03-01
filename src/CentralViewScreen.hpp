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



#ifndef CENTRAL_VIEW_SCREEN_HPP
#define CENTRAL_VIEW_SCREEN_HPP
#include <bw_ext/SpriteArray.hpp>
#include <bw_ext/SnakeDrawable.hpp>
#include "engine/const/EatableItem.hpp"
#include "GraphicalEnums.hpp"
#include <bw_ext/ParticleSystem.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <array>

namespace Bulletworm {

enum class TextureUnit;
enum class Orientation;
enum class Direction;
enum class PowerupType;




// just draw it
class CentraViewScreen {
public:

    CentraViewScreen();

    // with edges (constant!)
    [[nodiscard]] bool init(unsigned int texSz,
                            unsigned int texUnitWidth,
                            const sf::Vector2i& thesize,
                            const sf::Texture& texture,
                            std::uint32_t foggColor);

    void pushBgObj(const sf::Vector2i& position, 
                   TextureUnit unit, std::uint32_t theme,
                   Orientation orientation);
    void pushFgObj(const sf::Vector2i& position, 
                   TextureUnit unit, std::uint32_t theme,
                   Orientation orientation);

    void pushFruit(const sf::Vector2i& position, 
                   Direction tailing,
                   const sf::Vector2i& innerViewSize);
    void pushBonus(const sf::Vector2i& position, 
                   Direction tailing,
                   const sf::Vector2i& innerViewSize);
    void pushPowerup(PowerupType powerup,
                     const sf::Vector2i& position, 
                     Direction tailing,
                     const sf::Vector2i& innerViewSize);
    void pushUnknownPowerup(const sf::Vector2i& position,
                            Direction tailing, 
                            const sf::Vector2i& innerViewSize);

    [[nodiscard]] bool updateVBs();
    void clear() noexcept;

    const SpriteArray& getItemArray(EatableItem item) const noexcept;
    const SpriteArray& getScreenItemArray(EatableItem item,
                                          ScreenMode mode) const noexcept;

    // to draw

    const sf::RectangleShape& getFogg() const noexcept {
        return fogg;
    }

    const sf::VertexBuffer& getVbScreens() const noexcept {
        return vbscreens;
    }

    const sf::VertexBuffer& getvbbackgroundObjects() const noexcept {
        return vbbackgroundObjects;
    }

    const sf::VertexBuffer& getvbforegroundObjects() const noexcept {
        return vbforegroundObjects;
    }

    const SnakeDrawable& getSnakeDrawable() const noexcept {
        return snakeDrawable;
    }

    void push2snakeDrawable(const sf::Vector2i& position,
                            Direction ptdentry,
                            Direction ptdexit,
                            std::uint32_t snakeFillCol,
                            std::uint32_t snakeOutlineCol) {
        snakeDrawable.push(position, ptdentry, ptdexit,
                           m_texSz, snakeFillCol, snakeOutlineCol);
    }

    std::size_t getVbvxcountfg() const noexcept {
        return vbvxcounttodrawfg;
    }

    std::size_t getVbvxcountbg() const noexcept {
        return vbvxcounttodrawbg;
    }

    void setupThemes(std::uint32_t screen, std::uint32_t fruit,
                     std::uint32_t bonus, std::uint32_t superbonus) {
        m_fruitTheme = fruit;
        m_bonusTheme = bonus;
        m_screenTheme = screen;
        m_superbonusTheme = superbonus;
    }

private:

    // Screen modes are 'wider'
    std::array<SpriteArray,
        ScreenModeCount* ItemCount> screenItems;

    sf::RectangleShape fogg;

    std::array<SpriteArray, ItemCount> items;

    SpriteArray vabackgroundObjects;
    SpriteArray vaforegroundObjects;

    sf::VertexBuffer vbbackgroundObjects;
    sf::VertexBuffer vbforegroundObjects;
    sf::VertexBuffer vbscreens;

    SnakeDrawable snakeDrawable;

    // mess
    std::size_t vbvxcounttodrawfg = 0;
    std::size_t vbvxcounttodrawbg = 0;

    std::uint32_t m_screenTheme = 0;
    std::uint32_t m_fruitTheme = 0;
    std::uint32_t m_bonusTheme = 0;
    std::uint32_t m_superbonusTheme = 0;

    unsigned int m_texSz = 0;
    unsigned int m_texUnitWidth = 0;

    SpriteArray& useItemArray(EatableItem item) noexcept;
    SpriteArray& useScreenItemArray(EatableItem item,
                                    ScreenMode mode) noexcept;

    static sf::Transform getTransFromOrient(Orientation orientation,
                                            unsigned texSz) noexcept;

    void setTexture(const sf::Texture& texture) noexcept;

    void pushItem(const sf::Vector2i& position, Direction tailing,
                  const sf::Vector2i& innerViewSize, EatableItem item, 
                  TextureUnit unit, std::uint32_t theme);
};

}

#endif // !CENTRAL_VIEW_SCREEN_HPP