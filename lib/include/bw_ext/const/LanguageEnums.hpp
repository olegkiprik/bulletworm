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

#ifndef LANGUAGE_ENUMS_HPP
#define LANGUAGE_ENUMS_HPP

namespace Bulletworm {

enum class TimeUnit {
	Week,
	Day,
	Hour,
	Minute,
	Second,
	Millisecond,
	Microsecond,
	Count
};

// 0 - absolute singular (only 1)
// 1 - absolute dual (only 2)
// 2 - absolute paucal (only 3 or 4)
// 3 - relative singular (21, 31)
// 4 - relative dual (22, 32)
// 5 - relative paucal (23, 34)
// 6 - plural (0, 5, 11, 25, 100, 111)
enum class LinguisticNumericType {
	Single,
	Dual,
	Paucal,
	SeparatedSingle,
	SeparatedDual,
	SeparatedPaucal,
	Plural
};


constexpr int TimeUnitCount = static_cast<int>(TimeUnit::Count);

}

#endif // !LANGUAGE_ENUMS_HPP