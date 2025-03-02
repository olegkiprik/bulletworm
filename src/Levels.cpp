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

#include "Levels.hpp"
#include <bw_ext/FenwickTree.hpp>
#include "Constants.hpp"
#include <array>
#include "engine/const/AttribEnums.hpp"
#include <SFML/System/FileInputStream.hpp>
#include "engine/const/EatableItem.hpp"
#include <bw_ext/Endianness.hpp>
#include <cassert>

namespace {

template<class T>
void fwkReset(T& vec, const std::uint32_t* values,
			  std::size_t sz) noexcept {
	using fwt = Bulletworm::FenwickTree<typename T::iterator,
		typename T::const_iterator, std::ptrdiff_t, std::uintmax_t>;

	constexpr auto realsize = [](std::size_t val) {
		unsigned int bitlog = 0;
		std::size_t tval = val ? val - 1 : 0;
		while (tval) {
			tval >>= 1;
			++bitlog;
		}
		return (std::size_t)1 + (val ? ((std::size_t)1u << bitlog) : 0);
	};

	assert(vec.size() == realsize(sz));

	std::copy(values, values + sz, vec.data() + 1);
	std::fill(vec.data() + sz + 1, vec.data() + vec.size(), 0);
	vec[0] = 0;
	fwt::init(vec.begin(), vec.end());
}

}

namespace Bulletworm {
bool Levels::loadFromStream(unsigned int diffCount, 
							unsigned int levelCount, 
							sf::InputStream& stream,
							bool endiannessRequired) {
	assert(diffCount >= DiffCountMin);
	assert(diffCount <= DiffCountMax);
	assert(levelCount <= LevelCountMax);
	assert(levelCount >= LevelCountMin);

	// TODO: verify levels here

	std::vector<std::uint32_t> levelAttributes(diffCount * levelCount * LevelAttribCount);
	std::vector<std::uint32_t> levelPlotData(diffCount * levelCount * LevelPlotDataCount);
	std::vector<std::uint32_t> effectDurations(diffCount * levelCount * EffectCount);
	std::vector<std::array<std::uintmax_t, fwkGetRealSizeLvl<std::size_t, int>(PowerupCount)>> powerupProbs(diffCount * levelCount);
	std::vector<sf::Vector2u> mapSizes(diffCount * levelCount);
	std::vector<std::vector<std::uint32_t>> levelCountMaps(diffCount * levelCount * LevelCountMapCount);
	std::vector<std::vector<std::uint32_t>> itemProbCountMaps(diffCount * levelCount * ItemCount);

	std::array<std::uint32_t, PowerupCount> tempPowerupProb{};
	std::array<std::uint32_t, 2> tempTwo{};

	for (unsigned int lvl = 0; lvl < levelCount; ++lvl) {
		for (unsigned int diff = 0; diff < diffCount; ++diff) {
			std::int64_t readContent = 0;

			// attributes
			std::uint32_t* ctntdata = levelAttributes.data() +
				(lvl + diff * levelCount) * LevelAttribCount;
			std::int64_t ctntsize = (std::int64_t)sizeof(std::uint32_t) * LevelAttribCount;

			auto loadFunc = [&readContent, &ctntdata, &ctntsize, &stream, &endiannessRequired]()->bool {
				readContent = stream.read(ctntdata, ctntsize);
				bool success = (readContent == ctntsize);
				if (success) {
					// endianness
					if (endiannessRequired) {
						std::for_each(ctntdata, ctntdata + ctntsize / sizeof(std::uint32_t),
									  [](std::uint32_t& v) {
										  v = n2hl(v);
									  });
					}
				}
				return success;
			};

			if (!loadFunc())
				return false;

			// effect durations
			ctntdata = effectDurations.data() + (lvl + diff * levelCount) * EffectCount;
			ctntsize = (std::int64_t)sizeof(std::uint32_t) * EffectCount;

			if (!loadFunc())
				return false;

			// powerup probs
			ctntdata = tempPowerupProb.data();
			ctntsize = (std::int64_t)sizeof(std::uint32_t) * tempPowerupProb.size();

			if (!loadFunc())
				return false;

			fwkReset(powerupProbs[lvl + (std::size_t)diff * levelCount], tempPowerupProb.data(), PowerupCount);

			// plot data
			ctntdata = levelPlotData.data() + (lvl + diff * levelCount) * LevelPlotDataCount;
			ctntsize = (std::int64_t)sizeof(std::uint32_t) * LevelPlotDataCount;

			if (!loadFunc())
				return false;

			// width and height
			ctntdata = tempTwo.data();
			ctntsize = (std::int64_t)sizeof(std::uint32_t) * tempTwo.size();

			if (!loadFunc())
				return false;

			if (tempTwo[0] < WidthMin || tempTwo[1] < HeightMin ||
				tempTwo[0] > WidthMax || tempTwo[1] > HeightMax)
				return false;

			mapSizes[lvl + (std::size_t)diff * levelCount].x = tempTwo[0];
			mapSizes[lvl + (std::size_t)diff * levelCount].y = tempTwo[1];

			auto func = [&tempTwo, &stream, &lvl, &diff, &levelCount, &mapSizes, &endiannessRequired]
			(int fcount, std::vector<std::vector<std::uint32_t>>& ftarget)->bool {
				for (int levelCntId = 0; levelCntId < fcount; ++levelCntId) {
					std::uint32_t* lctntdata = tempTwo.data();
					std::int64_t lctntsize = (std::int64_t)sizeof(std::uint32_t);

					std::int64_t lreadContent = stream.read(lctntdata, lctntsize);
					if (lreadContent != lctntsize)
						return false;

					// endianness
					if (endiannessRequired) {
						std::for_each(lctntdata, lctntdata + 1,
									  [](std::uint32_t& v) {
										  v = n2hl(v);
									  });
					}

					std::size_t countMapSize = (std::size_t)tempTwo[0];
					countMapSize <<= 1; // chunks not elements

					if (!countMapSize)
						return false;

					std::size_t wheere = levelCntId +
						(std::size_t)(lvl + diff * levelCount) * fcount;
					ftarget[wheere].resize(countMapSize);

					lctntdata = ftarget[wheere].data();
					lctntsize = (std::int64_t)sizeof(std::uint32_t) * countMapSize;

					lreadContent = stream.read(lctntdata, lctntsize);
					if (lreadContent != lctntsize)
						return false;

					// endianness
					if (endiannessRequired) {
						std::for_each(lctntdata, lctntdata + countMapSize,
									  [](std::uint32_t& v) {
										  v = n2hl(v);
									  });
					}

					std::uintmax_t checkMapSize = 0;

					for (std::size_t ci = 0; ci < countMapSize; ci += 2)
						checkMapSize += ftarget[wheere][ci];

					if (checkMapSize !=
						(std::uintmax_t)mapSizes[lvl +
						(std::size_t)diff * levelCount].x *
						(std::uintmax_t)mapSizes[lvl +
						(std::size_t)diff * levelCount].y)
						return false;
				}

				return true;
			};

			// map data
			if (!func(LevelCountMapCount, levelCountMaps))
				return false;

			// separately probs for items
			if (!func(ItemCount, itemProbCountMaps))
				return false;
		}


	}

	// success

	m_levelAttributes.swap(levelAttributes);
	m_levelPlotData.swap(levelPlotData);
	m_effectDurations.swap(effectDurations);
	m_powerupProbs.swap(powerupProbs);
	m_mapSizes.swap(mapSizes);
	m_levelCountMaps.swap(levelCountMaps);
	m_itemProbCountMaps.swap(itemProbCountMaps);
	m_diffCount = diffCount;
	m_levelCount = levelCount;
	return true;
}

const std::uint32_t*
Levels::getLevelAttribPtr(unsigned int diffIndex,
						  unsigned int levelIndex) const noexcept {
	assert(diffIndex < m_diffCount);
	assert(levelIndex < m_levelCount);

	return m_levelAttributes.data() +
		(levelIndex + diffIndex * m_levelCount) * LevelAttribCount;
}

const std::uint32_t*
Levels::getLevelPlotDataPtr(unsigned int diffIndex,
							unsigned int levelIndex) const noexcept {
	assert(diffIndex < m_diffCount);
	assert(levelIndex < m_levelCount);

	return m_levelPlotData.data() +
		(levelIndex + diffIndex * m_levelCount) * LevelPlotDataCount;
}

const std::uint32_t*
Levels::getEffectDurationPtr(unsigned int diffIndex, 
							 unsigned int levelIndex) const noexcept {
	assert(diffIndex < m_diffCount);
	assert(levelIndex < m_levelCount);

	return m_effectDurations.data() + 
		(levelIndex + diffIndex * m_levelCount) * EffectCount;
}

const std::array<std::uintmax_t, fwkGetRealSizeLvl<std::size_t, int>(PowerupCount)>&
Levels::getPowerupProbs(unsigned int diffIndex,
						unsigned int levelIndex) const noexcept {
	assert(diffIndex < m_diffCount);
	assert(levelIndex < m_levelCount);

	return m_powerupProbs[levelIndex +
		(std::size_t)diffIndex * m_levelCount];
}

const sf::Vector2u&
Levels::getMapSize(unsigned int diffIndex, 
				   unsigned int levelIndex) const noexcept {
	assert(diffIndex < m_diffCount);
	assert(levelIndex < m_levelCount);

	return m_mapSizes[levelIndex +
		(std::size_t)diffIndex * m_levelCount];
}

const std::uint32_t*
Levels::getLevelCountMap(LevelCountMap what,
						 unsigned int diffIndex, 
						 unsigned int levelIndex) const noexcept {
	assert((int)what >= 0 && what < LevelCountMap::Count);
	assert(diffIndex < m_diffCount);
	assert(levelIndex < m_levelCount);

	return m_levelCountMaps[(unsigned)what +
		(std::size_t)(levelIndex +
					  diffIndex * m_levelCount) * LevelCountMapCount].data();
}

const std::uint32_t*
Levels::getItemProbCountMap(EatableItem what, 
							unsigned int diffIndex, 
							unsigned int levelIndex) const noexcept {
	assert((int)what >= 0 && what < EatableItem::Count);
	assert(diffIndex < m_diffCount);
	assert(levelIndex < m_levelCount);

	return m_itemProbCountMaps[(unsigned)what +
		(std::size_t)(levelIndex +
					  diffIndex * m_levelCount) * ItemCount].data();
}

}
