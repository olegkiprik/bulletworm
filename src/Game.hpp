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

#ifndef GAME_HPP
#define GAME_HPP
#include <bw_ext/EventProcessor.hpp>
#include "engine/const/EventEnums.hpp"
#include "engine/GameImpl.hpp"
#include <list>

namespace Bulletworm {

/// The interface of BlockSnake game engine.
/// Supports time management and event processing.
class Game {
public:

    using GameEventProcessor = EventProcessor<std::int64_t, std::uintmax_t, MainEventCount>;

    class Event {
    public:
        
        // Time when the event has activated (relative to the game start)
        std::int64_t time;

        // to detect unpredictable memory
        std::uint32_t unpredMemory;

        MainGameEvent mainGameEvent;
        GameSubevent subevent;
        bool isMain;

        struct BonusLostEvent {
            int x;
            int y;
        };

        struct PowerupLostEvent {
            PowerupType powerup;
            int x;
            int y;
        };

        struct PowerupEatenEvent {
            PowerupType powerup;
        };

        struct EffectEndedEvent {
            EffectTypeAl effect;
        };

        union {
            BonusLostEvent bonusLostEvent;
            PowerupLostEvent powerupLostEvent;
            PowerupEatenEvent powerupEatenEvent;
            EffectEndedEvent effectEndedEvent;
        };
    };

    Game(const Game&) = default;
    Game(Game&&) noexcept;

    Game& operator=(const Game&) = default;
    Game& operator=(Game&&) noexcept;

    explicit Game(const GameImpl& impl);
    explicit Game(GameImpl&& impl) noexcept;

    Game() noexcept;

    void restart(GameImpl&& impl) noexcept;
    void restart(const GameImpl& impl);

    void restart(const std::uint32_t* objectMemory);

    /// Kill the snake and stop the game (you can peek game states, some events can be active)
    void killSnake() noexcept {
        m_impl.killSnake();
    }

/// Update the game states. Fills the event queue.
    void update(std::int64_t now);

    /// Extract one event from the queue.
    /// If the queue is empty, the method returns false.
    [[nodiscard]] bool pollEvent(Event& event) noexcept;

    void pushCommand(std::int64_t now, Direction direction);

    const GameImpl& getImpl() const noexcept {
        return m_impl;
    }

    const GameEventProcessor& getEventProcessor() const noexcept {
        return m_eventProcessor;
    }

private:

    void innerRestart() noexcept;

    /// Process the outer event on update
    void processOuterEvent(std::int64_t eventTimePoint);

    /// Represents rotate command
    struct RotationEvent {
        std::int64_t timePoint; // Time when the command was pushed
        Direction direction; // Pushed direction
    };

    GameImpl m_impl;                             // Game implementation
    GameEventProcessor m_eventProcessor;
    std::list<Event> m_eventQueue;               // Event queue that simplifies the work with events
    std::list<RotationEvent> m_rotationEvents;
    std::int64_t m_lastUpdateTimePoint = 0;      // Time that is ordered to game implementation status
};

} // namespace Bulletworm

#endif // !GAME_HPP