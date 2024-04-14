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

#ifndef AUDIO_ENUMS_HPP
#define AUDIO_ENUMS_HPP

namespace Bulletworm {

// Sound type to play
enum class SoundType {
    StopHit,                // stop the snake
    CriticalError,          // some errors in game NOT USED
    BonusAppear,		    // bonus
    BonusDisappear,	    	// bonus
    ItemEat,				// fruit, bonus
    AccelerateUp,	        // accel
    Death,				    // lose the game (snake is dead by obstacles)
    Victory,			    // Level completed | moment
    TimeLimitExceedSignal,	// time limit exceed
    EffectEnded,		    // effect ended
    EffectStarted,		    // effect begun when snake eat some powerups
    LevelComplete,		    // When you win | statistics
    PowerupAppear,	        // pow
    PowerupDisappear,   	// pow
    AccelerateDown,		    // accel
    InstantPowerupChoke,	// powerup that is not effect
    AccelerateDefault,	    // accel
    ActivateSpikes,		    // spikes
    ForcedRotating,			// rotor
    LittleFail,				// some fails in game NOT USED

    Count
};

constexpr int SoundTypeCount = static_cast<int>(SoundType::Count);

}

#endif // !AUDIO_ENUMS_HPP