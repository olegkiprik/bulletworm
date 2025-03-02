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

#ifndef LEVEL_ELEMENTS_HPP
#define LEVEL_ELEMENTS_HPP

namespace Bulletworm {

// dedicated count and exclusions
enum class ObjectPair {
	Obstacle,                                     // Snake dies, if it collides there
	Spikes,                                              // Snake dies, if it collides there twice
	RotorWeak,                                  // Rotor that can rotate Snake up
	RotorStrong,                              // Rotor rotates Snake up
	Tube,                                       // Down -> Right, Left -> Up
	CombinedTube,                       // Up and right, left and down from itself
	Void,                                         // Nothing on the place
	Stopper,                                                  // Snake stops
	Bridge,                                                   // Snake can move here through itself
	Accelerator,                            // Accelerator-down
	Pointer,
	CombinedPointer,
	CombinedRotorStrong,
	RandomAccelerator,
	RandomDihotomicAccelerator,
	Count
};

constexpr int ObjectPairCount = (int)ObjectPair::Count;

}

#endif // LEVEL_ELEMENTS_HPP