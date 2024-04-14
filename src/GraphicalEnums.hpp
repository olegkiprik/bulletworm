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

#ifndef GRAPHICAL_ENUMS_HPP
#define GRAPHICAL_ENUMS_HPP

namespace Bulletworm {

enum class ScreenMode {
	Corner,
	Horizontal,
	Vertical,
	Count
};

enum class ColorDst {
	LogoTheme,
	MenuButtonPlain,
	MenuButtonHover,
	MenuButtonPressed,
	VolumeFill,
	VolumeOutline,
	ButtonEnabled,
	ButtonDisabled,
	BooleanButtonOutline,
	FloatingPointerFill,
	FloatingPointerOutline,
	SettingSignFill,
	SettingOkFill,
	FloatingPointerFillHover,
	ButtonDisabledHover,
	ButtonEnabledHover,
	SettingOkFillHover,
	BooleanButtonPressed,
	SettingOkPressed,
	LevelStats,
	LevelShapeOutline,
	LevelShapeCompleted,
	LevelShapeNCavailable,
	LevelShapeNonAvailable,
	Background,
	FruitChallengeVisual,
	BonusChallengeVisual,
	SuperbonusChallengeVisual,
	ChallengeVisualOutline,
	ChallengeVisualOutlineFill,
	F2Bvisual,
	F2BvisualOutline,
	F2BvisualOutlineFill,
	B2Svisual,
	B2SvisualOutline,
	B2SvisualOutlineFill,
	LevelCompletedParticleFirst,
	LevelCompletedParticleSecond,
	EffectEndedParticleFirst,
	EffectEndedParticleSecond,
	SpikesParticleFirst,
	SpikesParticleSecond,
	TimeLimitExceedParticleFirst,
	TimeLimitExceedParticleSecond,
	AcceleratedDefaultParticleFirst,
	AcceleratedDefaultParticleSecond,
	AcceleratedDownParticleFirst,
	AcceleratedDownParticleSecond,
	AcceleratedUpParticleFirst,
	AcceleratedUpParticleSecond,
	BonusEatenParticleFirst,
	BonusEatenParticleSecond,
	FruitEatenParticleFirst,
	FruitEatenParticleSecond,
	SuperbonusEatenParticleFirst,
	SuperbonusEatenParticleSecond,
	StoppedParticleFirst,
	StoppedParticleSecond,
	SnakeBodyFill,
	SnakeBodyOutline,
	SnakePointerFill,
	SnakePointerOutline,
	Score,
	HighestScore,
	Count
};

enum class FontType {
	Menu = 0, // buttons
	LevelStatistics = 0,
	Manual = 0,
	Plain = 0, // other uses
	Count
};

// texture as resource
enum class TextureUnit {
	ScreenCorner,
	ScreenHorizontal,
	Fruit,
	Bonus,
	PowerUnknown,
	PowerSlowDown,
	PowerTailHarmless,
	PowerInstantTailCut,
	Obstacle,
	SpikesClosed,
	SpikesOpened,
	RotorWeak,
	RotorStrong,
	Stopper,
	AccDefault,
	AccDown,
	AccUp,
	Bridge,
	TubeStraight,
	TubeRotated,
	CombinedTubeCross,
	CombinedTubeRotated,
	Void,
	Pointer,
	CombinedPointerRotated,
	CombinedRotorStrongCross,
	CombinedRotorStrongRotated,
	RandomAccelerator,
	RandomDihotomicAccelerator,
	Count
};

enum class VisualEffect {
	// snake
	SnakeDefault,
	SnakeSlow,
	SnakeFast,
	SnakeSlowDown,
	SnakeTailHarmless,
	SnakeStopped,
	SnakeTimeLimitExceed,

	// screen
	ScreenDefault,
	ScreenTimeLimitExceed,
	ScreenTimeLimitWarning,

	// fruit
	FruitDefault,
	FruitScreen,

	// bonus
	BonusDefault,
	BonusScreen,
	BonusWarning,
	BonusScreenWarning,

	// powerup
	PowerupDefault,
	PowerupScreen,
	PowerupWarning,
	PowerupScreenWarning,

	// challenge visual
	ChallengeVisualDefault,
	ChallengeVisualComplete,

	Count
};

constexpr int ColorDstCount = static_cast<int>(ColorDst::Count);
constexpr int FontCount = static_cast<int>(FontType::Count);
constexpr int TextureUnitCount = static_cast<int>(TextureUnit::Count);
constexpr int VisualEffectCount = static_cast<int>(VisualEffect::Count);
constexpr int ScreenModeCount = static_cast<int>(ScreenMode::Count);

}

#endif // GRAPHICAL_ENUMS_HPP
