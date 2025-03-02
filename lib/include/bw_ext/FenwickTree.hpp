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

#ifndef FENWICKTREEOPERATIONS_HPP
#define FENWICKTREEOPERATIONS_HPP
#include <type_traits>
#include <algorithm>

// https://en.wikipedia.org/wiki/Fenwick_tree#Implementation

namespace Bulletworm {

template<class Iter, class ConstIter, class Idx, class Value>
class FenwickTree {
public:

	static_assert(std::is_signed_v<Idx>&& std::is_integral_v<Idx>);

	[[nodiscard]] static Idx getParent(Idx i) {
		return i - (i & (-i));
	}

	[[nodiscard]] static Idx getNext(Idx i) {
		return i + (i & (-i));
	}

	[[nodiscard]] static Value getSum(ConstIter dataBegin, Idx i) {
		Value sum = *dataBegin;
		while (i != 0) {
			ConstIter datai = dataBegin;
			std::advance(datai, i);
			sum += *datai;
			i = getParent(i);
		}
		return sum;
	}

	static void update(Iter dataBegin, Iter dataEnd, Idx i, Value v) {
		Idx size = std::distance(dataBegin, dataEnd);
		if (i == 0) {
			*dataBegin += v;
			return;
		}
		while (i < size) {
			Iter datai = dataBegin;
			std::advance(datai, i);
			*datai += v;
			i = getNext(i);
		}
	}

	[[nodiscard]] static Value rangeSum(ConstIter dataBegin, Idx i, Idx j) {
		Value sum = 0;
		while (j > i) {
			ConstIter dataj = dataBegin;
			std::advance(dataj, j);
			sum += *dataj;
			j = getParent(j);
		}
		while (i > j) {
			ConstIter datai = dataBegin;
			std::advance(datai, i);
			sum -= *datai;
			i = getParent(i);
		}
		return sum;
	}

	static void init(Iter dataBegin, Iter dataEnd) {
		Idx size = std::distance(dataBegin, dataEnd);
		for (Idx i = 1; i < size; ++i) {
			Idx j = getNext(i);
			if (j < size) {
				Iter datai = dataBegin;
				Iter dataj = dataBegin;
				std::advance(datai, i);
				std::advance(dataj, j);
				*dataj += *datai;
			}
		}
	}

	static void fini(Iter dataBegin, Iter dataEnd) {
		Idx size = std::distance(dataBegin, dataEnd);
		for (Idx i = size - 1; i > 0; --i) {
			Idx j = getNext(i);
			if (j < size) {
				Iter datai = dataBegin;
				Iter dataj = dataBegin;
				std::advance(datai, i);
				std::advance(dataj, j);
				*dataj -= *datai;
			}
		}
	}

	[[nodiscard]] static Value get(ConstIter dataBegin, Idx i) {
		return rangeSum(dataBegin, i, i + 1);
	}

	static void set(Iter dataBegin, Iter dataEnd, Idx i, Value value) {
		update(dataBegin, dataEnd, i, value -
			   get(dataBegin, i));
	}

	// for non-negative values only
	[[nodiscard]] static Idx rankQuery(ConstIter dataBegin, ConstIter dataEnd, Value value) {
		Idx size = std::distance(dataBegin, dataEnd);
		Idx i = 0, j = size - 1;
		// j is a power of 2.

		value -= *dataBegin;
		for (; j > 0; j >>= 1) {
			if (i + j < size) {
				ConstIter dataij = dataBegin;
				std::advance(dataij, i + j);
				if (*dataij <= value) {
					value -= *dataij;
					i += j;
				}
			}
		}
		return i;
	}

};

} // ns

#endif // FENWICKTREEOPERATIONS_HPP