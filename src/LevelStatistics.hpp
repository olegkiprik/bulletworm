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

#ifndef LEVEL_STATISTICS_HPP
#define LEVEL_STATISTICS_HPP
#include "AttribEnums.hpp"
#include <vector>
#include <array>
#include <cstdint>

namespace sf {
class InputStream;
}

namespace Bulletworm {

class OutputStream;
class BlockSnake;


class LevelStatistics {
public:

	friend class BlockSnake;

	[[nodiscard]] bool loadFromStream(sf::InputStream& stream, bool endiannessRequired);
	[[nodiscard]] bool saveToStream(OutputStream& stream, bool withEndianness) const;

	struct StatisticsToAdd {
		unsigned int levelIndex = 0;
		unsigned int difficulty = 0;
		bool levelCompleted = false;
		std::uint64_t gameTime = 0;
		std::uint32_t score = 0;
	};

	void resetLevelStatistics() noexcept;
	void addStatistics(const StatisticsToAdd& stats) noexcept;

	std::uint32_t getDifficultyCount() const noexcept;
	std::uint32_t getLevelCount() const noexcept;
	std::uint64_t getWholeGameTime() const noexcept;

	std::uint32_t getLevelHighestScore(unsigned int levelIndex) const noexcept;

	bool isLevelCompleted(unsigned int difficulty,
						  unsigned int levelIndex) const noexcept;

	bool levelExists(unsigned int difficulty,
					 unsigned int levelIndex) const noexcept;
	
	std::uint32_t getLevelGameCount(unsigned int difficulty,
									unsigned int levelIndex) const noexcept;

	std::size_t getAvailableLevelCount() const noexcept {
		return m_availableLevelCount;
	}
	std::uintmax_t getTotalScore() const noexcept {
		return m_totalScore;
	}
	std::uintmax_t getTotalGameCount() const noexcept {
		return m_totalGameCount;
	}

private:

	std::vector<std::uint32_t> m_levelCompleted; // 0 is incompleted, 1 is completed, 2 is UNAVAILABLE
	std::vector<std::uint32_t> m_levelScores; // levels only
	std::vector<std::uint32_t> m_levelGameCounts;
	std::array<std::uint32_t, FirstLevelStatisticsCount> m_first;

	std::size_t m_availableLevelCount = 0;
	std::uintmax_t m_totalScore = 0;
	std::uintmax_t m_totalGameCount = 0;

public:

	LevelStatistics() noexcept;

	LevelStatistics(const LevelStatistics&);
	LevelStatistics(LevelStatistics&&) noexcept;

	LevelStatistics& operator=(const LevelStatistics&);
	LevelStatistics& operator=(LevelStatistics&&) noexcept;
};

}

#endif // !LEVEL_STATISTICS_HPP
