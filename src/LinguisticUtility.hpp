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

#ifndef LINGUISTIC_UTILITY_HPP
#define LINGUISTIC_UTILITY_HPP
#include "LanguageEnums.hpp"
#include "ExternalConstants.hpp"

namespace Bulletworm {

template<class T>
constexpr LinguisticNumericType linguisticCountType(T count) noexcept;

template<class T>
void convertTime(T srcMicroseconds, T* timeArray) noexcept;

template<class T>
constexpr LinguisticNumericType linguisticCountType(T count) noexcept {
	if (count == 1)
		return LinguisticNumericType::Single;

	if (count == 2)
		return LinguisticNumericType::Dual;

	if (count == 3 || count == 4)
		return LinguisticNumericType::Paucal;

	if (count % 10 == 1 && count % 100 != 11)
		return LinguisticNumericType::SeparatedSingle;

	if (count % 10 == 2 && count % 100 != 12)
		return LinguisticNumericType::SeparatedDual;

	if ((count % 10 == 3 || count % 10 == 4) &&
		count % 100 != 13 && count % 100 != 14)
		return LinguisticNumericType::SeparatedPaucal;

	return LinguisticNumericType::Plural;
}

template<class T>
void convertTime(T srcMicroseconds, T* timeArray) noexcept {
	for (int i = 0; i < TimeUnitCount; ++i) {
		timeArray[i] = srcMicroseconds / TimeUnitRatios[i];
		srcMicroseconds -= timeArray[i] * TimeUnitRatios[i];
	}
}

}

#endif // !LINGUISTIC_UTILITY_HPP