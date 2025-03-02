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

#include "ObjectBehaviorLoader.hpp"
#include <SFML/System/InputStream.hpp>
#include <bw_ext/BasicUtility.hpp>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <bw_ext/Endianness.hpp>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::string> 
ObjectBehaviorLoader::loadFromStream(std::vector<ObjectBehavior>& objBehvrs,    
                                      sf::InputStream& stream, bool endiannessRequired) {
                                                                
    // kw map (from keyword to bin value, we need reversed one)
    std::array<std::uint32_t, ObjectKeywordCount> objectKwMap{};
    std::int64_t finstatus;

    finstatus = stream.read(objectKwMap.data(), sizeof(std::uint32_t) * ObjectKeywordCount);
    if (finstatus != sizeof(std::uint32_t) * ObjectKeywordCount)
        return "Object behavior keyword map opening failure";

    // endianness
    if (endiannessRequired) {
        std::for_each(objectKwMap.begin(), objectKwMap.end(),
                      [](std::uint32_t& v) {
                          v = n2hl(v);
                      });
    }

    // TODO: check the kw map
    std::unordered_map<std::uint32_t, ObjectBehaviorKeyword> objectKwRevMap;

    for (int i = 0; i < ObjectKeywordCount; ++i)
        objectKwRevMap[objectKwMap[i]] = ObjectBehaviorKeyword(i);

    // behavior
    StuffForCreating stuffFc;

    std::uint32_t inputData;

    bool isInteger = false;

    for (;;) {
        if (stuffFc.context == Context::Ended)
            break;

        finstatus = stream.read(&inputData, 4);

        if (finstatus == 0)
            break;

        if (finstatus != 4)
            return "Object behavior file opening failure";

        // endianness
        if (endiannessRequired) {
            inputData = n2hl(inputData);
        }

        stuffFc.input = inputData;

        switch (stuffFc.context) {
        case Context::InputingCommand:
            if (!stuffFc.inputCommand())
                return "Object behavior command input failure";

            break;
        case Context::InputingConditionExpr:
        case Context::InputingCommandExpr:

            if (isInteger) {
                if (stuffFc.context == Context::InputingConditionExpr)
                    stuffFc.condExp.back().push_back(stuffFc.input);
                else
                    stuffFc.modExp.back().push_back(stuffFc.input);

                isInteger = false;
            } else if (stuffFc.input != objectKwMap[(std::size_t)ObjectBehaviorKeyword::ExpressionEnd]) {
                auto found = objectKwRevMap.find(stuffFc.input);
                if (found == objectKwRevMap.end())
                    return "Object behavior stack value input failure";

                if (stuffFc.context == Context::InputingConditionExpr)
                    stuffFc.condExp.back().push_back((std::uint32_t)found->second);
                else
                    stuffFc.modExp.back().push_back((std::uint32_t)found->second);

                if (found->second == ObjectBehaviorKeyword::Int)
                    isInteger = true;
            } else {
                stuffFc.context = Context::KeywordExpected;
            }
            break;
        case Context::KeywordExpected:
        {
            std::optional<std::string> kwLog{ stuffFc.inputKeyword() };
            if (kwLog)
                return kwLog;

            break;
        }
        default:
            break;
        }
    }

    // success
    objBehvrs = std::move(stuffFc.objBehPrep);
    return {};
}


////////////////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::string> ObjectBehaviorLoader::StuffForCreating::createObject() {
    // if else filling
    if (obcommands.size() < condExp.size())
        return "Conditions > real commands";

    else if (obcommands.size() == condExp.size()) {
        obcommands.push_back(ObjectCommand::NoCommand);
        modExp.emplace_back();
    }

    ObjectBehavior::CompileParameters param;
    param.commands = obcommands.data();

    std::vector<const std::uint32_t*>
        condPtrs(condExp.size()),
        modPtrs(modExp.size());

    std::vector<std::size_t>
        condSizes(condExp.size()),
        modSizes(modExp.size());

    for (std::size_t i = 0; i < condExp.size(); ++i) {
        condPtrs[i] = condExp[i].data();
        condSizes[i] = condExp[i].size();
    }

    for (std::size_t i = 0; i < modExp.size(); ++i) {
        modPtrs[i] = modExp[i].data();
        modSizes[i] = modExp[i].size();
    }

    param.condExpressions = condPtrs.data();
    param.condExpressionSizes = condSizes.data();
    param.modifyExpressions = modPtrs.data();
    param.modifyExpressionSizes = modSizes.data();

    param.conditionCount = condExp.size();

    ObjectBehavior newbie;
    std::optional<std::string> newbieLog{ newbie.compile(param) };

    if (newbieLog)
        return "Object behavior compiling failure: " + newbieLog.value();

    objBehPrep.push_back(std::move(newbie));

    // clear
    condExp.clear();
    obcommands.clear();
    modExp.clear();
    return {};
}


bool ObjectBehaviorLoader::StuffForCreating::inputCommand() {
    switch ((ObjectCommand)input) {
    case ObjectCommand::ModifyAcceleration:
    case ObjectCommand::ModifyDirection:
    case ObjectCommand::Remember:
        context = Context::InputingCommandExpr;
        break;
    case ObjectCommand::KillSnake:
    case ObjectCommand::NoCommand:
    case ObjectCommand::StopSnake:
        context = Context::KeywordExpected;
        break;
    default:
        return false;
    }

    obcommands.push_back((ObjectCommand)input);

    // anyway!
    modExp.emplace_back();
    return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::string> ObjectBehaviorLoader::StuffForCreating::inputKeyword() {
    LoaderKeyWord kw = (LoaderKeyWord)input;

    switch (kw) {
    case LoaderKeyWord::End:
        context = Context::Ended;
        break;
    case LoaderKeyWord::Comma:
    {
        std::optional<std::string> nameLog{ createObject() };
        if (nameLog)
            return nameLog;

        break;
    }
    case LoaderKeyWord::Command:
        context = Context::InputingCommand;
        break;
    case LoaderKeyWord::Condition:
        condExp.emplace_back();
        context = Context::InputingConditionExpr;
        break;
    default:
        return "Object behavior keyword failure";
    }

    return {};
}

}