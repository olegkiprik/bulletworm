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

#ifndef SNAKE_WORLD_HPP
#define SNAKE_WORLD_HPP
#include <SFML/System/Vector2.hpp>
#include <bw_ext/BasicUtility.hpp>
#include "const/EatableItem.hpp"
#include <bw_ext/const/ObjectParameterEnums.hpp>
#include <bw_ext/Map.hpp>
#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <list>

namespace Bulletworm {
class Randomizer;
enum class PowerupType;
enum class Direction;
enum class EatableItem;

// Physical Snake world (really simple)
class SnakeWorld {
public:

    struct TailDirection {
        Direction tdentry; // |>   |
        Direction tdexit;  // |   >|
    };

    struct Vector2iHash {
        union Union {
            int val;
            std::size_t h;
        };
        std::size_t operator()(const sf::Vector2i& keyval) const noexcept {
            if constexpr (sizeof(int) * 2 == sizeof(std::size_t)) {
                Union un{};
                un.val = keyval.x;
                un.h <<= 32;
                std::size_t h = un.h;
                un.val = keyval.y;
                h |= un.h;
                return h;
            }
            return
                std::hash<int>()(keyval.x) ^
                std::hash<int>()(-keyval.y);
        }
    };

    using ItemSet = std::unordered_set<sf::Vector2i, Vector2iHash>;
    using PowerupMap = std::unordered_map<sf::Vector2i, PowerupType, Vector2iHash>;
    
    using TailIdSubList = std::list<std::pair<std::uintmax_t, TailDirection>>;
    using TailIdVector = std::vector<TailIdSubList>;
    using TailIdMap = std::unordered_map<sf::Vector2i, TailIdSubList, Vector2iHash>;

    SnakeWorld() noexcept;

    SnakeWorld(const SnakeWorld&) = default;
    SnakeWorld(SnakeWorld&&) noexcept;

    SnakeWorld& operator=(const SnakeWorld&) = default;
    SnakeWorld& operator=(SnakeWorld&&) noexcept;

    // create the world
    SnakeWorld(const Map<std::uint32_t>* const* initItemProbArr, const sf::Vector2i& snakePosition);
    void restart(const Map<std::uint32_t>* const* initItemProbArr, const sf::Vector2i& snakePosition);
    void restart(const sf::Vector2i& snakePosition) noexcept;

    // if opposite, it will be just ignored
    // can return some of these subevents: FruitEaten, BonusEaten, PowerupEaten
    // Don't forget to use trimTail
    std::uintmax_t moveSnake(Direction direction);
    void trimTail() noexcept;

    void placeFruit(Randomizer& positionRandomizer);
    void placeBonus(Randomizer& positionRandomizer);
    void placePowerup(Randomizer& positionRandomizer, PowerupType certainPowerup);

    void removeItem(const sf::Vector2i& position);
    void clearBonuses() noexcept;
    void clearPowerups() noexcept;

    const sf::Vector2i& getCurrentSnakePosition() const noexcept {
        return m_snakePosition;
    }

    std::uint32_t getCurrentRelativeItemAcquireProb(EatableItem item, int x, int y) const noexcept;
    std::uint32_t getCurrentRelativeItemAcquireProb(EatableItem item, const sf::Vector2i& pos) const noexcept;

    std::uint32_t getInitialRelativeItemAcquireProb(EatableItem item, int x, int y) const noexcept;
    std::uint32_t getInitialRelativeItemAcquireProb(EatableItem item, const sf::Vector2i& pos) const noexcept;

    const sf::Vector2u& getMapSize() const noexcept;
    std::uintmax_t getTailSize() const noexcept;
    Direction getPreviousDirection() const noexcept;

    const ItemSet& getFruitPositions() const noexcept {
        return m_fruitPositions;
    }
    const ItemSet& getBonusPositions() const noexcept {
        return m_bonusPositions;
    }
    const PowerupMap& getPowerups()       const noexcept {
        return m_powerupPositions;
    }
    const sf::Vector2i& getBackPosition()   const noexcept {
        return m_backPosition;
    }
    const TailIdSubList& getTailIDs(const sf::Vector2i& position) const noexcept;

    std::uintmax_t       getStepCount()      const noexcept {
        return m_stepCount;
    }

private:

    // technically two similar functions but one is with noexcept
    void createItemProbs();
    void resetItemProbs() noexcept;
    void postInit(const sf::Vector2i& snakePosition) noexcept;

    /// Change the access of the map position.
    /// The access of position means the status whether the item acquire there or not.
    void setAccess(int x, int y, EatableItem item, std::uint32_t access) noexcept;

    void openAccess(int x, int y) noexcept;
    void closeAccess(int x, int y) noexcept;

    /// Get the random free position for acquiring item.
    [[nodiscard]] sf::Vector2i getAvailablePosition(EatableItem item, Randomizer& randomizer) const;

    // ALIASES
    void setAccess(const sf::Vector2i& position, EatableItem item, std::uint32_t access) noexcept;
    void openAccess(const sf::Vector2i& position) noexcept;
    void closeAccess(const sf::Vector2i& position) noexcept;

    sf::Vector2i getNeckPosition() const noexcept;

    ////////////////////////////////////////////////////////////
    /// Member data
    ////////////////////////////////////////////////////////////

    class TaidIdContainer {
    public:

        void reset(const sf::Vector2u& newsize, bool enable_map);
        void reset(bool enable_map) noexcept;

        const TailIdSubList& getList(const sf::Vector2i& position) const noexcept;

        TailIdSubList& setList(const sf::Vector2i& position) ;

    private:

        
        TailIdMap map;
        TailIdVector vector;
        sf::Vector2u size;
        bool is_map = true;
        static const TailIdSubList VirtualSublist;
    };

    TaidIdContainer m_tailIDs; // Tail IDs
    std::array<std::vector<std::uintmax_t>, ItemCount> m_itemProbabilities; 
    // For placing fruits, bonuses, powerups
    
    ItemSet m_fruitPositions; // Fruit position on the map
    ItemSet m_bonusPositions; // Bonus position on the map
    PowerupMap m_powerupPositions; // Powerup position on the map
    std::array<const Map<std::uint32_t>*, ItemCount> m_initItemProbabilities; // Dependencies
    std::uintmax_t m_stepCount = 0; // Total step count
    sf::Vector2i m_snakePosition; // Snake's head position on the map       
    sf::Vector2i m_backPosition; // Opens item access
    Direction m_previousSnakeDirection = Direction::Count; 
    // Previous snake direction (for rotate command processing)
};

} // namespace Bulletworm

#endif // !SNAKE_WORLD_HPP
