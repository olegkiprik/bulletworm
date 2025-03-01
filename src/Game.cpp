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

#include "Game.hpp"
#include "engine/const/AttribEnums.hpp"

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
Game::Game(const GameImpl& impl) {
    restart(impl);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
Game::Game(GameImpl&& impl) noexcept {
    restart(std::move(impl));
}

Game::Game() noexcept
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void Game::restart(GameImpl&& impl) noexcept {
    m_impl = std::move(impl);
    innerRestart();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void Game::restart(const GameImpl& impl) {
    m_impl = impl;
    innerRestart();
}


void Game::restart(const std::uint32_t* objectMemory) {
    m_impl.restart(objectMemory);
    innerRestart();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void Game::pushCommand(std::int64_t now, Direction direction) {
    if (m_impl.isSnakeAlive()) {
        RotationEvent revt{};
        revt.timePoint = now;
        revt.direction = direction;
        m_rotationEvents.push_back(revt);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
bool Game::pollEvent(Event& event) noexcept {
    if (!m_eventQueue.empty()) {
        event = m_eventQueue.front();
        m_eventQueue.pop_front();
        return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void Game::update(std::int64_t now) {
    bool again = true; // We have to run through all following events

    // We additionally test Snake's life
    while (again && m_impl.isSnakeAlive()) {
        bool rbpFirst = false; // Whether a rotate command is being pushing now

        // Outer event time relative to game starting event
        std::int64_t timeToNextEvent = m_eventProcessor.getTimeToNextEvent();
        std::int64_t eeTime = m_lastUpdateTimePoint + timeToNextEvent;

        std::int64_t rbpClockTime = 0; // Command time relative to game starting event
        const RotationEvent* earliestRBP = nullptr; // command pointer

        if (!m_rotationEvents.empty()) {
            earliestRBP = &m_rotationEvents.front();
            rbpClockTime = earliestRBP->timePoint;

            // Define command or outer event is first
            rbpFirst = rbpClockTime < eeTime;
        }

        // Define the first event time
        std::int64_t eventTime = (rbpFirst ? rbpClockTime : eeTime);

        // Inclusive!
        if (eventTime <= now) {
            if (rbpFirst) {
                // we don't check any event after pushing the command
                // because the method don't return any event.
                // We shall erase this hack in the future.
                m_eventProcessor.goTo(earliestRBP->timePoint - m_lastUpdateTimePoint);

                bool movbefore = m_impl.isSnakeMoving();

                m_impl.pushCommand(earliestRBP->direction);

                if (!movbefore && m_impl.isSnakeMoving())
                    m_eventProcessor.addFutureEvent((std::size_t)MainGameEvent::Moved,
                                                    m_impl.getFactualSnakePeriod());

                m_rotationEvents.pop_front();
            } else {
                processOuterEvent(eventTime);
            }

            // update the event time
            m_lastUpdateTimePoint = eventTime;
            again = true;
        } else {
            m_eventProcessor.goTo(now - m_lastUpdateTimePoint); // important
            m_lastUpdateTimePoint = now;
            again = false;
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void Game::processOuterEvent(std::int64_t eventTimePoint) {
    constexpr std::uintmax_t MAX_ONE = 1;

    // Collect some previous data
    const SnakeWorld& snakeWorld = m_impl.getSnakeWorld();
    const SnakeWorld::PowerupMap& powerups = snakeWorld.getPowerups();

    PowerupType previousPowerup =
        (powerups.empty() ? PowerupType::NoPowerup : powerups.begin()->second);
    EffectTypeAl previousEffect = m_impl.getEffect();

    sf::Vector2i previousBonusPosition;
    if (!snakeWorld.getBonusPositions().empty())
        previousBonusPosition = *snakeWorld.getBonusPositions().begin();

    sf::Vector2i previousPowerupPosition;
    if (!powerups.empty())
        previousPowerupPosition = powerups.begin()->first;

    std::uintmax_t events = m_eventProcessor.getNextEvent();
    m_eventProcessor.goToEvent(); // Important

    std::uintmax_t subevs = 0;

    if (events & (MAX_ONE << (int)MainGameEvent::Moved)) {
        std::int64_t nonActiveKey = GameEventProcessor::NotActive;
        subevs = m_impl.move();

        if (subevs & (MAX_ONE << (int)GameSubevent::BonusEaten)) {
            m_eventProcessor.addFutureEvent((std::size_t)(MainGameEvent::BonusExceed), nonActiveKey);
        }
        if (subevs & (MAX_ONE << (int)GameSubevent::PowerupEaten)) {
            m_eventProcessor.addFutureEvent((std::size_t)(MainGameEvent::PowerupExceed), nonActiveKey);
        }
        if (subevs & (MAX_ONE << (int)GameSubevent::BonusAppended)) {
            m_eventProcessor.addFutureEvent((std::size_t)(MainGameEvent::BonusExceed),
                                            m_impl.getLevelPointers()
                                            .attribArray[(int)LevelAttribEnum::BonusLifetime]);
            events &= ~(MAX_ONE << (int)MainGameEvent::BonusExceed);
        }
        if (subevs & (MAX_ONE << (int)GameSubevent::PowerupAppended)) {
            m_eventProcessor.addFutureEvent((std::size_t)(MainGameEvent::PowerupExceed),
                                            m_impl.getLevelPointers()
                                            .attribArray[(int)LevelAttribEnum::SuperbonusLifetime]);
            events &= ~(MAX_ONE << (int)MainGameEvent::PowerupExceed);
        }
        if (subevs & (MAX_ONE << (int)GameSubevent::EffectAppended)) {
            EffectTypeAl effect = m_impl.getEffect();
            std::int64_t effectDuration = m_impl.getLevelPointers().effectDurations[(int)effect];
            m_eventProcessor.addFutureEvent((std::size_t)(MainGameEvent::EffectEnded), effectDuration);
            events &= ~(MAX_ONE << (int)MainGameEvent::EffectEnded);
        }
        if (subevs & (MAX_ONE << (int)GameSubevent::Stopped)) {
            m_eventProcessor.addFutureEvent((std::size_t)(MainGameEvent::Moved), nonActiveKey);
        }

        if (m_impl.isSnakeMoving())
            m_eventProcessor.addFutureEvent((std::size_t)(MainGameEvent::Moved),
                                            m_impl.getFactualSnakePeriod());
    }
    if (events & (MAX_ONE << (int)MainGameEvent::BonusExceed)) {
        m_impl.removeBonus();
    }
    if (events & (MAX_ONE << (int)MainGameEvent::PowerupExceed)) {
        m_impl.removePowerup();
    }
    if (events & (MAX_ONE << (int)MainGameEvent::EffectEnded)) {
        m_impl.finishEffect();
    }
    if (events & (MAX_ONE << (int)MainGameEvent::TimeLimitExceed)) {
        m_impl.killSnake();
    }

    // We prefer to push events after moving

    Event commonMainEvent{};
    commonMainEvent.isMain = true;
    commonMainEvent.unpredMemory = (subevs >> 32);

    Event commonSubevent{};
    commonSubevent.isMain = false;
    commonSubevent.unpredMemory = (subevs >> 32);

    commonMainEvent.time = eventTimePoint;
    commonSubevent.time = eventTimePoint;

    if (events & (MAX_ONE << (int)MainGameEvent::Moved)) {
        Event movedEvent = commonMainEvent;
        movedEvent.mainGameEvent = MainGameEvent::Moved;
        m_eventQueue.push_back(movedEvent);
    }

    if (subevs & (MAX_ONE << (int)GameSubevent::FruitEaten)) {
        Event fruitEatenEvent = commonSubevent;
        fruitEatenEvent.subevent = GameSubevent::FruitEaten;
        m_eventQueue.push_back(fruitEatenEvent);
    }

    if (subevs & (MAX_ONE << (int)GameSubevent::BonusEaten)) {
        Event bonusEatenEvent = commonSubevent;
        bonusEatenEvent.subevent = GameSubevent::BonusEaten;
        m_eventQueue.push_back(bonusEatenEvent);
    }

    if (subevs & (MAX_ONE << (int)GameSubevent::PowerupEaten)) {
        Event powerupEatenEvent = commonSubevent;
        powerupEatenEvent.subevent = GameSubevent::PowerupEaten;
        powerupEatenEvent.powerupEatenEvent.powerup = previousPowerup;
        m_eventQueue.push_back(powerupEatenEvent);
    }

    if (subevs & (MAX_ONE << (int)GameSubevent::RotatedPreEffect)) {
        Event rotatedEvent = commonSubevent;
        rotatedEvent.subevent = GameSubevent::RotatedPreEffect;
        m_eventQueue.push_back(rotatedEvent);
    }

    if (subevs & (MAX_ONE << (int)GameSubevent::RotatedPostEffect)) {
        Event rotatedEvent = commonSubevent;
        rotatedEvent.subevent = GameSubevent::RotatedPostEffect;
        m_eventQueue.push_back(rotatedEvent);
    }

    if (subevs & (MAX_ONE << (int)GameSubevent::Accelerated)) {
        Event acceleratedEvent = commonSubevent;
        acceleratedEvent.subevent = GameSubevent::Accelerated;
        m_eventQueue.push_back(acceleratedEvent);
    }

    if (subevs & (MAX_ONE << (int)GameSubevent::Stopped)) {
        Event stoppedEvent = commonSubevent;
        stoppedEvent.subevent = GameSubevent::Stopped;
        m_eventQueue.push_back(stoppedEvent);
    }

    if (subevs & (MAX_ONE << (int)GameSubevent::Killed)) {
        Event killedEvent = commonSubevent;
        killedEvent.subevent = GameSubevent::Killed;
        m_eventQueue.push_back(killedEvent);
    }

    if (subevs & (MAX_ONE << (int)GameSubevent::BonusAppended)) {
        Event bonusAppendedEvent = commonSubevent;
        bonusAppendedEvent.subevent = GameSubevent::BonusAppended;
        m_eventQueue.push_back(bonusAppendedEvent);
    }

    if (subevs & (MAX_ONE << (int)GameSubevent::PowerupAppended)) {
        Event powerupAppendedEvent = commonSubevent;
        powerupAppendedEvent.subevent = GameSubevent::PowerupAppended;
        m_eventQueue.push_back(powerupAppendedEvent);
    }

    if (subevs & (MAX_ONE << (int)GameSubevent::EffectAppended)) {
        Event effectAppendedEvent = commonSubevent;
        effectAppendedEvent.subevent = GameSubevent::EffectAppended;
        m_eventQueue.push_back(effectAppendedEvent);
    }

    if (events & (MAX_ONE << (int)MainGameEvent::BonusExceed)) {
        Event bonusLostEvent = commonMainEvent;
        bonusLostEvent.mainGameEvent = MainGameEvent::BonusExceed;
        bonusLostEvent.bonusLostEvent.x = previousBonusPosition.x;
        bonusLostEvent.bonusLostEvent.y = previousBonusPosition.y;
        m_eventQueue.push_back(bonusLostEvent);
    }

    if (events & (MAX_ONE << (int)MainGameEvent::PowerupExceed)) {
        Event powerupLostEvent = commonMainEvent;
        powerupLostEvent.mainGameEvent = MainGameEvent::PowerupExceed;
        powerupLostEvent.powerupLostEvent.powerup = previousPowerup;
        powerupLostEvent.powerupLostEvent.x = previousPowerupPosition.x;
        powerupLostEvent.powerupLostEvent.y = previousPowerupPosition.y;
        m_eventQueue.push_back(powerupLostEvent);
    }

    if (events & (MAX_ONE << (int)MainGameEvent::EffectEnded)) {
        Event effectEndedEvent = commonMainEvent;
        effectEndedEvent.mainGameEvent = MainGameEvent::EffectEnded;
        effectEndedEvent.effectEndedEvent.effect = previousEffect;
        m_eventQueue.push_back(effectEndedEvent);
    }

    if (events & (MAX_ONE << (int)MainGameEvent::TimeLimitExceed)) {
        Event timeLimitExceededEvent = commonMainEvent;
        timeLimitExceededEvent.mainGameEvent = MainGameEvent::TimeLimitExceed;
        m_eventQueue.push_back(timeLimitExceededEvent);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void Game::innerRestart() noexcept {
    m_lastUpdateTimePoint = 0;
    std::list<Event>().swap(m_eventQueue);
    std::list<RotationEvent>().swap(m_rotationEvents);
    m_eventProcessor.clear();
    m_eventProcessor.addFutureEvent((std::size_t)(MainGameEvent::TimeLimitExceed),
                                    m_impl.getLevelPointers()
                                    .attribArray[(int)LevelAttribEnum::TimeLimit]);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
Game::Game(Game&& src) noexcept :
    m_eventProcessor(std::move(src.m_eventProcessor)),
    m_eventQueue(std::move(src.m_eventQueue)),
    m_impl(std::move(src.m_impl)),
    m_lastUpdateTimePoint(src.m_lastUpdateTimePoint),
    m_rotationEvents(std::move(src.m_rotationEvents)) {
    src.m_lastUpdateTimePoint = 0;
    src.m_eventProcessor.clear();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
Game& Game::operator=(Game&& src) noexcept {
    if (this == &src)
        return *this;

    m_eventProcessor = std::move(src.m_eventProcessor);
    m_eventQueue = std::move(src.m_eventQueue);
    m_impl = std::move(src.m_impl);
    m_lastUpdateTimePoint = src.m_lastUpdateTimePoint;
    m_rotationEvents = std::move(src.m_rotationEvents);

    src.m_lastUpdateTimePoint = 0;
    src.m_eventProcessor.clear();

    return *this;
}

} // namespace Bulletworm