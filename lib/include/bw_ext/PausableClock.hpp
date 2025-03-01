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

#ifndef PAUSABLE_CLOCK_HPP
#define PAUSABLE_CLOCK_HPP
#include <chrono>

namespace Bulletworm {

// Steady clock that measures microseconds
class PausableClock {
public:

	enum class Status {
		Paused,
		Running
	};

	// clock is running by default
	PausableClock() noexcept;

	// choose the status explicitly
	explicit PausableClock(Status status) noexcept;

	// Pause the clock (means you stop the clock but save the current time measured)
	void pause() noexcept;

	// Resume the clock
	void resume() noexcept;

	// Restart the clock (reset to zero and start)
	template<class Rep, class Period = std::ratio<1, 1>>
	Rep restart() noexcept;

	// Stop the clock (stop and reset to zero)
	template<class Rep, class Period = std::ratio<1, 1>>
	Rep stop() noexcept;

	// Reset the clock (reset to zero only, doesn't care about the status)
	template<class Rep, class Period = std::ratio<1, 1>>
	Rep reset() noexcept;

	// Get time elapsed from the starting moment
	template<class Rep, class Period = std::ratio<1, 1>>
	Rep getElapsedTime() const noexcept;

	// running or paused?
	Status getStatus() const noexcept;

private:

	using clock_t =
		std::conditional_t<std::chrono::high_resolution_clock::is_steady,
		std::chrono::high_resolution_clock,
		std::chrono::steady_clock>;

	using duration_t = clock_t::duration;

	// Convert the duration to the native type (microseconds)
	template<class Rep, class Period = std::ratio<1, 1>>
	static Rep convert(duration_t duration) noexcept;

	// Get the local-typed elapsed time
	duration_t getElapsed(clock_t::time_point now) const noexcept;

	clock_t::time_point m_begin; // Starting moment
	duration_t m_pauseDuration;  // Time from the beginning
	Status m_status;             // Is the clock paused or running
};

////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Rep, class Period>
Rep PausableClock::convert(duration_t duration) noexcept {
	return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(duration).count();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Rep, class Period>
Rep PausableClock::getElapsedTime() const noexcept {
	return convert<Rep, Period>(getElapsed(clock_t::now()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Rep, class Period>
Rep PausableClock::restart() noexcept {
	Rep elapsed = reset<Rep, Period>();
	m_status = Status::Running;
	return elapsed;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Rep, class Period>
Rep PausableClock::stop() noexcept {
	Rep elapsed = reset<Rep, Period>();
	m_status = Status::Paused;
	return elapsed;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Rep, class Period>
Rep PausableClock::reset() noexcept {
	clock_t::time_point now = clock_t::now();
	duration_t elapsed = getElapsed(now);
	m_begin = now;
	m_pauseDuration = duration_t();
	return convert<Rep, Period>(elapsed);
}

} // namespace Bulletworm

#endif // PAUSABLE_CLOCK_HPP
