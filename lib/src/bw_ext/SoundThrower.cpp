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

#include <bw_ext/SoundThrower.hpp>
#include <vector>
#include <algorithm>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
SoundThrower::SoundThrower() :
	m_mutex(),
	m_sounds(),
	m_threadWorks(true),
	m_thread(&SoundThrower::threadFunc, this) {}


////////////////////////////////////////////////////////////////////////////////////////////////////
SoundThrower::~SoundThrower() noexcept {
	m_threadWorks.store(false);
	m_thread.join();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundThrower::play(const sf::SoundBuffer& soundBuffer, const Parameters& parameters) {
	std::lock_guard<std::mutex> lock(m_mutex);

	sf::Sound& sound = m_sounds.emplace_back(soundBuffer);
	sound.setAttenuation(parameters.attenuation);
	sound.setMinDistance(parameters.minDistance);
	sound.setPitch(parameters.pitch);
	sound.setPlayingOffset(parameters.playingOffset);
	sound.setPosition(parameters.position);
	sound.setRelativeToListener(parameters.relativeToListener);
	sound.setVolume(parameters.volume);

	sound.play();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SoundThrower::threadFunc() {
	for (;;) {
		std::unique_lock<std::mutex> lock(m_mutex);

		std::vector<sound_it_t> played;
		played.reserve(m_sounds.size());

		for (sound_it_t now = m_sounds.cbegin(); now != m_sounds.cend(); ++now)
			if (now->getStatus() != sf::Sound::Playing)
				played.push_back(now);

		std::size_t nrSounds = m_sounds.size();

		std::for_each(played.begin(), played.end(),
					  [this](sound_it_t it) {
						  m_sounds.erase(it);
					  });

		lock.unlock();

		if (!m_threadWorks.load()) {
			if (played.size() == nrSounds)
				return;
		}

		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
}

} // ex
