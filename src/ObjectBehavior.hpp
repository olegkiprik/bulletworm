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

#ifndef OBJECT_BEHAVIOR_HPP
#define OBJECT_BEHAVIOR_HPP
#include "ObjectEnums.hpp"
#include <vector>
#include <optional>
#include <bitset>
#include <vector>
#include <cstdint>

namespace Bulletworm {

enum class Direction;
enum class Acceleration;
class Randomizer;

class ObjectBehavior {
public:

    struct ExecutionArguments {
        Direction previousSnakeDirection{};
        Randomizer* randomizer = nullptr;
        std::uint32_t parameter{};
    };

    // The execution environment
    struct ExecutionTarget {
        Acceleration snakeAcceleration;
        Direction snakeDirection;
        bool alive;
        bool moving;
        std::uint32_t remembered;
    };

    struct CompileParameters {
        // TODO: explain CompileParameters
        std::uint32_t const* const* condExpressions = nullptr;
        std::uint32_t const* const* modifyExpressions = nullptr;
        ObjectCommand const* commands = nullptr;
        std::size_t   const* condExpressionSizes = nullptr;
        std::size_t   const* modifyExpressionSizes = nullptr;
        std::size_t                 conditionCount = 0;
    };

    ObjectBehavior(const ObjectBehavior&) = default;
    ObjectBehavior(ObjectBehavior&&) noexcept;

    ObjectBehavior& operator=(const ObjectBehavior&) = default;
    ObjectBehavior& operator=(ObjectBehavior&&) noexcept;

    /// Generates a 'void' behavior
    ObjectBehavior() noexcept;

    [[nodiscard]] std::optional<std::string> compile(const CompileParameters& parameters);

    void activate(ExecutionTarget& target, const ExecutionArguments& arguments) const;

    /// Get type of parameter that the object require.
    ObjectParameterType getParameterType() const noexcept;

    bool getProperty(ObjectProperty prop) const noexcept;

private:

    enum class StackValueType {
        Integer,
        Acceleration,
        Direction,
        DoubleDirection,
        CombinedDirection
    };

    /// Effect states to define the program attributes
    struct EffectAttributeStates {
        bool requiresRandom = false;
        ObjectParameterType paramType = ObjectParameterType::NoParameter;
    };

    [[nodiscard]] static std::optional<std::string>
        validateValueExpression(StackValueType type,
                                const std::uint32_t* program,
                                std::size_t keywordCount,
                                EffectAttributeStates& states);

    static std::uint32_t computeValueExpression(const std::uint32_t* program,
                                                std::size_t keywordCount,
                                                const ExecutionTarget& target,
                                                const ExecutionArguments& arguments);

    using Expression = std::vector<std::uint32_t>;

    std::vector<Expression> m_conditionExpressions;           // doesn't have any empty conditions!
    std::vector<Expression> m_modifyExpressions;              // expressions can be empty, but the vector can't be
    std::vector<ObjectCommand> m_commands;          // if it's empty, there is 'empty behavior'

    std::bitset<ObjectPropertyCount> m_properties;
    ObjectParameterType m_parameterType;            //!< The parameter type that the object requires
};

} // namespace Bulletworm

#endif // !OBJECT_BEHAVIOR_HPP