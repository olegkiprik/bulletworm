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

#ifndef BASIC_UTILITY_HPP
#define BASIC_UTILITY_HPP
#include <SFML/System/Vector2.hpp>
#include <cstdint>

namespace Bulletworm {

template<class T, class U>
constexpr decltype(auto) divTrunc(T first, U second) noexcept;

template<class T, class U>
constexpr T divFloor(T first, U second) noexcept;

template<class T, class U>
constexpr T divCeil(T first, U second) noexcept;

template<class T, class U>
constexpr T divRound(T first, U second) noexcept;

template<class T, class U> // get the module of some integers
T& modulate(T& dst, U mod) noexcept;

template<class T, class U>
sf::Vector2<T>& modulateVector2(sf::Vector2<T>& vector, const sf::Vector2<U>& mod) noexcept;

template<class T, class U>
constexpr T getModule(T src, U mod) noexcept;

template<class T> // just stupid abs for integers
constexpr T abs(T value) noexcept;

template<class T, class U, class V>
constexpr bool between(const T& src, const U& left, const V& right,
                       bool includingLeft, bool includingRight) noexcept;

template<class T, class U, class V> // including
constexpr bool between(const T& src, const U& left, const V& right) noexcept;


///////////////////////////////////////////////////////////////////////////////
template<class T, class U>
constexpr decltype(auto) divTrunc(T first, U second) noexcept {
    return first / second;
}


///////////////////////////////////////////////////////////////////////////////
template<class T, class U>
constexpr T divFloor(T first, U second) noexcept {
    if (first == T())
        return T();
    bool sgnNeg{
        (first > T() && second < U()) ||
        (first < T() && second > U())
    };
    T preres = first / second;
    T back = preres * second;
    return preres - ((sgnNeg && back != first) ? T(1) : T());
}


///////////////////////////////////////////////////////////////////////////////
template<class T, class U>
constexpr T divCeil(T first, U second) noexcept {
    if (first == T())
        return 0;
    bool sgnPos{
        first > T() && second > U() ||
        first < T() && second < U()
    };
    T preres = first / second;
    T back = preres * second;
    return preres + ((sgnPos && back != first) ? T(1) : T());
}


///////////////////////////////////////////////////////////////////////////////
template<class T, class U>
constexpr T divRound(T first, U second) noexcept {
    if (first == T())
        return T();
    T preres = divFloor(first << 1, second);
    if (preres & T(1))
        ++preres;
    return preres >> 1;
}


///////////////////////////////////////////////////////////////////////////////
template<class T>
constexpr T abs(T value) noexcept {
    return ((value >= T()) ? value : -value);
}


///////////////////////////////////////////////////////////////////////////////
template<class T, class U, class V>
constexpr bool between(const T& src, const U& left, const V& right,
                       bool includingLeft, bool includingRight) noexcept {
    bool grLeft{ includingLeft ? (src >= left) : (src > left) };
    bool lsRight{ includingRight ? (src <= right) : (src < right) };
    return grLeft && lsRight;
}


///////////////////////////////////////////////////////////////////////////////
template<class T, class U, class V>
constexpr bool between(const T& src, const U& left, const V& right) noexcept {
    return between(src, left, right, true, true);
}


///////////////////////////////////////////////////////////////////////////////
template<class T, class U>
constexpr T getModule(T src, U mod) noexcept {
    src %= mod;
    if (src < T())
        src += mod;
    return src;
}


///////////////////////////////////////////////////////////////////////////////
template<class T, class U>
T& modulate(T& dst, U mod) noexcept {
    return dst = getModule(dst, mod);
}


///////////////////////////////////////////////////////////////////////////////
template<class T, class U>
sf::Vector2<T>& modulateVector2(sf::Vector2<T>& vector, const sf::Vector2<U>& mod) noexcept {
    modulate(vector.x, mod.x);
    modulate(vector.y, mod.y);
    return vector;
}

} // namespace Bulletworm

#endif // BASIC_UTILITY_HPP