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

#include "LevelStatistics.hpp"
#include "Constants.hpp"
#include "OutputStream.hpp"
#include <SFML/System/InputStream.hpp>
#include <cassert>
#include "Endianness.hpp"
#include <algorithm>

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
std::size_t getLevelId(unsigned int levelCount, unsigned int difficulty,
                       unsigned int levelIndex) noexcept {
    return (std::size_t)difficulty * levelCount + levelIndex;
}

}

namespace Bulletworm {

void LevelStatistics::resetLevelStatistics() noexcept {
    std::uint32_t diffcnt = getDifficultyCount();
    std::uint32_t lvlcnt = getLevelCount();

    if (!lvlcnt)
        return;

    assert(diffcnt >= DiffCountMin);
    assert(lvlcnt >= LevelCountMin);
    assert(diffcnt <= DiffCountMax);
    assert(lvlcnt <= LevelCountMax);

    m_first[(std::size_t)FirstLevelStatisticsEnum::TotalGametimeLeast32] = 0;
    m_first[(std::size_t)FirstLevelStatisticsEnum::TotalGametimeMost32] = 0;

    std::fill(m_levelCompleted.begin(), m_levelCompleted.end(), 0);
    std::fill(m_levelScores.begin(), m_levelScores.end(), 0);
    std::fill(m_levelGameCounts.begin(), m_levelGameCounts.end(), 0);

    m_availableLevelCount = 1;
    m_totalScore = 0;
    m_totalGameCount = 0;
}
void LevelStatistics::addStatistics(const StatisticsToAdd& stats) noexcept {
    std::uint32_t diffcnt = getDifficultyCount();
    std::uint32_t lvlcnt = getLevelCount();

    assert(diffcnt >= DiffCountMin);
    assert(lvlcnt >= LevelCountMin);
    assert(diffcnt <= DiffCountMax);
    assert(lvlcnt <= LevelCountMax);

    assert(stats.difficulty < diffcnt);
    assert(stats.levelIndex < lvlcnt);

    std::uint64_t wholeTime = getWholeGameTime();
    wholeTime += stats.gameTime;

    m_first[(std::size_t)FirstLevelStatisticsEnum::TotalGametimeLeast32] =
        static_cast<std::uint32_t>(wholeTime << 32 >> 32);
    m_first[(std::size_t)FirstLevelStatisticsEnum::TotalGametimeMost32] =
        static_cast<std::uint32_t>(wholeTime >> 32);

    if (stats.levelCompleted) {
        if (stats.score > getLevelHighestScore(stats.levelIndex)) {
            m_totalScore -= getLevelHighestScore(stats.levelIndex);
            m_totalScore += stats.score;
            m_levelScores[stats.levelIndex] = stats.score;
        }

        if (!m_levelCompleted[getLevelId(lvlcnt, stats.difficulty, stats.levelIndex)]) {
            m_levelCompleted[getLevelId(lvlcnt, stats.difficulty, stats.levelIndex)] = true;
            // the first difficulty is introductory
            if (stats.difficulty > 0) {
                m_availableLevelCount = std::min(std::max(m_availableLevelCount,
                                                 (std::size_t)stats.levelIndex + 2),
                                                 (std::size_t)lvlcnt);
            }
        }
    }

    ++m_levelGameCounts[getLevelId(lvlcnt, stats.difficulty, stats.levelIndex)];
    ++m_totalGameCount;
}
std::uint32_t LevelStatistics::getDifficultyCount() const noexcept {
    return m_first[(std::size_t)FirstLevelStatisticsEnum::DiffCount];
}
std::uint32_t LevelStatistics::getLevelCount() const noexcept {
    return m_first[(std::size_t)FirstLevelStatisticsEnum::LevelCount];
}
std::uint64_t LevelStatistics::getWholeGameTime() const noexcept {
    std::uint64_t wholeTime = 0;
    wholeTime |= m_first[(std::size_t)FirstLevelStatisticsEnum::TotalGametimeMost32];
    wholeTime <<= 32;
    wholeTime |= m_first[(std::size_t)FirstLevelStatisticsEnum::TotalGametimeLeast32];
    return wholeTime;
}
std::uint32_t LevelStatistics::getLevelHighestScore(unsigned int levelIndex) const noexcept {
    std::uint32_t diffcnt = getDifficultyCount();
    std::uint32_t lvlcnt = getLevelCount();

    assert(diffcnt >= DiffCountMin);
    assert(lvlcnt >= LevelCountMin);
    assert(diffcnt <= DiffCountMax);
    assert(lvlcnt <= LevelCountMax);

    assert(levelIndex < lvlcnt);

    return m_levelScores[levelIndex];
}
bool LevelStatistics::isLevelCompleted(unsigned int difficulty,
                                       unsigned int levelIndex) const noexcept {
    std::uint32_t diffcnt = getDifficultyCount();
    std::uint32_t lvlcnt = getLevelCount();

    assert(diffcnt >= DiffCountMin);
    assert(lvlcnt >= LevelCountMin);
    assert(diffcnt <= DiffCountMax);
    assert(lvlcnt <= LevelCountMax);

    assert(difficulty < diffcnt);
    assert(levelIndex < lvlcnt);

    return m_levelCompleted[getLevelId(lvlcnt, difficulty, levelIndex)] == 1;
}
bool LevelStatistics::levelExists(unsigned int difficulty, unsigned int levelIndex) const noexcept {
    std::uint32_t diffcnt = getDifficultyCount();
    std::uint32_t lvlcnt = getLevelCount();

    assert(diffcnt >= DiffCountMin);
    assert(lvlcnt >= LevelCountMin);
    assert(diffcnt <= DiffCountMax);
    assert(lvlcnt <= LevelCountMax);

    assert(difficulty < diffcnt);
    assert(levelIndex < lvlcnt);

    return m_levelCompleted[getLevelId(lvlcnt, difficulty, levelIndex)] <= 1;
}
unsigned int LevelStatistics::getLevelGameCount(unsigned int difficulty,
                                                unsigned int levelIndex) const noexcept {
    std::uint32_t diffcnt = getDifficultyCount();
    std::uint32_t lvlcnt = getLevelCount();

    assert(diffcnt >= DiffCountMin);
    assert(lvlcnt >= LevelCountMin);
    assert(diffcnt <= DiffCountMax);
    assert(lvlcnt <= LevelCountMax);

    assert(difficulty < diffcnt);
    assert(levelIndex < lvlcnt);

    return m_levelGameCounts[getLevelId(lvlcnt, difficulty, levelIndex)];
}



bool LevelStatistics::loadFromStream(sf::InputStream& stream, bool endiannessRequired) {
    std::int64_t ctntsize = 0;
    std::uint32_t* ctntdata = nullptr;
    std::int64_t readCtnt = 0;

    std::array<std::uint32_t, FirstLevelStatisticsCount> first{};
    std::vector<std::uint32_t> levelCompleted;
    std::vector<std::uint32_t> levelScores;
    std::vector<std::uint32_t> levelGameCounts;

    ctntdata = first.data();
    ctntsize = (std::int64_t)sizeof(std::uint32_t) * first.size();

    readCtnt = stream.read(ctntdata, ctntsize);
    if (readCtnt != ctntsize)
        return false;

    // endianness
    std::for_each(ctntdata, ctntdata + first.size(),
                  [&endiannessRequired](uint32_t& v) {
                      if (endiannessRequired) {
                          v = n2hl(v);
                      }
                  });

    std::uint32_t diffn = first[(std::size_t)FirstLevelStatisticsEnum::DiffCount];
    std::uint32_t lvln = first[(std::size_t)FirstLevelStatisticsEnum::LevelCount];

    if (diffn < DiffCountMin || lvln < LevelCountMin ||
        diffn > DiffCountMax || lvln > LevelCountMax)
        return false;

    levelCompleted.resize((std::size_t)diffn * lvln);
    levelScores.resize(lvln);
    levelGameCounts.resize((std::size_t)diffn * lvln);

    auto func = [&stream, &endiannessRequired](std::vector<std::uint32_t>& vec) {
        std::int64_t lctntsize = 0;
        std::uint32_t* lctntdata = nullptr;
        std::int64_t lreadCtnt = 0;

        lctntdata = vec.data();
        lctntsize = (std::int64_t)sizeof(std::uint32_t) * vec.size();

        lreadCtnt = stream.read(lctntdata, lctntsize);
        if (lreadCtnt != lctntsize)
            return false;

        // endianness
        std::for_each(lctntdata, lctntdata + vec.size(),
                      [&endiannessRequired](uint32_t& v) {
                          if (endiannessRequired) {
                              v = n2hl(v);
                          }
                      });

        return true;
    };

    if (!func(levelCompleted))
        return false;

    if (!func(levelScores))
        return false;

    if (!func(levelGameCounts))
        return false;

    // succ

    m_totalScore = 0;
    for (auto now : levelScores)
        m_totalScore += now;

    m_totalGameCount = 0;
    for (auto now : levelGameCounts)
        m_totalGameCount += now;

    m_availableLevelCount = 1;
    for (std::uint32_t i = 0; i < lvln; ++i) {
        bool yes = false;
        // The first difficulty is introductory
        for (std::uint32_t j = 1; j < diffn; ++j) {
            if (levelCompleted[getLevelId(lvln, j, lvln - i - 1)]) {
                yes = true;
                break;
            }
        }

        if (yes) {
            m_availableLevelCount = (std::size_t)std::min(lvln - i + 1, lvln);
            break;
        }
    }

    m_first.swap(first);
    m_levelCompleted.swap(levelCompleted);
    m_levelGameCounts.swap(levelGameCounts);
    m_levelScores.swap(levelScores);
    return true;
}

bool LevelStatistics::saveToStream(OutputStream& stream, bool withEndianness) const {

    std::uint32_t diffcnt = getDifficultyCount();
    std::uint32_t lvlcnt = getLevelCount();

    assert(diffcnt >= DiffCountMin);
    assert(lvlcnt >= LevelCountMin);
    assert(diffcnt <= DiffCountMax);
    assert(lvlcnt <= LevelCountMax);

    const std::uint32_t* ctntdata = nullptr;
    std::int64_t ctntsize = 0;
    std::int64_t readctnt = 0;

    ctntdata = m_first.data();
    ctntsize = (std::int64_t)sizeof(std::uint32_t) * m_first.size();

    // endianness
    std::array<std::uint32_t, FirstLevelStatisticsCount> networkData;
    std::transform(m_first.begin(), m_first.end(), networkData.begin(),
                   [&withEndianness](std::uint32_t v) {
                       if (withEndianness)
                           return h2nl(v);
                       return v;
                   });

    readctnt = stream.write(networkData.data(), ctntsize);
    if (readctnt != ctntsize)
        return false;

    auto func = [&stream, &withEndianness](const std::vector<std::uint32_t>& src) {
        // endianness
        std::vector<std::uint32_t> srcnet(src.size());
        std::transform(src.begin(), src.end(), srcnet.begin(),
                       [&withEndianness](std::uint32_t v) {
                           if (withEndianness)
                               return h2nl(v);
                           return v;
                       });

        std::int64_t lctntsize = (std::int64_t)sizeof(std::uint32_t) * src.size();
        std::int64_t lreadctnt = stream.write(srcnet.data(), lctntsize);
        if (lreadctnt != lctntsize)
            return false;

        return true;
    };

    if (!func(m_levelCompleted))
        return false;

    if (!func(m_levelScores))
        return false;

    if (!func(m_levelGameCounts))
        return false;

    return true;
}






LevelStatistics::LevelStatistics() noexcept :
    m_first{} {}

LevelStatistics::LevelStatistics(const LevelStatistics&) = default;

LevelStatistics::LevelStatistics(LevelStatistics&& src) noexcept :
    m_first(std::move(src.m_first)),
    m_availableLevelCount(src.m_availableLevelCount),
    m_levelCompleted(std::move(src.m_levelCompleted)),
    m_levelGameCounts(std::move(src.m_levelGameCounts)),
    m_levelScores(std::move(src.m_levelScores)),
    m_totalGameCount(src.m_totalGameCount),
    m_totalScore(src.m_totalScore) {
    src.m_totalScore = 0;
    src.m_totalGameCount = 0;
    src.m_availableLevelCount = 0;

    m_first.fill(0);
}

LevelStatistics& LevelStatistics::operator=(const LevelStatistics&) = default;

LevelStatistics& LevelStatistics::operator=(LevelStatistics&& src) noexcept {
    if (this == &src)
        return *this;

    m_first = std::move(src.m_first);
    m_availableLevelCount = src.m_availableLevelCount;
    m_levelCompleted = std::move(src.m_levelCompleted);
    m_levelGameCounts = std::move(src.m_levelGameCounts);
    m_levelScores = std::move(src.m_levelScores);
    m_totalGameCount = src.m_totalGameCount;
    m_totalScore = src.m_totalScore;

    src.m_first.fill(0);
    src.m_availableLevelCount = 0;
    src.m_totalGameCount = 0;
    src.m_totalScore = 0;

    return *this;
}

}