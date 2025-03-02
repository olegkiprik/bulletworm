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

#ifndef ATTRIB_ENUMS_HPP
#define ATTRIB_ENUMS_HPP

namespace Bulletworm {
	// First level statistics
enum class FirstLevelStatisticsEnum {
	DiffCount,
	LevelCount,
	TotalGametimeLeast32, // total gametime
	TotalGametimeMost32, // total gametime
	Count
};

enum class SettingEnum {
	oddSetting,
	oddSetting1,
	LanguageIndex, // language index
	SoundVolumePer10000, // sound volume [0 - 10000]
	MusicVolumePer10000, // music volume [0 - 10000]
	AmbientVolumePer10000, // ambient volume [0 - 10000]
	FullscreenEnabled, // fullscreen
	SnakeHeadPointerEnabled, // head pointer on
	Count
};

// level count map
enum class LevelCountMap {
	ObjPair,
	Param,
	Memory,
	Theme,
	SnakeStartPos,
	Count,
};

enum class LevelAttribEnum {
	TailSize,
	TailMaxSize,
	TailCollapseMaxSize, // min size*
	TailGrowth,
	FruitCountToBonus,
	BonusCountToSuperbonus,
	SnakePeriod, // mcs
	BonusLifetime, // mcs
	SuperbonusLifetime, // mcs
	TimeLimit, // mcs
	SlowDownNumerator,
	SlowDownDenominator,
	AccelDownNumerator,
	AccelDownDenominator,
	AccelUpNumerator,
	AccelUpDenominator,
	FruitCount,
	Count
};

enum class LevelPlotDataEnum {
	BonusScaleVisible,
	SuperbonusScaleVisible,
	SuperbonusVisible,
	EffectScaleVisible,
	TimeLimitScaleVisible,
	BonusScreenProjectionDistance,
	SuperbonusScreenProjectionDistance,
	FruitScreenProjectionDistance,
	FruitCountToBonusVisible,
	BonusCountToSuperbonusVisible,
	SnakeSightX,
	SnakeSightY,
	MusicIndex,
	BackgroundIndex,
	Challenge,
	ChallengeCount,
	FruitScoreCoeff,
	BonusScoreCoeff,
	SuperbonusScoreCoeff,

	ScreenTheme,
	FruitTheme,
	BonusTheme,
	SuperbonusTheme,

	FoggColor,

	FoggBlendSrcColor,
	FoggBlendDstColor,
	FoggBlendColorEq,
	FoggBlendSrcAlpha,
	FoggBlendDstAlpha,
	FoggBlendAlphaEq,

	MusicEnabled,
	AmbientEnabled,
	AmbientIndex,

	Count
};

constexpr int SettingCount = static_cast<int>(SettingEnum::Count);
constexpr int LevelCountMapCount = static_cast<int>(LevelCountMap::Count);
constexpr int FirstLevelStatisticsCount = static_cast<int>(FirstLevelStatisticsEnum::Count);
constexpr int LevelAttribCount = static_cast<int>(LevelAttribEnum::Count);
constexpr int LevelPlotDataCount = static_cast<int>(LevelPlotDataEnum::Count);
}

#endif // !ATTRIB_ENUMS_HPP