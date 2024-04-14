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

#ifndef OBJ_PARAM_ENUM_UTILITY_HPP
#define OBJ_PARAM_ENUM_UTILITY_HPP
#include "ObjectParameterEnums.hpp"
#include <SFML/System/Vector2.hpp>

namespace Bulletworm {

enum class Orientation;

Direction transformDirectionByOrient(Direction direction, Orientation orientation) noexcept;

Direction oppositeDirection(Direction direction) noexcept;

Acceleration oppositeAcceleration(Acceleration acceleration) noexcept;

bool directionIsExit(DoubleDirection dd, Direction direction) noexcept;

Direction getCombinedTubeExit(CombinedDirection combinedTube, Direction enterDirection) noexcept;

template<class T>
sf::Vector2<T>& move(sf::Vector2<T>& vector, Direction direction) noexcept;

template<class T>
sf::Vector2<T>& moveOnModulus(sf::Vector2<T>& vector,
							  Direction direction, const sf::Vector2<T>& mod) noexcept;

						  ////////////////////////////////////////////////////////////////////////////////
template<class T>
sf::Vector2<T>& moveOnModulus(sf::Vector2<T>& vector,
							  Direction direction, const sf::Vector2<T>& mod) noexcept {
	return modulateVector2(move(vector, direction), mod);
}

////////////////////////////////////////////////////////////////////////////////
template<class T>
sf::Vector2<T>& move(sf::Vector2<T>& vector, Direction direction) noexcept {
	// T may be float!

	switch (direction) {
	case Direction::Up:
		vector.y -= 1;
		break;
	case Direction::Right:
		vector.x += 1;
		break;
	case Direction::Down:
		vector.y += 1;
		break;
	case Direction::Left:
		vector.x -= 1;
		break;
	default:
		break;
	}

	return vector;
}

} // namespace Bulletworm

#endif // !OBJ_PARAM_ENUM_UTILITY_HPP