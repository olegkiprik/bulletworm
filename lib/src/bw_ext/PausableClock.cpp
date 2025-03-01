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

#include <bw_ext/PausableClock.hpp>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
PausableClock::PausableClock() noexcept :
	PausableClock(Status::Running) {}


////////////////////////////////////////////////////////////////////////////////////////////////////
PausableClock::PausableClock(Status status) noexcept :
	m_begin(clock_t::now()),
	m_pauseDuration(),	
	m_status(status) {}

	
////////////////////////////////////////////////////////////////////////////////////////////////////
void PausableClock::pause() noexcept {
	if (m_status == Status::Running) {
		m_status = Status::Paused;
		m_pauseDuration = clock_t::now() - m_begin;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void PausableClock::resume() noexcept {
	if (m_status == Status::Paused) {
		m_status = Status::Running;
		m_begin += clock_t::now() - m_begin - m_pauseDuration;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
PausableClock::Status PausableClock::getStatus() const noexcept {
	return m_status;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
PausableClock::duration_t PausableClock::getElapsed(clock_t::time_point now) const noexcept {
	return (m_status == Status::Paused) ? m_pauseDuration : (now - m_begin);
}

}
