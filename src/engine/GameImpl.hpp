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

#ifndef GAME_IMPL_HPP
#define GAME_IMPL_HPP
#include "SnakeWorld.hpp"
#include "const/MiscEnum.hpp"

/// Note that Snake has the factual direction when the snake
/// has the tail. You can get it by getSnakeShape method.
/// Don't confuse two directions. 
/// The formal direction means the direction where the snake would move if
/// the according event was activated. It can be changed by the rotors,
/// so don't rely on it when you predict the Snake's behavior.

namespace {
    template<class T>
    constexpr unsigned int fwkBitCeilLog(T num) noexcept {
        unsigned int nr = 0;
        while (num) {
            num >>= 1;
            ++nr;
        }
        return nr;
    }

    template<class T, class U>
    constexpr T fwkGetRealSize(U size) noexcept {
        return 1 + (size ? (T(1) << fwkBitCeilLog(size - 1)) : T(0));
    }
}

namespace Bulletworm {

enum class LevelAttribEnum;
class Randomizer;
enum class ObjectEffect;
class ObjectBehavior;

class GameImpl {

public:

    struct LevelPointers {
        // single

        const std::array<std::uintmax_t, fwkGetRealSize<std::size_t, int>(PowerupCount)>* powerupProbs = nullptr;
        const std::vector<std::uintmax_t>* snakePositionProbs = nullptr;

        // arrays

        const ObjectBehavior* objectBehs = nullptr;
        const std::uint32_t* preEffectBehIndices = nullptr;
        const std::uint32_t* postEffectBehIndices = nullptr;
        const std::uint32_t* tailCapacities1 = nullptr;

        const std::uint32_t* objectPairIndices = nullptr;
        const std::uint32_t* objectParams = nullptr;
        const std::uint32_t* effectDurations = nullptr;
        const std::uint32_t* attribArray = nullptr;
    };

    GameImpl(const GameImpl&) = default;
    GameImpl(GameImpl&&) noexcept;

    GameImpl& operator=(const GameImpl&) = default;
    GameImpl& operator=(GameImpl&&) noexcept;

    // Start it then
    GameImpl() noexcept; // TODO

    // they restart
    GameImpl(const LevelPointers& ptrs,
             Randomizer* const* randomizers,
             const std::uint32_t* objectMemory,
             Map<std::uint32_t> const* const* itemProbs);

    void reset(const LevelPointers& ptrs,
               Randomizer* const* randomizers,
               const std::uint32_t* objectMemory,
               Map<std::uint32_t> const* const* itemProbs);

           // Controlling

    void restart(const std::uint32_t* objectMemory);

    /// Kill the snake and stop the game
    void killSnake() noexcept {
        m_snakeIsAlive = false;
    }

    void finishEffect() noexcept;
    void removeBonus() noexcept;
    void removePowerup() noexcept;

    std::uintmax_t move();
    void pushCommand(Direction rotateCommand) noexcept;

    // Getters

    const Randomizer* getRandomizer(RandomizerType what) const noexcept;

    /// The method returns true if Snake is alive and the game is active, otherwise returns false
    bool isSnakeAlive() const noexcept {
        return m_snakeIsAlive;
    }

/// The method returns true if Snake is moving, otherwise retuns false.
/// The snake can be stopped by the stopper, but the player can move the snake back.
/// Don't confuse with isSnakeAlive.
    bool isSnakeMoving() const noexcept {
        return m_snakeIsMoving;
    }

/// The snake can be accelerated by the according objects on the map or by the effect.
    Acceleration getSnakeAcceleration() const noexcept {
        return m_acceleration;
    }

    Direction getSnakeDirection() const noexcept {
        return m_snakeDirection;
    }

/// Get the current effect that Snake has.
    EffectTypeAl getEffect() const noexcept {
        return m_effect;
    }

/// Compute the factual snake period using some states like effect and acceleration.
    std::intmax_t getFactualSnakePeriod() const noexcept;

    /// How many fruits Snake should eat to bonus acquiring.
    unsigned int getFruitCountToBonus() const noexcept {
        return m_fruitCountToBonus;
    }

/// How many bonuses Snake should eat to powerup acquiring.
    unsigned int getBonusCountToPowerup() const noexcept {
        return m_bonusCountToPowerup;
    }

/// Check spikes on the position on the map.
    std::uint32_t getObjectMemory(int x, int y) const;

    const SnakeWorld& getSnakeWorld() const noexcept {
        return m_snakeWorld;
    }

    std::uintmax_t getHarmlessLessStepID() const noexcept {
        return m_harmlessLessStepID;
    }

// internal use?
    const LevelPointers& getLevelPointers() const noexcept {
        return m_levelPtrs;
    }

private:

    std::uint32_t getLevelAttribute(LevelAttribEnum what) const noexcept;

    [[nodiscard]] PowerupType getRandomPowerup() const;

    void objectEffect(ObjectEffect effect);

    Randomizer& useRandomizer(RandomizerType what) const noexcept;

    ////////////////////////////////////////////////////////////
    /// Member data
    ////////////////////////////////////////////////////////////

    // Dependencies

    // The main component
    SnakeWorld m_snakeWorld;

    LevelPointers m_levelPtrs;
    std::array<Randomizer*, RandomTypeCount> m_randomizers;
    std::array<const Map<std::uint32_t>*, ItemCount> m_intiItemProbs;

    // main states

    // For detecting activated spikes
    std::vector<std::uint32_t> m_objectMemory;

    std::uintmax_t m_aimedTailSize = 0;

    // id LESS THAT step is harmless (harm is ||-sed)
    std::uintmax_t m_harmlessLessStepID = 0;

    // Current formal snake direction (where Snake would move)
    Direction m_snakeDirection = Direction::Count;

    // Current acceleration of Snake
    Acceleration m_acceleration = Acceleration::Default;

    // Current active Snake's effect
    EffectTypeAl m_effect = EffectTypeAl::NoEffect;

    // How many fruits Snake should eat to bonus acquiring
    unsigned int m_fruitCountToBonus = 0;

    // How many bonuses Snake should eat to powerup acquiring
    unsigned int m_bonusCountToPowerup = 0;

    // Temporarily it can be disabled,
    // but it can be enabled back if Player pushes direction.
    // Formally Snake can continue moving after death 
    bool m_snakeIsMoving = false;

    // Snake is alive (the game is active)
    // When Snake is alive, it can move.
    // If Snake dies, other objects continue living
    bool m_snakeIsAlive = false;
};

} // namespace Bulletworm

#endif // !GAME_IMPL_HPP
