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

#include "SnakeWorld.hpp"
#include <bw_ext/FenwickTree.hpp>
#include <bw_ext/random/Randomizer.hpp>
#include <bw_ext/ObjParamEnumUtility.hpp>
#include "const/EventEnums.hpp"
#include "const/EngineConstants.hpp"
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

void fwkCreate(std::vector<std::uintmax_t>& vec, const std::uint32_t* values,
            std::size_t sz) {
    using fwt = Bulletworm::FenwickTree<std::vector<std::uintmax_t>::iterator,
        std::vector<std::uintmax_t>::const_iterator, std::ptrdiff_t, std::uintmax_t>;

    constexpr auto realsize = [](std::size_t val) {
        unsigned int bitlog = 0;
        std::size_t tval = val ? val - 1 : 0;
        while (tval) {
            tval >>= 1;
            ++bitlog;
        }
        return (std::size_t)1 + (val ? ((std::size_t)1u << bitlog) : 0);
    };

    vec.resize(realsize(sz));

    std::copy(values, values + sz, vec.data() + 1);
    std::fill(vec.data() + sz + 1, vec.data() + vec.size(), 0);
    vec[0] = 0;
    fwt::init(vec.begin(), vec.end());
}

void fwkReset(std::vector<std::uintmax_t>& vec, const std::uint32_t* values,
              std::size_t sz) noexcept {
    using fwt = Bulletworm::FenwickTree<std::vector<std::uintmax_t>::iterator,
        std::vector<std::uintmax_t>::const_iterator, std::ptrdiff_t, std::uintmax_t>;

    constexpr auto realsize = [](std::size_t val) {
        unsigned int bitlog = 0;
        std::size_t tval = val ? val - 1 : 0;
        while (tval) {
            tval >>= 1;
            ++bitlog;
        }
        return (std::size_t)1 + (val ? ((std::size_t)1u << bitlog) : 0);
    };

    assert(vec.size() == realsize(sz));

    std::copy(values, values + sz, vec.data() + 1);
    std::fill(vec.data() + sz + 1, vec.data() + vec.size(), 0);
    vec[0] = 0;
    fwt::init(vec.begin(), vec.end());
}

}

namespace Bulletworm {

SnakeWorld::SnakeWorld(const Map<std::uint32_t>* const* initItemProbArr,
                       const sf::Vector2i& snakePosition) {
    restart(initItemProbArr, snakePosition);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::restart(const Map<std::uint32_t>* const* initItemProbArr,
                         const sf::Vector2i& snakePosition) {
    // assert
    {
        const sf::Vector2u& anchSize = initItemProbArr[0]->getSize();
        for (int i = 1; i < ItemCount; ++i) {
            const sf::Vector2u& currSize = initItemProbArr[i]->getSize();
            assert(currSize == anchSize);
        }
    }

    // init
    std::copy(initItemProbArr,
              initItemProbArr + ItemCount,
              m_initItemProbabilities.begin());

    createItemProbs();
    postInit(snakePosition);
    m_tailIDs.reset(getMapSize(), (std::size_t)getMapSize().x * getMapSize().y >= TriggerMapSize);
}


void SnakeWorld::postInit(const sf::Vector2i& snakePosition) noexcept {
    

    m_backPosition = m_snakePosition = snakePosition;

    // snake head
    closeAccess(m_snakePosition);

    // clear other states
    m_previousSnakeDirection = Direction::Count;

    m_bonusPositions.clear();
    m_fruitPositions.clear();
    m_powerupPositions.clear();

    m_stepCount = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::restart(const sf::Vector2i& snakePosition) noexcept {
    resetItemProbs();
    postInit(snakePosition);
    m_tailIDs.reset((std::size_t)getMapSize().x * getMapSize().y >= TriggerMapSize);
}


const SnakeWorld::TailIdSubList&
SnakeWorld::getTailIDs(const sf::Vector2i& position) const noexcept {
    return m_tailIDs.getList(position);
}

void SnakeWorld::createItemProbs() {
    // item accesses
    sf::Vector2i mapSize{ getMapSize() };
    std::size_t area = (std::size_t)mapSize.x * mapSize.y;

    for (int i = 0; i < ItemCount; ++i) {
        assert(getMapSize() == m_initItemProbabilities[i]->getSize());
        fwkCreate(m_itemProbabilities[i],
                  m_initItemProbabilities[i]->data(),
                  area);
    }
}


void SnakeWorld::resetItemProbs() noexcept {
    // item accesses
    sf::Vector2i mapSize{ getMapSize() };
    std::size_t area = (std::size_t)mapSize.x * mapSize.y;

    for (int i = 0; i < ItemCount; ++i) {
        assert(getMapSize() == m_initItemProbabilities[i]->getSize());
        fwkReset(m_itemProbabilities[i],
                  m_initItemProbabilities[i]->data(),
                  area);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
std::uintmax_t SnakeWorld::moveSnake(Direction direction) {
    assert(direction != Direction::Count);

    // Is it opposite?
    if (m_previousSnakeDirection != Direction::Count &&
        direction == oppositeDirection(m_previousSnakeDirection))
        return 0;

    const sf::Vector2u& mapSize = getMapSize();

    // Save the previous states
    sf::Vector2i previousPosition = m_snakePosition;
    sf::Vector2i previousNeckPosition;
    if (m_previousSnakeDirection != Direction::Count)
        previousNeckPosition = getNeckPosition();

    // Move Snake
    moveOnModulus(m_snakePosition, direction, sf::Vector2i(mapSize));

    // Set the tail

    TailDirection tailDirection{};
    const auto& prevNeckList = m_tailIDs.getList(previousNeckPosition);

    // add 'entry'
    if (m_previousSnakeDirection != Direction::Count && !prevNeckList.empty())
        tailDirection.tdentry = prevNeckList.rbegin()->second.tdexit;

    auto& prevList = m_tailIDs.setList(previousPosition);

    // add 'exit'
    tailDirection.tdexit = direction;

    prevList.push_back(std::pair(m_stepCount, tailDirection));

    m_previousSnakeDirection = direction;

    // Close access
    closeAccess(m_snakePosition);

    // Events
    std::uintmax_t events = 0;
    constexpr std::uintmax_t MAX_ONE = 1;

    if (m_fruitPositions.find(m_snakePosition) != m_fruitPositions.end())
        events |= (MAX_ONE << (int)GameSubevent::FruitEaten);

    if (m_bonusPositions.find(m_snakePosition) != m_bonusPositions.end())
        events |= (MAX_ONE << (int)GameSubevent::BonusEaten);

    if (m_powerupPositions.find(m_snakePosition) != m_powerupPositions.end())
        events |= (MAX_ONE << (int)GameSubevent::PowerupEaten);

    // Add the step
    ++m_stepCount;
    return events;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::trimTail() noexcept {
    assert(getTailSize());

    const sf::Vector2u& mapSize = getMapSize();
    auto& backList = m_tailIDs.setList(m_backPosition);

    // open access
    if (backList.size() == 1)
        openAccess(m_backPosition);

    Direction backDir = backList.begin()->second.tdexit;

    // from tail ids
    backList.erase(backList.begin());

    // back position forward
    moveOnModulus(m_backPosition, backDir, sf::Vector2i(mapSize));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::placeFruit(Randomizer& positionRandomizer) {
    sf::Vector2i randPos = getAvailablePosition(EatableItem::Fruit, positionRandomizer);
    sf::Vector2i mapSizei{ getMapSize() };

    if (randPos == mapSizei)
        return;

    closeAccess(randPos);
    m_fruitPositions.insert(randPos);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::placeBonus(Randomizer& positionRandomizer) {
    sf::Vector2i randPos = getAvailablePosition(EatableItem::Bonus, positionRandomizer);
    sf::Vector2i mapSizei{ getMapSize() };

    if (randPos == mapSizei)
        return;

    closeAccess(randPos);
    m_bonusPositions.insert(randPos);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::placePowerup(Randomizer& positionRandomizer, PowerupType certainPowerup) {
    sf::Vector2i randPos = getAvailablePosition(EatableItem::Powerup, positionRandomizer);
    sf::Vector2i mapSizei{ getMapSize() };

    if (randPos == mapSizei)
        return;

    closeAccess(randPos);
    auto pair{ std::pair(randPos, certainPowerup) };
    m_powerupPositions.insert(pair);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::removeItem(const sf::Vector2i& position) {
    auto fruitFound = m_fruitPositions.find(position);
    auto bonusFound = m_bonusPositions.find(position);
    auto powerupFound = m_powerupPositions.find(position);

    if (fruitFound != m_fruitPositions.end())
        m_fruitPositions.erase(fruitFound);
    else if (bonusFound != m_bonusPositions.end())
        m_bonusPositions.erase(bonusFound);
    else if (powerupFound != m_powerupPositions.end())
        m_powerupPositions.erase(powerupFound);
    else
        return;

    if (position != m_snakePosition &&
        m_tailIDs.getList(position).empty())
        openAccess(position);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::clearBonuses() noexcept {
    for (const auto& now : m_bonusPositions)
        if (now != m_snakePosition &&
            m_tailIDs.getList(now).empty())
            openAccess(now);

    m_bonusPositions.clear();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::clearPowerups() noexcept {
    for (const auto& now : m_powerupPositions)
        if (now.first != m_snakePosition &&
            m_tailIDs.getList(now.first).empty())
            openAccess(now.first);

    m_powerupPositions.clear();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
std::uintmax_t SnakeWorld::getTailSize() const noexcept {
    if (m_previousSnakeDirection == Direction::Count)
        return 0;

    sf::Vector2i neckPos = getNeckPosition();
    const auto& neck = m_tailIDs.getList(neckPos);

    if (neck.empty())
        return 0;

    std::uint64_t thenewest = neck.rbegin()->first;
    std::uint64_t theoldest = m_tailIDs.getList(m_backPosition).begin()->first;
    return thenewest + 1 - theoldest;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
sf::Vector2i SnakeWorld::getAvailablePosition(EatableItem item, Randomizer& randomizer) const {
    auto itemIndex = (std::size_t)item;
    const auto& itemProb = m_itemProbabilities[itemIndex];
    return getRandomPosition(itemProb, getMapSize(), randomizer);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
sf::Vector2i SnakeWorld::getNeckPosition() const noexcept {
    assert(m_previousSnakeDirection != Direction::Count);

    sf::Vector2i mapSizei{ getMapSize() };
    sf::Vector2i neckPos = m_snakePosition;
    Direction direction = oppositeDirection(m_previousSnakeDirection);
    moveOnModulus(neckPos, direction, mapSizei);
    return neckPos;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::setAccess(int x, int y, EatableItem item, std::uint32_t access) noexcept {
    std::size_t itemIndex = (std::size_t)item;
    const auto& mapSize = getMapSize();
    std::size_t valueIndex = x + (std::size_t)y * mapSize.x;

    using fwt = Bulletworm::FenwickTree<std::vector<std::uintmax_t>::iterator,
        std::vector<std::uintmax_t>::const_iterator, std::ptrdiff_t, std::uintmax_t>;

    fwt::update(m_itemProbabilities[itemIndex].begin(),
                m_itemProbabilities[itemIndex].end(),
                valueIndex + 1, (std::uintmax_t)access - fwt::get(m_itemProbabilities[itemIndex].begin(),
                valueIndex)); // not bug, but feature
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::closeAccess(int x, int y) noexcept {
    for (int i = 0; i < ItemCount; ++i)
        setAccess(x, y, EatableItem(i), 0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::openAccess(int x, int y) noexcept {
    for (int i = 0; i < ItemCount; ++i) {
        const auto& initItemProb = m_initItemProbabilities[i];
        setAccess(x, y, EatableItem(i), initItemProb->at(x, y));
    }
}


SnakeWorld::SnakeWorld(SnakeWorld&& src) noexcept :
    m_tailIDs(std::move(src.m_tailIDs)),
    m_itemProbabilities(std::move(src.m_itemProbabilities)),
    m_fruitPositions(std::move(src.m_fruitPositions)),
    m_bonusPositions(std::move(src.m_bonusPositions)),
    m_powerupPositions(std::move(src.m_powerupPositions)),
    m_initItemProbabilities(std::move(src.m_initItemProbabilities)),
    m_stepCount(src.m_stepCount),
    m_snakePosition(src.m_snakePosition),
    m_backPosition(src.m_backPosition),
    m_previousSnakeDirection(src.m_previousSnakeDirection) {
    src.m_stepCount = 0;
}


SnakeWorld& SnakeWorld::operator=(SnakeWorld&& src) noexcept {
    if (this == &src)
        return *this;

    m_backPosition = src.m_backPosition;
    m_bonusPositions = std::move(src.m_bonusPositions);
    m_fruitPositions = std::move(src.m_fruitPositions);
    m_initItemProbabilities = std::move(src.m_initItemProbabilities);
    m_itemProbabilities = std::move(src.m_itemProbabilities);
    m_powerupPositions = std::move(src.m_powerupPositions);
    m_previousSnakeDirection = src.m_previousSnakeDirection;
    m_snakePosition = src.m_snakePosition;
    m_stepCount = src.m_stepCount;
    m_tailIDs = std::move(src.m_tailIDs);

    src.m_stepCount = 0;

    return *this;
}


SnakeWorld::SnakeWorld() noexcept :
    m_initItemProbabilities{} {}



Direction SnakeWorld::getPreviousDirection() const noexcept {
    return m_previousSnakeDirection;
}


std::uint32_t SnakeWorld::getCurrentRelativeItemAcquireProb(EatableItem item, 
                                                            int x, int y) const noexcept {
    using fwt = Bulletworm::FenwickTree<std::vector<std::uintmax_t>::iterator,
        std::vector<std::uintmax_t>::const_iterator, std::ptrdiff_t, std::uintmax_t>;

    return (std::uint32_t)fwt::get(m_itemProbabilities[(std::size_t)item].begin(),
                    (std::ptrdiff_t)x + (std::ptrdiff_t)y * getMapSize().x);
}


std::uint32_t SnakeWorld::getCurrentRelativeItemAcquireProb(EatableItem item, 
                                                            const sf::Vector2i& pos) const noexcept {
    return getCurrentRelativeItemAcquireProb(item, pos.x, pos.y);
}


std::uint32_t SnakeWorld::getInitialRelativeItemAcquireProb(EatableItem item, 
                                                            int x, int y) const noexcept {
    return m_initItemProbabilities[(std::size_t)item]->at(x, y);
}


std::uint32_t SnakeWorld::getInitialRelativeItemAcquireProb(EatableItem item,
                                                            const sf::Vector2i& pos) const noexcept {
    return getInitialRelativeItemAcquireProb(item, pos.x, pos.y);
}


const sf::Vector2u& SnakeWorld::getMapSize() const noexcept {
    return m_initItemProbabilities.front()->getSize();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SnakeWorld::setAccess(const sf::Vector2i& position, EatableItem item, 
                           std::uint32_t access) noexcept {
    setAccess(position.x, position.y, item, access);
}

void SnakeWorld::openAccess(const sf::Vector2i& position) noexcept {
    openAccess(position.x, position.y);
}

void SnakeWorld::closeAccess(const sf::Vector2i& position) noexcept {
    closeAccess(position.x, position.y);
}

void SnakeWorld::TaidIdContainer::reset(const sf::Vector2u& newsize, bool enable_map) 
{
    if (is_map && enable_map) {
        TailIdMap().swap(map);
    } else if (!is_map && enable_map) {
        TailIdVector().swap(vector);
    } else if (is_map && !enable_map) {
        TailIdMap().swap(map);
        TailIdVector((std::size_t)newsize.x * newsize.y).swap(vector);
    } else {
        TailIdVector().swap(vector);
        vector.resize((std::size_t)newsize.x * newsize.y);
    }
    size = newsize;
    is_map = enable_map;
}

void SnakeWorld::TaidIdContainer::reset(bool enable_map) noexcept {
    if (is_map && enable_map) {
        TailIdMap().swap(map);
    } else if (!is_map && enable_map) {
        TailIdVector().swap(vector);
    } else if (is_map && !enable_map) {
        TailIdMap().swap(map);
        TailIdVector((std::size_t)size.x * size.y).swap(vector);
    } else {
        TailIdVector().swap(vector);
        std::for_each(vector.begin(), vector.end(),
                      [](TailIdSubList& item) {
                          TailIdSubList().swap(item);
                      });
    }
    is_map = enable_map;
}

const SnakeWorld::TailIdSubList& SnakeWorld::TaidIdContainer::getList(const sf::Vector2i& position)
const noexcept {
    if (is_map) {
        auto iter = map.find(position);
        
        if (iter == map.end())
            return VirtualSublist;

        return iter->second;
    } else {
        return vector[position.x + (std::size_t)position.y * size.x];
    }
}

SnakeWorld::TailIdSubList& SnakeWorld::TaidIdContainer::setList(const sf::Vector2i& position) {
    if (is_map) {
        return map[position];
    } else {
        return vector[position.x + (std::size_t)position.y * size.x];
    }
}

const SnakeWorld::TailIdSubList SnakeWorld::TaidIdContainer::VirtualSublist;

} // namespace Bulletworm