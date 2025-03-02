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

#ifndef OBJECT_PARAMETER_ENUMS_HPP
#define OBJECT_PARAMETER_ENUMS_HPP

namespace Bulletworm {

// The accelerator attribute
enum class Acceleration {
	Default, // No effect
	Down,    // Slow-down
	Up,      // Speed-up

	Count    // Acceleration count
};

// Combined tube attribute
enum class CombinedDirection {
	UpRight, // Up and right, left and down from itself
	Cross,   // Up and down, left and right from itself
	UpLeft,  // Up and left, down and right from itself

	Count    // Combined tube count
};

enum class Direction {
	Up,    // Up from itself
	Right, // Right from itself
	Down,  // Down from itself
	Left,  // Left from itself

	Count  // Direction count
};

// Tube attribute
enum class DoubleDirection {
	UpRight,    // Up and right from itself
	Vertical,   // Up and down from itself
	UpLeft,     // Up and left from itself
	RightDown,  // Down and right from itself
	Horizontal, // Left and right from itself
	DownLeft,   // Down and left from itself

	Count       // Double direction count
};

constexpr int AccelerationCount = (int)Acceleration::Count;
constexpr int DoubleDirectionCount = (int)DoubleDirection::Count;
constexpr int DirectionCount = (int)Direction::Count;
constexpr int CombinedTubeCount = (int)CombinedDirection::Count;

}

#endif // !OBJECT_PARAMETER_ENUMS_HPP