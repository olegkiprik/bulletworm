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

#include "GameImpl.hpp"
#include "const/AttribEnums.hpp"
#include "const/ObjectEnums.hpp"
#include "const/EventEnums.hpp"
#include "ObjectBehavior.hpp"
#include <bw_ext/ObjParamEnumUtility.hpp>
#include <bw_ext/FenwickTree.hpp>
#include <bw_ext/random/Randomizer.hpp>
#include <cassert>

namespace {
template<class Vec>
sf::Vector2i getRandomPosition(const Vec& probMap,
                               const sf::Vector2u& mapSize,
                               Bulletworm::Randomizer& randomizer) {
    using fwt = Bulletworm::FenwickTree<typename Vec::iterator,
        typename Vec::const_iterator, std::ptrdiff_t, std::uintmax_t>;

    std::uintmax_t modulo = fwt::getSum(probMap.begin(), probMap.size() - 1);
    if (!modulo) return sf::Vector2i(mapSize);

    std::uintmax_t random = randomizer.get(0, modulo - 1);
    std::size_t target = fwt::rankQuery(probMap.begin(), probMap.end(), random);

    sf::Vector2i result;
    result.x = int(target % (std::size_t)mapSize.x);
    result.y = int(target / (std::size_t)mapSize.x);
    return result;
}
}

namespace Bulletworm {

GameImpl::GameImpl(GameImpl&& src) noexcept :
    m_snakeWorld(std::move(src.m_snakeWorld)),
    m_levelPtrs(src.m_levelPtrs),
    m_randomizers(std::move(src.m_randomizers)),
    m_intiItemProbs(std::move(src.m_intiItemProbs)),
    m_objectMemory(std::move(src.m_objectMemory)),
    m_aimedTailSize(src.m_aimedTailSize),
    m_harmlessLessStepID(src.m_harmlessLessStepID),
    m_snakeDirection(src.m_snakeDirection),
    m_acceleration(src.m_acceleration),
    m_effect(src.m_effect),
    m_fruitCountToBonus(src.m_fruitCountToBonus),
    m_bonusCountToPowerup(src.m_bonusCountToPowerup),
    m_snakeIsMoving(src.m_snakeIsMoving),
    m_snakeIsAlive(src.m_snakeIsAlive) {
    src.m_intiItemProbs.fill(nullptr);
    src.m_levelPtrs = LevelPointers{};
    src.m_randomizers.fill(nullptr);
    src.m_snakeIsAlive = false;
}


GameImpl& GameImpl::operator=(GameImpl&& src) noexcept {
    if (this == &src)
        return *this;

    m_acceleration = src.m_acceleration;
    m_aimedTailSize = src.m_aimedTailSize;
    m_bonusCountToPowerup = src.m_bonusCountToPowerup;
    m_effect = src.m_effect;
    m_fruitCountToBonus = src.m_fruitCountToBonus;
    m_harmlessLessStepID = src.m_harmlessLessStepID;
    m_intiItemProbs = std::move(src.m_intiItemProbs);
    m_levelPtrs = src.m_levelPtrs;
    m_objectMemory = std::move(src.m_objectMemory);
    m_randomizers = std::move(src.m_randomizers);
    m_snakeDirection = src.m_snakeDirection;
    m_snakeIsAlive = src.m_snakeIsAlive;
    m_snakeIsMoving = src.m_snakeIsMoving;
    m_snakeWorld = std::move(src.m_snakeWorld);

    src.m_intiItemProbs.fill(nullptr);
    src.m_levelPtrs = LevelPointers{};
    src.m_randomizers.fill(nullptr);
    src.m_snakeIsAlive = false;

    return *this;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
GameImpl::GameImpl() noexcept {
    m_randomizers.fill(nullptr);
    m_intiItemProbs.fill(nullptr);
}

GameImpl::GameImpl(const LevelPointers& ptrs,
                   Randomizer* const* randomizers,
                   const std::uint32_t* objectMemory,
                   Map<std::uint32_t> const* const* itemProbs) {
    reset(ptrs, randomizers, objectMemory, itemProbs);
}

void GameImpl::reset(const LevelPointers& ptrs,
                     Randomizer* const* randomizers,
                     const std::uint32_t* objectMemory,
                     Map<std::uint32_t> const* const* itemProbs) {
    std::copy(randomizers, randomizers + RandomTypeCount, m_randomizers.begin());
    std::copy(itemProbs, itemProbs + ItemCount, m_intiItemProbs.begin());

    assert(ptrs.attribArray);
    assert(ptrs.effectDurations);
    assert(ptrs.objectBehs);
    assert(ptrs.objectPairIndices);
    assert(ptrs.objectParams);
    assert(ptrs.postEffectBehIndices);
    assert(ptrs.powerupProbs);
    assert(ptrs.preEffectBehIndices);
    assert(ptrs.snakePositionProbs);
    assert(ptrs.tailCapacities1);

    m_levelPtrs = ptrs;
    restart(objectMemory);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void GameImpl::restart(const std::uint32_t* objectMemory) {
    sf::Vector2i snakePos =
        getRandomPosition(*m_levelPtrs.snakePositionProbs,
                          m_intiItemProbs.front()->getSize(),
                          useRandomizer(RandomizerType::Position));

    m_snakeWorld.restart(m_intiItemProbs.data(), snakePos);

    for (std::uint32_t i = 0; i < getLevelAttribute(LevelAttribEnum::FruitCount); ++i)
        m_snakeWorld.placeFruit(*m_randomizers[(std::size_t)RandomizerType::Position]);

    if (objectMemory) {
        m_objectMemory.assign(objectMemory, objectMemory + (std::size_t)getSnakeWorld().getMapSize().x *
                              getSnakeWorld().getMapSize().y);
    } else {
        m_objectMemory.clear();
        m_objectMemory.resize((std::size_t)getSnakeWorld().getMapSize().x *
                              getSnakeWorld().getMapSize().y);
    }

    // reset some states
    m_snakeDirection = Direction::Count;
    m_harmlessLessStepID = 0;
    m_snakeIsAlive = true;
    m_snakeIsMoving = false;
    m_acceleration = Acceleration::Default;
    m_effect = EffectTypeAl::NoEffect;
    m_aimedTailSize = getLevelAttribute(LevelAttribEnum::TailSize);
    m_fruitCountToBonus = getLevelAttribute(LevelAttribEnum::FruitCountToBonus);
    m_bonusCountToPowerup = getLevelAttribute(LevelAttribEnum::BonusCountToSuperbonus);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
std::uintmax_t GameImpl::move() {
    std::uintmax_t gameEvents = 0;
    constexpr std::uintmax_t MAX_ONE = 1;

    // Accelerations can eliminate!
    Acceleration previousAcceleration = m_acceleration;
    Direction directionBefore = m_snakeDirection;

    objectEffect(ObjectEffect::Pre);

    Direction preEffectDirection = m_snakeDirection;

    if (directionBefore != preEffectDirection)
        gameEvents |= (MAX_ONE << (int)GameSubevent::RotatedPreEffect);

    // deleting mode

    int backDeletingMode;

    if (m_aimedTailSize > m_snakeWorld.getTailSize()) {
        // increase the tail size by moving the head
        backDeletingMode = 0;
    } else if (m_aimedTailSize == m_snakeWorld.getTailSize()) {
        // don't change the tail size (compensate)
        backDeletingMode = 1;
    } else {
        // decrease the tail size (1 to compensate and 1 to erase)
        backDeletingMode = 2;
    }

    // possibly eaten powerup

    PowerupType possiblyEatenPowerup = PowerupType::NoPowerup;
    const auto& powerupMap = m_snakeWorld.getPowerups();

    if (!powerupMap.empty())
        possiblyEatenPowerup = powerupMap.begin()->second;

    // Move

    gameEvents |= m_snakeWorld.moveSnake(m_snakeDirection);

    // delete tail

    for (int i = 0; i < backDeletingMode; ++i)
        m_snakeWorld.trimTail();

    // Post effect

    sf::Vector2i currentSnakePosition = m_snakeWorld.getCurrentSnakePosition();
    std::uint64_t wereSpActdBeforePost =
        getObjectMemory(currentSnakePosition.x, 
                        currentSnakePosition.y);

    objectEffect(ObjectEffect::Post);

    // game events

    wereSpActdBeforePost <<= 32;
    static_assert(sizeof(std::uintmax_t) >= sizeof(std::uint64_t));
    gameEvents |= wereSpActdBeforePost;

    if (m_snakeDirection != preEffectDirection)
        gameEvents |= (MAX_ONE << (int)GameSubevent::RotatedPostEffect);

    if (previousAcceleration != m_acceleration)
        gameEvents |= (MAX_ONE << (int)GameSubevent::Accelerated);

    // Items, powerups, effects etc.

    bool bonusAcquired = false;
    bool powerupAcquired = false;

    bool eatenFruit = gameEvents & (MAX_ONE << (int)GameSubevent::FruitEaten);
    bool eatenBonus = gameEvents & (MAX_ONE << (int)GameSubevent::BonusEaten);
    bool eatenPowerup = gameEvents & (MAX_ONE << (int)GameSubevent::PowerupEaten);

    bool notNeedToTestTail = (eatenFruit || eatenBonus || eatenPowerup);

    if (eatenFruit) {
        m_snakeWorld.removeItem(currentSnakePosition);
        --m_fruitCountToBonus;

        // Increase the formal tail size.
        // By the way, only here this action does.
        std::uint64_t nextTailSize = m_aimedTailSize +
            std::uint64_t(getLevelAttribute(LevelAttribEnum::TailGrowth));
        std::uint64_t maxTailSize = getLevelAttribute(LevelAttribEnum::TailMaxSize);

        m_aimedTailSize = std::min(nextTailSize, maxTailSize);

        // If there is time to bonus acquiring
        if (m_fruitCountToBonus == 0) {
            // Update the count
            m_fruitCountToBonus = getLevelAttribute(LevelAttribEnum::FruitCountToBonus);

            // To check later (item acquiring does on step 4)
            bonusAcquired = true;
        }
    } else if (eatenBonus) {
        m_snakeWorld.removeItem(currentSnakePosition);
        --m_bonusCountToPowerup;

        // If there is time to powerup acquiring
        if (m_bonusCountToPowerup == 0) {
            // Update the count
            m_bonusCountToPowerup = getLevelAttribute(LevelAttribEnum::BonusCountToSuperbonus);

            // To check later (item acquiring does on step 4)
            powerupAcquired = true;
        }
    } else if (eatenPowerup) {
        m_snakeWorld.removeItem(currentSnakePosition);

        if (possiblyEatenPowerup < PowerupType::EffectCount) {
            m_effect = possiblyEatenPowerup;
            gameEvents |= (MAX_ONE << (int)GameSubevent::EffectAppended);
        } else {
            switch (possiblyEatenPowerup) {
            case PowerupType::InstantTailCut:
            {
                std::uint64_t stepCount = m_snakeWorld.getStepCount();
                unsigned int maxCollapseSize = getLevelAttribute(LevelAttribEnum::TailCollapseMaxSize);

                if (stepCount > m_harmlessLessStepID + maxCollapseSize)
                    m_harmlessLessStepID = stepCount - maxCollapseSize;

                break;
            }
            default:
                break;
            }
        }
    }

    // kill by tail

    if (!notNeedToTestTail) {
        unsigned int width = m_intiItemProbs.front()->getSize().x;
        const auto& tailId = m_snakeWorld.getTailIDs(currentSnakePosition);

        std::size_t tailElementFound = tailId.size();

        // without harmless'es
        std::size_t harmfullElementFound = tailElementFound;
        for (auto now = tailId.begin(); now != tailId.end(); ++now) {
            if (now->first < m_harmlessLessStepID) --harmfullElementFound;
            else break;
        }

        std::size_t freedom =
            (std::size_t)m_levelPtrs.tailCapacities1[m_levelPtrs.objectPairIndices[currentSnakePosition.x +
            currentSnakePosition.y * width]] - 1;

        // Check some reasons for staying alive
        bool ordinaryReason = (harmfullElementFound <= freedom);
        bool effectReason = (m_effect == EffectTypeAl::TailHarmless);

        if (!ordinaryReason && !effectReason) {
            m_snakeIsAlive = false; // kill Snake by tail
            gameEvents |= (MAX_ONE << (int)GameSubevent::Killed);
        }
    }

    // Place items

    Randomizer& positionRand = useRandomizer(RandomizerType::Position);
    PowerupType randPowerup = getRandomPowerup();

    if (eatenFruit)
        m_snakeWorld.placeFruit(positionRand);

    if (bonusAcquired) {
        gameEvents |= (MAX_ONE << (int)GameSubevent::BonusAppended);
        m_snakeWorld.clearBonuses();
        m_snakeWorld.placeBonus(positionRand);
    }

    if (powerupAcquired && randPowerup != PowerupType::PowerupCount) {
        gameEvents |= (MAX_ONE << (int)GameSubevent::PowerupAppended);
        m_snakeWorld.clearPowerups();
        m_snakeWorld.placePowerup(positionRand, randPowerup);
    }

    if (!m_snakeIsMoving)
        gameEvents |= (MAX_ONE << (int)GameSubevent::Stopped);

    if (!m_snakeIsAlive)
        gameEvents |= (MAX_ONE << (int)GameSubevent::Killed);

    return gameEvents;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void GameImpl::objectEffect(ObjectEffect effect) {
    // To start with
    sf::Vector2i currSnakePos = m_snakeWorld.getCurrentSnakePosition();
    bool preEffect = (effect == ObjectEffect::Pre);

    std::uint32_t param = m_levelPtrs.objectParams[currSnakePos.x +
        currSnakePos.y * m_intiItemProbs.front()->getSize().x];

    // Fill arguments
    ObjectBehavior::ExecutionArguments arguments;
    arguments.parameter = param;
    arguments.previousSnakeDirection = m_snakeWorld.getPreviousDirection();
    arguments.randomizer = &useRandomizer(RandomizerType::Behavior);

    // Fill target
    ObjectBehavior::ExecutionTarget target{};
    target.remembered = getObjectMemory(currSnakePos.x, currSnakePos.y);
    target.alive = m_snakeIsAlive;
    target.moving = m_snakeIsMoving;
    target.snakeAcceleration = m_acceleration;
    target.snakeDirection = m_snakeDirection;

    // Activate!

    {
        std::int64_t tmp1 = currSnakePos.x + (std::int64_t)currSnakePos.y * m_intiItemProbs.front()->getSize().x;
        std::uint32_t tmp2 = m_levelPtrs.objectPairIndices[tmp1];
        const std::uint32_t* tmp3 = (preEffect ? m_levelPtrs.preEffectBehIndices : m_levelPtrs.postEffectBehIndices);
        std::uint32_t tmp4 = tmp3[tmp2];
        const ObjectBehavior& currentBehavior = m_levelPtrs.objectBehs[tmp4];

        currentBehavior.activate(target, arguments);
    }

    // Save from target
    m_snakeDirection = target.snakeDirection;
    m_acceleration = target.snakeAcceleration;
    m_snakeIsMoving = target.moving;
    m_snakeIsAlive = target.alive;

    m_objectMemory[(std::size_t)currSnakePos.x +
        (std::size_t)currSnakePos.y * getSnakeWorld().getMapSize().x] = target.remembered;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void GameImpl::pushCommand(Direction rotateCommand) noexcept {
    if (m_snakeIsAlive) {
        // check the condition that Snake can't rotate opposite itself
        bool areOpposite = false;

        Direction prevDir = m_snakeWorld.getPreviousDirection();

        // if the previous snake direction is defined
        if (prevDir != Direction::Count)
            areOpposite = (rotateCommand == oppositeDirection(prevDir));

        // rotate Snake as it's possible
        if (!areOpposite)
            m_snakeDirection = rotateCommand;

        // Even if Player click 'back' direction, then Snake will be revoked!!!
        // the snake will move if the rotate command pushed
        m_snakeIsMoving = true;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
std::intmax_t GameImpl::getFactualSnakePeriod() const noexcept {
    std::int64_t period = getLevelAttribute(LevelAttribEnum::SnakePeriod);
    std::int64_t numerator = 1;
    std::int64_t denominator = 1;

    switch (m_acceleration) {
    case Acceleration::Down:
        numerator *= getLevelAttribute(LevelAttribEnum::AccelDownNumerator);
        denominator *= getLevelAttribute(LevelAttribEnum::AccelDownDenominator);
        break;
    case Acceleration::Up:
        numerator *= getLevelAttribute(LevelAttribEnum::AccelUpNumerator);
        denominator *= getLevelAttribute(LevelAttribEnum::AccelUpDenominator);
        break;
    default:
        break;
    }

    if (m_effect == EffectTypeAl::SlowDown) {
        numerator *= getLevelAttribute(LevelAttribEnum::SlowDownNumerator);
        denominator *= getLevelAttribute(LevelAttribEnum::SlowDownDenominator);
    }

    return divRound(period * numerator, denominator);
}


std::uint32_t GameImpl::getLevelAttribute(LevelAttribEnum what) const noexcept {
    return m_levelPtrs.attribArray[static_cast<int>(what)];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
PowerupType GameImpl::getRandomPowerup() const {
    // To start with

    using fwt = Bulletworm::FenwickTree<
        std::array<std::uintmax_t,
        fwkGetRealSize<std::size_t, int>(PowerupCount)>::iterator,
        std::array<std::uintmax_t,
        fwkGetRealSize<std::size_t, int>(PowerupCount)>::const_iterator,
        std::ptrdiff_t, std::uintmax_t>;

    std::uintmax_t modulo = fwt::getSum(m_levelPtrs.powerupProbs->begin(), 
                                        m_levelPtrs.powerupProbs->size() - 1);

    // Use random
    auto& currRandom = useRandomizer(RandomizerType::Powerup);
    std::uintmax_t random = currRandom.get(0, modulo - 1);

    // Map the powerup
    std::size_t index =
        fwt::rankQuery(m_levelPtrs.powerupProbs->begin(),
                       m_levelPtrs.powerupProbs->end(),
                       random);

    return (PowerupType)index;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
std::uint32_t GameImpl::getObjectMemory(int x, int y) const {
    return m_objectMemory[(std::size_t)x + (std::size_t)y * getSnakeWorld().getMapSize().x];
}


////////////////////////////////////////////////////////////////////////////////////////////////////
const Randomizer* GameImpl::getRandomizer(RandomizerType what) const noexcept {
    return m_randomizers[static_cast<std::size_t>(what)];
}


////////////////////////////////////////////////////////////////////////////////////////////////////
Randomizer& GameImpl::useRandomizer(RandomizerType what) const noexcept {
    assert(m_randomizers[static_cast<std::size_t>(what)]);
    return *m_randomizers[static_cast<std::size_t>(what)];
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void GameImpl::finishEffect() noexcept {
    m_effect = EffectTypeAl::NoEffect;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void GameImpl::removeBonus() noexcept {
    m_snakeWorld.clearBonuses();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void GameImpl::removePowerup() noexcept {
    m_snakeWorld.clearPowerups();
}

} // namespace Bulletworm
