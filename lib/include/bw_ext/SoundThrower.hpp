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

#ifndef SOUND_THROWER_HPP
#define SOUND_THROWER_HPP
#include <SFML/Audio/Sound.hpp>
#include <list>
#include <thread>
#include <mutex>
#include <atomic>

namespace sf {

class Sound;
class SoundBuffer;

}

namespace Bulletworm {

// just leave the sound playing to the background (oh, you can set no sound real time, try to use the global listener params)
class SoundThrower {
public:

	// put the params
	struct Parameters {
		float volume = 100; // [0; 100]
		float pitch = 1;
		sf::Time playingOffset; // from what time

		// advanced
		sf::Vector3f position;
		bool relativeToListener = false;
		float attenuation = 0;
		float minDistance = 1;
	};

	SoundThrower();
	~SoundThrower() noexcept;

	// keep the sound buffer, I don't save it. Just throw and forget
	void play(const sf::SoundBuffer& soundBuffer, const Parameters& parameters);

private:

	using sound_it_t = std::list<sf::Sound>::const_iterator;

	// rare background 'for' to delete played sounds
	void threadFunc();

	mutable std::mutex m_mutex;
	std::list<sf::Sound> m_sounds;
	std::atomic_bool m_threadWorks;
	std::thread m_thread;
};

}

#endif // !SOUND_THROWER_HPP
