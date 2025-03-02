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

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdint>

namespace Bulletworm {

constexpr std::size_t NrParticles = 200;

// text
const char* const GameTitle = "Bulletworm";
const char* const ShaderTimeTitle = "time";

// textures
constexpr unsigned int TexSz = 128;
constexpr unsigned int TexUnitWidth = 8;
constexpr unsigned int ThemeCount = 4;

// music
constexpr unsigned int MenuMusicId = 0;
constexpr unsigned int LevelStatsMusicId = 1;

// some limits
constexpr unsigned int LevelCountMin = 1;
constexpr unsigned int LevelCountMax = 150;
constexpr unsigned int DiffCountMin = 2;
constexpr unsigned int DiffCountMax = 5;

constexpr std::int64_t TimeLimitMaxAsMcs = (std::int64_t)60 * 60 * 1000 * 1000;
constexpr std::int64_t TimeLimitMinAsMcs = (std::int64_t)15 * 1000 * 1000;
constexpr std::int64_t SnakePeriodMinAsMcs = (std::int64_t)10 * 1000;
constexpr std::int64_t SnakePeriodMaxAsMcs = (std::int64_t)30 * 1000 * 1000;

constexpr unsigned int BonusCountToPowerupMax = 100;
constexpr unsigned int FruitCountToBonusMax = 100;

constexpr std::uint32_t WidthMin = 3;
constexpr std::uint32_t WidthMax = 0x1000;
constexpr std::uint32_t HeightMin = 3;
constexpr std::uint32_t HeightMax = 0x1000;

// window config
constexpr unsigned int ScaleYieldNumerator = 39;
constexpr unsigned int ScaleYieldDenominator = 40;

constexpr unsigned int WindowModeRatioNumerator = 3;
constexpr unsigned int WindowModeRatioDenominator = 4;

// 2560 -> 1920
// 1920 -> 1440
// 1440 -> 1080
// 1080 -> 810

constexpr std::uint64_t StatusHillEncryptionModulus = 65537;

constexpr std::size_t NrWallpaperQualities = 6;

}

#endif // CONSTANTS_HPP
