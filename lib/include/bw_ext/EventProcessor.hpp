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

#ifndef EVENT_PROCESSOR_HPP
#define EVENT_PROCESSOR_HPP
#include <cstddef>
#include <array>

namespace Bulletworm {

// just few timers
template<class time_t, class mask_t, std::size_t N>
class EventProcessor {
public:

    static constexpr time_t NotActive = time_t();

    EventProcessor() noexcept;

    // the soonest event time
    time_t getTimeToNextEvent() const noexcept;

    // what events are soonest
    mask_t getNextEvent() const noexcept;

    // certain
    time_t getTimeToEvent(std::size_t theEvent) const noexcept;

    // all
    constexpr std::size_t getEventCount() const noexcept {
        return N;
    }

// the soonest gone
    void goToEvent() noexcept;

    // just go and ignore everything
    void goTo(time_t through) noexcept;

    // newbie
    void addFutureEvent(std::size_t mainEvent, time_t time) noexcept;

    // no events
    void clear() noexcept;

private:

    // find the soonest time
    void updateLessTime() const noexcept;

    // find the soonest events
    void updateNextEventMask() const noexcept;

    std::array<time_t, N> m_eventTimes;           //!< Clocks for events

    mutable time_t m_lessTime;            //!< Cached time to next event
    mutable mask_t m_nextEvents;
    mutable bool m_updatedNextEvents;
    mutable bool m_updatedLessTime;        //!< Should use the cached time to next event
};

template<class time_t, class mask_t, std::size_t N>
inline EventProcessor<time_t, mask_t, N>::EventProcessor() noexcept {
    clear();
}

template<class time_t, class mask_t, std::size_t N>
inline time_t EventProcessor<time_t, mask_t, N>::getTimeToNextEvent() const noexcept {
    updateLessTime();
    return m_lessTime;
}

template<class time_t, class mask_t, std::size_t N>
inline mask_t EventProcessor<time_t, mask_t, N>::getNextEvent() const noexcept {
    updateNextEventMask();
    return m_nextEvents;
}

template<class time_t, class mask_t, std::size_t N>
inline time_t EventProcessor<time_t, mask_t, N>::getTimeToEvent(std::size_t theEvent) const noexcept {
    return m_eventTimes[theEvent];
}

template<class time_t, class mask_t, std::size_t N>
inline void EventProcessor<time_t, mask_t, N>::goToEvent() noexcept {
    goTo(getTimeToNextEvent());
}

template<class time_t, class mask_t, std::size_t N>
inline void EventProcessor<time_t, mask_t, N>::goTo(time_t through) noexcept {
    for (std::size_t i = 0; i < N; ++i)
        if (m_eventTimes[i] != NotActive && through <= m_eventTimes[i])
            m_eventTimes[i] -= through;

    m_updatedLessTime = false;
    m_updatedNextEvents = false;
}

template<class time_t, class mask_t, std::size_t N>
inline void EventProcessor<time_t, mask_t, N>::addFutureEvent(std::size_t mainEvent, time_t time) noexcept {
    m_eventTimes[mainEvent] = std::max(time, NotActive);
    m_updatedLessTime = false;
    m_updatedNextEvents = false;
}

template<class time_t, class mask_t, std::size_t N>
inline void EventProcessor<time_t, mask_t, N>::clear() noexcept {
    m_eventTimes.fill(NotActive);
    m_lessTime = NotActive;
    m_nextEvents = mask_t();
    m_updatedLessTime = true;
    m_updatedNextEvents = true;
}

template<class time_t, class mask_t, std::size_t N>
inline void EventProcessor<time_t, mask_t, N>::updateLessTime() const noexcept {
    if (!m_updatedLessTime) {
        time_t minimum = NotActive;

        for (std::size_t i = 0; i < N; ++i)
            if ((m_eventTimes[i] < minimum && m_eventTimes[i] > NotActive) || minimum == NotActive)
                minimum = m_eventTimes[i];

        m_lessTime = minimum;
        m_updatedLessTime = true;
    }
}

template<class time_t, class mask_t, std::size_t N>
inline void EventProcessor<time_t, mask_t, N>::updateNextEventMask() const noexcept {
    if (!m_updatedNextEvents) {
        time_t lessTime = getTimeToNextEvent();
        m_nextEvents = mask_t();

        if (lessTime != NotActive) {
            for (std::size_t i = 0; i < N; ++i)
                if (m_eventTimes[i] == lessTime)
                    m_nextEvents |= (mask_t(1) << i);
        }

        m_updatedNextEvents = true;
    }
}

}

#endif // !EVENT_PROCESSOR_HPP