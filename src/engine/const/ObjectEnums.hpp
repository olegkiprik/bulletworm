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

#ifndef OBJECT_ENUMS_HPP
#define OBJECT_ENUMS_HPP

namespace Bulletworm {

enum class ObjectEffect {
	Pre,
	Post
};

enum class ObjectCommand {
	NoCommand,
	Remember,
	ModifyDirection,
	ModifyAcceleration,
	StopSnake,
	KillSnake
};

enum class ObjectProperty {
	ImpactsToSnake,
	IsDangerous,
	RequiresRandom,
	Count
};

// Type of parameter that object require to execute
enum class ObjectParameterType {
	NoParameter,        // The object doesn't require any parameter (for example, the obstacle)
	Acceleration,       // The acceleration parameter type (for example, accelerator requires it)
	Direction,          // The direction parameter type (for example, rotor requires it)
	DoubleDirection,    // The double direction type (for example, tube requires it)
	CombinedDirection       // The combined direction type (for example, combined tube requires it)
};

// The object behavior program keyword
enum class ObjectBehaviorKeyword {
	ExpressionEnd,
	Int,                            // Unsigned integer value (32b)

	RememberedInt,                     // The integer value that the object remembered
	Not,                            // Function that inverts the logical value
	OppositeDirection,              // Function that returns the opposite direction
	OppositeAcceleration,           // Function that returns the opposite acceleration
	Or,                             // Function that returns logical disjunction of two values
	And,                            // Function that returns logical conjunction of two values
	Equal,                 // Function that returns true if the two arguments are equal, false otherwise
	IsDirExitOfDoubleDir, // Function that returns true if the direction is exit of the double direction
	GetCombDirExit,                 // Function that returns the exit direction of the combined one
	Select,                         // Function that selects the one of two values (?:)

	IntRandomValue, // with max value included
	RandomAcceleration,
	RandomDirection,
	RandomDoubleDirection,
	RandomCombinedDirection,

	AccelerationDown,
	AccelerationDefault,
	AccelerationUp,

	SnakeAcceleration,              // Current Snake acceleration keyword
	SnakeDirection,                 // Current Snake direction keyword
	PreviousSnakeDirection,         // Previous Snake direction keyword

	ParamBegin,                     // Iteration hint

	ParamAcceleration = ParamBegin, // Acceleration parameter
	ParamDirection,                 // Direction parameter
	ParamDoubleDirection,           // Double direction parameter
	ParamCombinedDirection,         // Combined direction parameter

	// new
	IntAdd,
	IntSubtract,
	IntMultiply,
	IntDivideAndFloor, // x / 0 = 0
	IntModulo, // x % 0 = 0
	IntMinus, // UINT32_MAX - x
	IntLogicalLeftShift,
	IntLogicalRightShift,
	IntCyclicLeftShift,
	IntCyclicRightShift,
	IntBitAnd,
	IntBitOr,
	IntBitXor,
	IntBitNot,
	IntCountOfOnes,
	IntAddOverflow,
	IntMultiplyOverflow,
	IntLess,

	Count
};

constexpr int ObjectPropertyCount = (int)ObjectProperty::Count;
constexpr int ObjectKeywordCount = (int)ObjectBehaviorKeyword::Count;

}

#endif // !OBJECT_ENUMS_HPP