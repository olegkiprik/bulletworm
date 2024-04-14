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

#ifndef ORIENTATION_HPP
#define ORIENTATION_HPP

namespace Bulletworm {

// Orthogonal two-dimensional orientation
enum class Orientation {
	Identity,                // No effect
	FlipHorizontally,        // Flip on vertical axis
	RotateClockwise,         // Rotate clockwise (right-down system)
	InverseTranspose,        // Flip on alternative diagonal (./.)
	Flip,                    // Rotate on 180 degrees
	FlipVertically,          // Flip on horizontal axis
	RotateCounterClockwise,  // Rotate counter-clockwise (right-down system)
	MainTranspose,           // Flip on main diagonal (.\.)

	Count
};

constexpr int OrientationCount = (int)Orientation::Count;

}

#endif // !ORIENTATION_HPP