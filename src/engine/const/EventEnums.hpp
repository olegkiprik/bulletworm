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

#ifndef EVENT_ENUMS_HPP
#define EVENT_ENUMS_HPP

namespace Bulletworm {

// leading events
enum class MainGameEvent {
	Moved,             // Snake has changed its position
	BonusExceed,       // Bonus has been exceed
	PowerupExceed,     // Powerup has been exceed
	EffectEnded,       // Effect has ended
	TimeLimitExceed,   // The time limit has exceeded and Snake has died

	Count,
};

// events after move
enum class GameSubevent {
	Killed,          // Snake has been killed

	// Post-move events
	FruitEaten,      // Fruit has been eaten by Snake
	BonusEaten,      // Bonus has been eaten by Snake
	PowerupEaten,    // Powerup has been eaten by Snake

	BonusAppended,   // Bonus has acquired on the map
	PowerupAppended, // Powerup has acquired on the map
	EffectAppended,  // Effect has acquired on Snake

	// Effect events
	RotatedPreEffect,   // Snake has been rotated by rotor
	RotatedPostEffect,  // Snake has been rotated by rotor
	Accelerated,        // Snake has been accelerated by accelerator
	Stopped,            // Snake has been stopped by stopper

	Count
};

constexpr int MainEventCount = static_cast<int>(MainGameEvent::Count);
constexpr int SubeventCount = static_cast<int>(GameSubevent::Count);

static_assert(MainEventCount <= 64);
static_assert(SubeventCount <= 32); // putting unpredictable memory value into the left side of mask

}

#endif // !EVENT_ENUMS_HPP