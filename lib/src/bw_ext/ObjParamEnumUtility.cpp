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

#include <bw_ext/ObjParamEnumUtility.hpp>
#include <bw_ext/const/Orientation.hpp>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
Direction transformDirectionByOrient(Direction direction, Orientation orientation) noexcept {
	unsigned orientFd = (unsigned)orientation;
	unsigned dirFd = (unsigned)direction;

	bool fl = orientFd & 4;
	bool rocw = orientFd & 2;
	bool flh = orientFd & 1;

	bool dirLd = dirFd & 2;
	bool dirHor = dirFd & 1;

	bool resLd = (fl != ((flh == rocw) ? dirLd : (dirLd != dirHor)));
	bool resHor = (dirHor != rocw);

	unsigned resultFd = 0;

	if (resLd)
		resultFd |= 2;

	if (resHor)
		resultFd |= 1;

	return (Direction)resultFd;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
Direction oppositeDirection(Direction direction) noexcept {
	return transformDirectionByOrient(direction, Orientation::Flip);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
Acceleration oppositeAcceleration(Acceleration acceleration) noexcept {
	if (acceleration == Acceleration::Down)
		return Acceleration::Up;

	if (acceleration == Acceleration::Up)
		return Acceleration::Down;

	return acceleration;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
bool directionIsExit(DoubleDirection dd, Direction direction) noexcept {
	switch (dd) {
	case DoubleDirection::UpRight:
		return (direction == Direction::Up) || (direction == Direction::Right);
	case DoubleDirection::Vertical:
		return (direction == Direction::Up) || (direction == Direction::Down);
	case DoubleDirection::UpLeft:
		return (direction == Direction::Up) || (direction == Direction::Left);
	case DoubleDirection::RightDown:
		return (direction == Direction::Right) || (direction == Direction::Down);
	case DoubleDirection::Horizontal:
		return (direction == Direction::Left) || (direction == Direction::Right);
	case DoubleDirection::DownLeft:
		return (direction == Direction::Down) || (direction == Direction::Left);
	default:
		return false;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
Direction getCombinedTubeExit(CombinedDirection combinedTube, Direction enterDirection) noexcept {
	switch (combinedTube) {
	case CombinedDirection::UpRight:
		switch (enterDirection) {
		case Direction::Up:    return Direction::Left;
		case Direction::Right: return Direction::Down;
		case Direction::Down:  return Direction::Right;
		case Direction::Left:  return Direction::Up;
		default:
			break;
		}
		break;
	case CombinedDirection::Cross:
		return enterDirection;
	case CombinedDirection::UpLeft:
		switch (enterDirection) {
		case Direction::Up:    return Direction::Right;
		case Direction::Right: return Direction::Up;
		case Direction::Down:  return Direction::Left;
		case Direction::Left:  return Direction::Down;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return Direction::Count;
}

} // namespace Bulletworm
