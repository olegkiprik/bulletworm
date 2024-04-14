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

#include "ObjectBehavior.hpp"
#include "Randomizer.hpp"
#include "ObjectParameterEnums.hpp"
#include "ObjParamEnumUtility.hpp"
#include <forward_list>
#include <cassert>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectBehavior::ObjectBehavior() noexcept :
    m_properties(),
    m_parameterType(ObjectParameterType::NoParameter),
    m_commands(),
    m_conditionExpressions(),
    m_modifyExpressions() {}


////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectBehavior::ObjectBehavior(ObjectBehavior&& src) noexcept :
    m_commands(std::move(src.m_commands)),
    m_conditionExpressions(std::move(src.m_conditionExpressions)),
    m_modifyExpressions(std::move(src.m_modifyExpressions)),
    m_parameterType(src.m_parameterType),
    m_properties(std::move(src.m_properties)) {
    src.m_properties.reset();
    src.m_parameterType = ObjectParameterType::NoParameter;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectBehavior& ObjectBehavior::operator=(ObjectBehavior&& src) noexcept {
    if (this == &src)
        return *this;

    m_commands = std::move(src.m_commands);
    m_conditionExpressions = std::move(src.m_conditionExpressions);
    m_modifyExpressions = std::move(src.m_modifyExpressions);
    m_parameterType = src.m_parameterType;
    m_properties = std::move(src.m_properties);

    src.m_parameterType = ObjectParameterType::NoParameter;
    src.m_properties.reset();

    return *this;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::string> ObjectBehavior::compile(const CompileParameters& parameters) {
    assert(parameters.commands);
    assert(parameters.modifyExpressions);
    assert(parameters.modifyExpressionSizes);

    // The states to define the program attributes
    EffectAttributeStates states;
    bool impacts = false;
    bool dangerous = false;

    for (std::size_t i = 0; i < parameters.conditionCount; ++i) {
        std::optional<std::string> log{ validateValueExpression(StackValueType::Integer,
            parameters.condExpressions[i], parameters.condExpressionSizes[i], states) };

        if (log) return log;
    }

    for (std::size_t i = 0; i < parameters.conditionCount + 1; ++i) {
        bool currentCommandModifies =
            parameters.commands[i] == ObjectCommand::ModifyAcceleration ||
            parameters.commands[i] == ObjectCommand::ModifyDirection;

        if (currentCommandModifies || parameters.commands[i] == ObjectCommand::Remember) {
            StackValueType checkType{};

            switch (parameters.commands[i]) {
            case ObjectCommand::ModifyAcceleration:
                checkType = StackValueType::Acceleration;
                break;
            case ObjectCommand::ModifyDirection:
                checkType = StackValueType::Direction;
                break;
            case ObjectCommand::Remember:
                checkType = StackValueType::Integer;
                break;
            default:
                break;
            }

            std::optional<std::string> log{ validateValueExpression(checkType,
                parameters.modifyExpressions[i], parameters.modifyExpressionSizes[i], states) };

            if (log)
                return log;
        }

        if (currentCommandModifies || parameters.commands[i] == ObjectCommand::StopSnake)
            impacts = true;

        if (parameters.commands[i] == ObjectCommand::KillSnake)
            dangerous = true;
    }

    // compilation successful

    m_properties[(std::size_t)ObjectProperty::ImpactsToSnake] = impacts;
    m_properties[(std::size_t)ObjectProperty::IsDangerous] = dangerous;
    m_properties[(std::size_t)ObjectProperty::RequiresRandom] = states.requiresRandom;
    m_parameterType = states.paramType;

    m_commands.assign(parameters.commands, parameters.commands + parameters.conditionCount + 1);
    m_conditionExpressions.resize(parameters.conditionCount);
    m_modifyExpressions.resize(parameters.conditionCount + 1);

    for (std::size_t i = 0; i < parameters.conditionCount; ++i)
        m_conditionExpressions[i].assign(parameters.condExpressions[i],
                                         parameters.condExpressions[i] + 
                                         parameters.condExpressionSizes[i]);

    for (std::size_t i = 0; i < parameters.conditionCount + 1; ++i) {
        if (parameters.modifyExpressionSizes[i]) {
            m_modifyExpressions[i].assign(parameters.modifyExpressions[i],
                                          parameters.modifyExpressions[i] + 
                                          parameters.modifyExpressionSizes[i]);
        } else {
            Expression().swap(m_modifyExpressions[i]);
        }
    }

    return {};
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjectBehavior::activate(ExecutionTarget& target, const ExecutionArguments& arguments) const {
    if (m_commands.empty())
        return;

    std::size_t commandIndex = 0;
    while (commandIndex < m_conditionExpressions.size()) {
        const auto& currentExpression = m_conditionExpressions[commandIndex];

        if (computeValueExpression(currentExpression.data(),
            currentExpression.size(), target, arguments))
            break;

        ++commandIndex;
    }

    // If without else
    if (commandIndex >= m_commands.size())
        return;

    const auto& activeModifyExpression = m_modifyExpressions[commandIndex];

    switch (m_commands[commandIndex]) {
    case ObjectCommand::KillSnake:
        target.alive = false;
        break;
    case ObjectCommand::StopSnake:
        target.moving = false;
        break;
    case ObjectCommand::ModifyAcceleration:
        target.snakeAcceleration =
            (Acceleration)computeValueExpression(activeModifyExpression.data(),
                                      activeModifyExpression.size(), target, arguments);
        break;
    case ObjectCommand::ModifyDirection:
        target.snakeDirection =
            (Direction)computeValueExpression(activeModifyExpression.data(),
                                   activeModifyExpression.size(), target, arguments);
        break;
    case ObjectCommand::Remember:
        target.remembered = computeValueExpression(activeModifyExpression.data(),
                                                   activeModifyExpression.size(), target, arguments);
        break;
    default:
        break;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
std::uint32_t ObjectBehavior::computeValueExpression(
    const std::uint32_t* expression,
    std::size_t keywordCount,
    const ExecutionTarget& target,
    const ExecutionArguments& arguments) {
    std::forward_list<std::uint32_t> stack;
    std::size_t pointer = 0;

    bool isInteger = false;

    bool again = true;
    while (again) {
        if (pointer >= keywordCount)
            break;

        if (isInteger) {
            stack.push_front(expression[pointer]);
            isInteger = false;
        } else {
            switch ((ObjectBehaviorKeyword)expression[pointer]) {
            case ObjectBehaviorKeyword::AccelerationDefault:
                stack.push_front((std::uint32_t)Acceleration::Default);
                break;
            case ObjectBehaviorKeyword::AccelerationDown:
                stack.push_front((std::uint32_t)Acceleration::Down);
                break;
            case ObjectBehaviorKeyword::AccelerationUp:
                stack.push_front((std::uint32_t)Acceleration::Up);
                break;
            case ObjectBehaviorKeyword::RandomAcceleration:
                stack.push_front((std::uint32_t)arguments.randomizer->get(0, 
                                 (std::uint64_t)AccelerationCount - 1));
                break;
            case ObjectBehaviorKeyword::RandomCombinedDirection:
                stack.push_front((std::uint32_t)arguments.randomizer->get(0, 
                                 (std::uint64_t)CombinedTubeCount - 1));
                break;
            case ObjectBehaviorKeyword::RandomDirection:
                stack.push_front((std::uint32_t)arguments.randomizer->get(0, 
                                 (std::uint64_t)DirectionCount - 1));
                break;
            case ObjectBehaviorKeyword::RandomDoubleDirection:
                stack.push_front((std::uint32_t)arguments.randomizer->get(0, 
                                 (std::uint64_t)DoubleDirectionCount - 1));
                break;
            case ObjectBehaviorKeyword::IntRandomValue:
                stack.front() = (std::uint32_t)arguments.randomizer->get(0,
                                                                         stack.front());
                break;
            case ObjectBehaviorKeyword::RememberedInt:
                stack.push_front(target.remembered);
                break;
            case ObjectBehaviorKeyword::Not:
                stack.front() = static_cast<std::uint32_t>(!static_cast<bool>(stack.front()));
                break;
            case ObjectBehaviorKeyword::OppositeDirection:
                stack.front() = (std::uint32_t)oppositeDirection(Direction(stack.front()));
                break;
            case ObjectBehaviorKeyword::OppositeAcceleration:
                stack.front() = (std::uint32_t)oppositeAcceleration((Acceleration)stack.front());
                break;
            case ObjectBehaviorKeyword::Or:
            {
                bool reserved = stack.front();
                stack.pop_front();
                stack.front() =
                    static_cast<std::uint32_t>(reserved || static_cast<bool>(stack.front()));
                break;
            }
            case ObjectBehaviorKeyword::And:
            {
                bool reserved = stack.front();
                stack.pop_front();
                stack.front() =
                    static_cast<std::uint32_t>(reserved && static_cast<bool>(stack.front()));
                break;
            }
            case ObjectBehaviorKeyword::Equal:
            {
                std::uint32_t reserved = stack.front();
                stack.pop_front();
                stack.front() = std::uint32_t(stack.front() == reserved);
                break;
            }
            case ObjectBehaviorKeyword::Select:
            {
                bool selectFarther = stack.front();
                stack.pop_front();

                if (selectFarther)
                    stack.pop_front();
                else
                    stack.erase_after(stack.begin());

                break;
            }
            case ObjectBehaviorKeyword::IsDirExitOfDoubleDir:
            {
                Direction reserved = (Direction)stack.front();
                stack.pop_front();
                stack.front() = (std::uint32_t)directionIsExit((DoubleDirection)stack.front(), reserved);
                break;
            }
            case ObjectBehaviorKeyword::GetCombDirExit:
            {
                CombinedDirection reserved = (CombinedDirection)stack.front();
                stack.pop_front();
                stack.front() = (std::uint32_t)getCombinedTubeExit(reserved, (Direction)stack.front());
                break;
            }
            case ObjectBehaviorKeyword::SnakeAcceleration:
                stack.push_front((std::uint32_t)target.snakeAcceleration);
                break;
            case ObjectBehaviorKeyword::SnakeDirection:
                stack.push_front((std::uint32_t)target.snakeDirection);
                break;
            case ObjectBehaviorKeyword::PreviousSnakeDirection:
                stack.push_front((std::uint32_t)arguments.previousSnakeDirection);
                break;
            case ObjectBehaviorKeyword::ParamAcceleration:
            case ObjectBehaviorKeyword::ParamDirection:
            case ObjectBehaviorKeyword::ParamDoubleDirection:
            case ObjectBehaviorKeyword::ParamCombinedDirection:
                stack.push_front(arguments.parameter);
                break;
            case ObjectBehaviorKeyword::Int:
                isInteger = true;
                break;
            case ObjectBehaviorKeyword::IntAdd:
            {
                std::uint32_t intval = stack.front();
                stack.pop_front();
                stack.front() += intval;
                break;
            }
            case ObjectBehaviorKeyword::IntSubtract:
            {
                std::uint32_t intval = stack.front();
                stack.pop_front();
                stack.front() -= intval;
                break;
            }
            case ObjectBehaviorKeyword::IntAddOverflow:
            {
                std::uint32_t intval = stack.front();
                stack.pop_front();
                stack.front() = (UINT32_MAX - intval < stack.front());
                break;
            }
            case ObjectBehaviorKeyword::IntBitAnd:
            {
                std::uint32_t intval = stack.front();
                stack.pop_front();
                stack.front() &= intval;
                break;
            }
            case ObjectBehaviorKeyword::IntBitNot:
                stack.front() = ~stack.front();
                break;
            case ObjectBehaviorKeyword::IntBitOr:
            {
                std::uint32_t intval = stack.front();
                stack.pop_front();
                stack.front() |= intval;
                break;
            }
            case ObjectBehaviorKeyword::IntBitXor:
            {
                std::uint32_t intval = stack.front();
                stack.pop_front();
                stack.front() ^= intval;
                break;
            }
            case ObjectBehaviorKeyword::IntCountOfOnes:
            {
                unsigned int howmany = 0;
                for (std::uint32_t i = 1; i; i <<= 1) {
                    if (stack.front() & i)
                        ++howmany;
                }
                stack.front() = howmany;
                break;
            }
            case ObjectBehaviorKeyword::IntCyclicLeftShift:
            {
                std::uint32_t intmod = stack.front();
                stack.pop_front();
                intmod %= 32;
                std::uint32_t intsrc = stack.front();
                stack.front() <<= intmod;
                intsrc >>= (32 - intmod);
                stack.front() |= intsrc;
                break;
            }
            case ObjectBehaviorKeyword::IntCyclicRightShift:
            {
                std::uint32_t intmod = stack.front();
                stack.pop_front();
                intmod %= 32;
                std::uint32_t intsrc = stack.front();
                stack.front() >>= intmod;
                intsrc <<= (32 - intmod);
                stack.front() |= intsrc;
                break;
            }
            case ObjectBehaviorKeyword::IntDivideAndFloor:
            {
                std::uint32_t divisor = stack.front();
                stack.pop_front();
                if (divisor == 0)
                    stack.front() = 0;
                else
                    stack.front() /= divisor;
                break;
            }
            case ObjectBehaviorKeyword::IntLess:
            {
                std::uint32_t rightVal = stack.front();
                stack.pop_front();
                stack.front() = (stack.front() < rightVal);
                break;
            }
            case ObjectBehaviorKeyword::IntLogicalLeftShift:
            {
                std::uint32_t intval = stack.front();
                stack.pop_front();
                stack.front() <<= intval;
                break;
            }
            case ObjectBehaviorKeyword::IntLogicalRightShift:
            {
                std::uint32_t intval = stack.front();
                stack.pop_front();
                stack.front() >>= intval;
                break;
            }
            case ObjectBehaviorKeyword::IntMinus:
                stack.front() = UINT32_MAX - stack.front();
                break;
            case ObjectBehaviorKeyword::IntModulo:
            {
                std::uint32_t divisor = stack.front();
                stack.pop_front();
                if (divisor == 0)
                    stack.front() = 0;
                else
                    stack.front() %= divisor;
                break;
            }
            case ObjectBehaviorKeyword::IntMultiply:
            {
                std::uint32_t intval = stack.front();
                stack.pop_front();
                stack.front() *= intval;
                break;
            }
            case ObjectBehaviorKeyword::IntMultiplyOverflow:
            {
                std::uint64_t intval = stack.front();
                stack.pop_front();
                std::uint64_t product64 = intval * stack.front();
                stack.front() = (product64 > UINT32_MAX);
                break;
            }
            case ObjectBehaviorKeyword::ExpressionEnd:
            default:
                again = false;
                break;
            }
        }

        ++pointer;
    }

    return stack.front();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::string> ObjectBehavior::validateValueExpression(StackValueType type,
                                                                    const std::uint32_t* expression,
                                                                    std::size_t keywordCount,
                                                                    EffectAttributeStates& states) {
    assert(expression && keywordCount);

    std::forward_list<StackValueType> stack;
    std::size_t pointer = 0;

    bool isInteger = false;

    bool again = true;
    while (again) {
        if (pointer >= keywordCount)
            break;

        if (isInteger) {
            stack.push_front(StackValueType::Integer);
            isInteger = false;
        } else {
            switch ((ObjectBehaviorKeyword)expression[pointer]) {
            case ObjectBehaviorKeyword::AccelerationDown:
            case ObjectBehaviorKeyword::AccelerationDefault:
            case ObjectBehaviorKeyword::AccelerationUp:
                stack.push_front(StackValueType::Acceleration);
                break;

            case ObjectBehaviorKeyword::RandomAcceleration:
                stack.push_front(StackValueType::Acceleration);
                states.requiresRandom = true;
                break;

            case ObjectBehaviorKeyword::IntRandomValue:

                if (stack.empty() || stack.front() != StackValueType::Integer)
                    return "TODO";

                states.requiresRandom = true;
                break;

            case ObjectBehaviorKeyword::RandomCombinedDirection:
                stack.push_front(StackValueType::CombinedDirection);
                states.requiresRandom = true;
                break;

            case ObjectBehaviorKeyword::RandomDirection:
                stack.push_front(StackValueType::Direction);
                states.requiresRandom = true;
                break;

            case ObjectBehaviorKeyword::RandomDoubleDirection:
                stack.push_front(StackValueType::DoubleDirection);
                states.requiresRandom = true;
                break;

            case ObjectBehaviorKeyword::RememberedInt:

                stack.push_front(StackValueType::Integer);
                break;

            case ObjectBehaviorKeyword::OppositeDirection:
                if (stack.empty() || stack.front() != StackValueType::Direction)
                    return "Lack of value in the stack (Direction)";

                break;

            case ObjectBehaviorKeyword::OppositeAcceleration:
                if (stack.empty() || stack.front() != StackValueType::Acceleration)
                    return "Lack of value in the stack (Acceleration)";

                break;

            case ObjectBehaviorKeyword::Or:
            case ObjectBehaviorKeyword::And:
            case ObjectBehaviorKeyword::IntAdd:
            case ObjectBehaviorKeyword::IntAddOverflow:
            case ObjectBehaviorKeyword::IntBitAnd:
            case ObjectBehaviorKeyword::IntBitOr:
            case ObjectBehaviorKeyword::IntBitXor:
            case ObjectBehaviorKeyword::IntCyclicLeftShift:
            case ObjectBehaviorKeyword::IntCyclicRightShift:
            case ObjectBehaviorKeyword::IntDivideAndFloor:
            case ObjectBehaviorKeyword::IntLogicalLeftShift:
            case ObjectBehaviorKeyword::IntLogicalRightShift:
            case ObjectBehaviorKeyword::IntModulo:
            case ObjectBehaviorKeyword::IntMultiply:
            case ObjectBehaviorKeyword::IntMultiplyOverflow:
            case ObjectBehaviorKeyword::IntSubtract:
            case ObjectBehaviorKeyword::IntLess:
                if (stack.empty() || stack.front() != StackValueType::Integer)
                    return "Lack of value in the stack (Int)";

                stack.pop_front();
                [[fallthrough]];

            case ObjectBehaviorKeyword::Not:
            case ObjectBehaviorKeyword::IntBitNot:
            case ObjectBehaviorKeyword::IntCountOfOnes:
            case ObjectBehaviorKeyword::IntMinus:
                if (stack.empty() || stack.front() != StackValueType::Integer)
                    return "Lack of value in the stack (Int)";

                break;

            case ObjectBehaviorKeyword::Equal:
            {
                if (stack.empty())
                    return "Lack of value in the stack (empty)";

                StackValueType currentType = stack.front();
                stack.pop_front();

                if (stack.empty() || currentType != stack.front())
                    return "Lack of value in the stack: wrong type";

                if (currentType != StackValueType::Integer) {
                    stack.pop_front();
                    stack.push_front(StackValueType::Integer);
                }

                break;
            }
            case ObjectBehaviorKeyword::Select:
            {
                if (stack.empty() || stack.front() != StackValueType::Integer)
                    return "Lack of value in the stack (Boolean)";

                stack.pop_front();

                if (stack.empty())
                    return "Lack of value in the stack (empty)";

                StackValueType currentType = stack.front();
                stack.pop_front();

                if (stack.empty() || currentType != stack.front())
                    return "Lack of value in the stack: wrong type";

                if (currentType != StackValueType::Integer) {
                    stack.pop_front();
                    stack.push_front(currentType);
                }

                break;
            }
            case ObjectBehaviorKeyword::IsDirExitOfDoubleDir:
                if (stack.empty() || stack.front() != StackValueType::Direction)
                    return "Lack of value in the stack (direction)";

                stack.pop_front();

                if (stack.empty() || stack.front() != StackValueType::DoubleDirection)
                    return "Lack of value in the stack (DoubleDirection)";

                stack.pop_front();
                stack.push_front(StackValueType::Integer);
                break;

            case ObjectBehaviorKeyword::GetCombDirExit:
                if (stack.empty() || stack.front() != StackValueType::CombinedDirection)
                    return "Lack of value in the stack (CombinedDirection)";

                stack.pop_front();

                if (stack.empty() || stack.front() != StackValueType::Direction)
                    return "Lack of value in the stack (direction)";

                break;

            case ObjectBehaviorKeyword::SnakeAcceleration:
                stack.push_front(StackValueType::Acceleration);
                break;

            case ObjectBehaviorKeyword::SnakeDirection:
            case ObjectBehaviorKeyword::PreviousSnakeDirection:
                stack.push_front(StackValueType::Direction);
                break;

            case ObjectBehaviorKeyword::ParamAcceleration:

                if (states.paramType != ObjectParameterType::Acceleration &&
                    states.paramType != ObjectParameterType::NoParameter)
                    return "Parameter corruption (acceleration)";

                states.paramType = ObjectParameterType::Acceleration;
                stack.push_front(StackValueType::Acceleration);
                break;

            case ObjectBehaviorKeyword::ParamDirection:

                if (states.paramType != ObjectParameterType::Direction &&
                    states.paramType != ObjectParameterType::NoParameter)
                    return "Parameter corruption (direction)";

                states.paramType = ObjectParameterType::Direction;
                stack.push_front(StackValueType::Direction);
                break;

            case ObjectBehaviorKeyword::ParamDoubleDirection:

                if (states.paramType != ObjectParameterType::DoubleDirection &&
                    states.paramType != ObjectParameterType::NoParameter)
                    return "Parameter corruption (double direction)";

                states.paramType = ObjectParameterType::DoubleDirection;
                stack.push_front(StackValueType::DoubleDirection);
                break;

            case ObjectBehaviorKeyword::ParamCombinedDirection:

                if (states.paramType != ObjectParameterType::CombinedDirection &&
                    states.paramType != ObjectParameterType::NoParameter)
                    return "Parameter corruption (combined direction)";

                states.paramType = ObjectParameterType::CombinedDirection;
                stack.push_front(StackValueType::CombinedDirection);
                break;

            case ObjectBehaviorKeyword::Int:
                isInteger = true;
                break;
            case ObjectBehaviorKeyword::ExpressionEnd:
            default:
                again = false;
                break;
            }
        }
        ++pointer;
    }

    if (stack.empty() || (stack.front() != type))
        return "Expression is invalid: stack is empty or returns wrong type";

    return {};
}

ObjectParameterType ObjectBehavior::getParameterType() const noexcept {
    return m_parameterType;
}


bool ObjectBehavior::getProperty(ObjectProperty prop) const noexcept {
    return m_properties[(std::size_t)prop];
}

} // namespace Bulletworm