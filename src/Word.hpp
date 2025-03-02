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

#ifndef WORD_HPP
#define WORD_HPP

namespace Bulletworm {
	// Spaces only. You can change some of them to endlines.
enum class Word {
	LanguageName,

	PlayMainMenu,
	SettingsMainMenu,
	ManualMainMenu,
	LanguagesMainMenu,
	ExitFromMainMenu,
	ContinuePauseMenu,
	SettingsPauseMenu,
	ManualPauseMenu,
	OpenMainMenuPauseMenu,
	ExitFromBlockSnakePauseMenu,
	OkSettings,
	ManualText,
	OkManual,
	OkLanguages,
	LevelStatsLS,
	EatenLS,

	FruitsSingleLS,
	FruitsDualLS,
	FruitsPaucalLS,
	FruitsRSingleLS,
	FruitsRDualLS,
	FruitsRPaucalLS,
	FruitsPluralLS,

	BonusesSingleLS,
	BonusesDualLS,
	BonusesPaucalLS,
	BonusesRSingleLS,
	BonusesRDualLS,
	BonusesRPaucalLS,
	BonusesPluralLS,

	PowerupsSingleLS,
	PowerupsDualLS,
	PowerupsPaucalLS,
	PowerupsRSingleLS,
	PowerupsRDualLS,
	PowerupsRPaucalLS,
	PowerupsPluralLS,

	GoneLS,

	StepsSingleLS,
	StepsDualLS,
	StepsPaucalLS,
	StepsRSingleLS,
	StepsRDualLS,
	StepsRPaucalLS,
	StepsPluralLS,

	GameTimeLS,

	WeeksSingleLS,
	WeeksDualLS,
	WeeksPaucalLS,
	WeeksRSingleLS,
	WeeksRDualLS,
	WeeksRPaucalLS,
	WeeksPluralLS,

	DaysSingleLS,
	DaysDualLS,
	DaysPaucalLS,
	DaysRSingleLS,
	DaysRDualLS,
	DaysRPaucalLS,
	DaysPluralLS,

	HoursSingleLS,
	HoursDualLS,
	HoursPaucalLS,
	HoursRSingleLS,
	HoursRDualLS,
	HoursRPaucalLS,
	HoursPluralLS,

	MinutesSingleLS,
	MinutesDualLS,
	MinutesPaucalLS,
	MinutesRSingleLS,
	MinutesRDualLS,
	MinutesRPaucalLS,
	MinutesPluralLS,

	SecondsSingleLS,
	SecondsDualLS,
	SecondsPaucalLS,
	SecondsRSingleLS,
	SecondsRDualLS,
	SecondsRPaucalLS,
	SecondsPluralLS,

	TotalGameStatsLS,
	LevelCompletedLS,
	ExitFromBlockSnakeLS,
	RestartTheLevelLS,
	OpenMainMenuLS,

	SelectTheLevel,
	MusicVolume,
	SoundVolume,
	Fullscreen,

	GameCountLS,
	ScoreLS,
	ScoreSumLS,

	AmbientVolume,
	BonusLifetime,
	SuperbonusLifetime,
	EffectLifetime,
	TimeLeft,
	ResetStatistics,
	StatusNotSaved,
	VanillaTheme,
	WaterTheme,
	FireTheme,
	CursedTheme,
	AcceleratorDefault,
	AcceleratorDown,
	AcceleratorUp,
	SpontaneousAcceleratorUpDown,
	SpontaneousAcceleratorUpDownDefault,
	BonusItem,
	FruitItem,
	SuperbonusItem,
	TailResistanceItem,
	TailCollapseItem,
	SlowdownItem,
	Bridge,
	Stopper,
	Obstacle,
	Spikes,
	Screens,
	Tubes,
	RotorWeak,
	Pointer,
	RotorStrong,
	CombinedTube,
	CombinedRotorStrong,
	CombinedPointer,
	Reserved34,
	Reserved35,
	Reserved36,
	Reserved37,
	Reserved38,
	Reserved39,
	Reserved40,
	Reserved41,
	Reserved42,
	Reserved43,
	Reserved44,
	Reserved45,
	Reserved46,
	Reserved47,
	Reserved48,
	Reserved49,
	Reserved50,
	Reserved51,
	Reserved52,
	Reserved53,
	Reserved54,
	Reserved55,
	Reserved56,
	Reserved57,
	Reserved58,
	Reserved59,
	Reserved60,
	Reserved61,
	Reserved62,
	Reserved63,
	Reserved64,
	Reserved65,
	Reserved66,
	Reserved67,
	Reserved68,
	Reserved69,
	Reserved70,
	Reserved71,
	Reserved72,
	Reserved73,

	Count
};

constexpr int WordCount = (int)Word::Count;
}

#endif