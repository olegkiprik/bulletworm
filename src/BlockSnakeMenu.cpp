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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "BlockSnake.hpp"
#include <bw_ext/FenwickTree.hpp>
#include "TextureLoader.hpp"
#include "Constants.hpp"
#include <bw_ext/Endianness.hpp>
#include "Word.hpp"
#include <bw_ext/const/Orientation.hpp>
#include "FilePaths.hpp"
#include "InterfaceEnums.hpp"
#include <bw_ext/stream/FileOutputStream.hpp>
#include <bw_ext/const/ExternalConstants.hpp>
#include "ObjectBehaviorLoader.hpp"
#include "LanguageLoader.hpp"
#include <bw_ext/LinguisticUtility.hpp>
#include <bw_ext/ObjParamEnumUtility.hpp>
#include <SFML/System/FileInputStream.hpp>
#include <SFML/System/MemoryInputStream.hpp>
#include <SFML/Graphics/Text.hpp>
#include <bw_ext/stream/MemoryOutputStream.hpp>
#include <SFML/Audio/Listener.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <bw_ext/sha256.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <optional>
#include <cassert>
#include <iostream>
#include <filesystem>
#include <cstring>

namespace Bulletworm {

LevelMenuCommand BlockSnake::selectLevel() {
    sf::Vector2f winSz(m_virtualWinSize);

    std::size_t avlc = m_levelStatistics.getAvailableLevelCount();

    unsigned int diffCount = m_levelStatistics.getDifficultyCount();
    unsigned int levelCount = m_levelStatistics.getLevelCount();

    unsigned int generalLevelCount = diffCount * levelCount;

    std::vector<sf::CircleShape> buttons(generalLevelCount);
    std::vector<sf::Text> descriptions(generalLevelCount);

    sf::Text chooseLevel;

    chooseLevel.setCharacterSize((unsigned int)(winSz.x * 45 / 1920));
    chooseLevel.setFont(getFont(FontType::Plain));
    chooseLevel.setPosition(winSz.x * 0.4f, winSz.y * 0.1f);
    chooseLevel.setString(getWord2fit(getWord(getSetting(SettingEnum::LanguageIndex),
                          Word::SelectTheLevel) + std::to_string(m_levelStatistics.getTotalScore()),
                          winSz.x / 4,
                          (unsigned int)(winSz.x * 45 / 1920),
                          getFont(FontType::Plain)));

    auto dstColFun = [this](ColorDst dst) {
        return getDestinationColor(dst);
    };

    for (unsigned int i = 0; i < levelCount; ++i) {
        for (unsigned int j = 0; j < diffCount; ++j) {
            const auto index = j + (std::size_t)i * diffCount;
            auto& desc = descriptions[index];
            auto& button = buttons[index];

            desc.setCharacterSize((unsigned int)(winSz.x * 38 / 1920));
            desc.setFont(getFont(FontType::Plain));
            desc.setPosition(winSz.x * 0.4f, winSz.y * 0.1f);

            const auto& levelDescr = getLevelDescr(getSetting(SettingEnum::LanguageIndex),
                                                   i, j);

            desc.setString(getWord2fit(levelDescr,
                           winSz.x / 4,
                           (unsigned int)(winSz.x * 38 / 1920),
                           getFont(FontType::Plain)));

            button.setRadius(winSz.x * 15 / 1920);
            button.setOutlineColor(dstColFun(ColorDst::LevelShapeOutline));
            button.setOutlineThickness(1.f);
            button.setPosition(winSz.x * (125 + 200 * j) / 1920,
                               winSz.y * (125 + 45 * i) / 1080);

            if (m_levelStatistics.isLevelCompleted(j, i))
                button.setFillColor(dstColFun(ColorDst::LevelShapeCompleted));

            else if (i < avlc && m_levelStatistics.levelExists(j, i))
                button.setFillColor(dstColFun(ColorDst::LevelShapeNCavailable));

            else {
                //button.setFillColor(dstColFun(ColorDst::LevelShapeNonAvailable));
                button.setOutlineColor(sf::Color::Transparent);
                button.setFillColor(sf::Color::Transparent);
            }
        }
    }

    unsigned currentDescrIndex = levelCount;
    unsigned currentDescrDiff = diffCount;

    sf::Vector2u oldSize = m_window.getSize();
    for (;;) {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Resized:
            {
                sf::Vector2u newSize(event.size.width, event.size.height);
                if (newSize.x == 0 && newSize.y == 0)
                    newSize = oldSize;
                else if (oldSize.x * newSize.y > newSize.x * oldSize.y) // new is looser
                    newSize.y = newSize.x * oldSize.y / oldSize.x;
                else
                    newSize.x = newSize.y * oldSize.x / oldSize.y;
                m_window.setSize(newSize);
                oldSize = newSize;
                break;
            }
            case sf::Event::Closed:
                return LevelMenuCommand::Exit;
            case sf::Event::MouseMoved: {
                bool hover = false;
                sf::Vector2i mouseMovePos{
                    event.mouseMove.x,
                    event.mouseMove.y
                };

                for (unsigned i = 0; i < levelCount; ++i)
                    for (unsigned j = 0; j < diffCount; ++j) {
                        auto& button = buttons[j + (std::size_t)i * diffCount];
                        auto bounds = button.getGlobalBounds();

                        if (bounds.contains(m_window.mapPixelToCoords(mouseMovePos)) && (i < avlc) &&
                            (m_levelStatistics.levelExists(j, i))) {
                            currentDescrIndex = i;
                            currentDescrDiff = j;
                            hover = true;
                            break;
                        }
                    }

                if (!hover)
                    currentDescrIndex = levelCount;
                break;
            }
            case sf::Event::MouseButtonPressed:
                for (unsigned i = 0; i < levelCount; ++i)
                    for (unsigned j = 0; j < diffCount; ++j) {
                        auto& button = buttons[j + (std::size_t)i * diffCount];
                        auto bounds = button.getGlobalBounds();
                        sf::Vector2i mouseMovePos{
                            event.mouseButton.x,
                            event.mouseButton.y
                        };
                        if (bounds.contains(m_window.mapPixelToCoords(mouseMovePos)) && (i < avlc &&
                            m_levelStatistics.levelExists(j, i))) {
                            m_levelIndex = i;
                            m_difficulty = j;
                            return LevelMenuCommand::Selected;
                        }
                    }


                break;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Escape || event.key.scancode == sf::Keyboard::Scancode::Q) {
                    return LevelMenuCommand::Back;
                } else if (event.key.code == sf::Keyboard::Enter || event.key.scancode == sf::Keyboard::Scancode::Space) {
                    if (currentDescrIndex < levelCount) {
                        m_levelIndex = currentDescrIndex;
                        m_difficulty = currentDescrDiff;
                        return LevelMenuCommand::Selected;
                    }
                } else {
                    if (event.key.code == sf::Keyboard::Up || event.key.scancode == sf::Keyboard::Scancode::W) {
                        if (currentDescrIndex >= levelCount) {
                            currentDescrIndex = 0;
                            currentDescrDiff = 0;
                        } else {
                            if (currentDescrIndex != 0 && 
                                m_levelStatistics.levelExists(currentDescrDiff, currentDescrIndex - 1))
                                --currentDescrIndex;
                        }
                    } else if (event.key.code == sf::Keyboard::Down || event.key.scancode == sf::Keyboard::Scancode::S) {
                        if (currentDescrIndex >= levelCount) {
                            currentDescrIndex = 0;
                            currentDescrDiff = 0;
                        } else {
                            if ((std::size_t)currentDescrIndex + 1 < avlc &&
                                m_levelStatistics.levelExists(currentDescrDiff, currentDescrIndex + 1))
                                ++currentDescrIndex;
                        }
                    } else if (event.key.code == sf::Keyboard::Left || event.key.scancode == sf::Keyboard::Scancode::A) {
                        if (currentDescrIndex >= levelCount) {
                            currentDescrIndex = 0;
                            currentDescrDiff = 0;
                        } else {
                            if (currentDescrDiff != 0 && m_levelStatistics
                                .levelExists(currentDescrDiff - 1, currentDescrIndex))
                                --currentDescrDiff;
                        }
                    } else if (event.key.code == sf::Keyboard::Right || event.key.scancode == sf::Keyboard::Scancode::D) {
                        if (currentDescrIndex >= levelCount) {
                            currentDescrIndex = 0;
                            currentDescrDiff = 0;
                        } else {
                            if (currentDescrDiff != diffCount - 1)
                                ++currentDescrDiff;
                        }
                    }
                }
                break;
            default:
                break;
            }
        }

        m_window.clear();
        m_window.draw(m_background);

        for (unsigned i = 0; i < generalLevelCount; ++i)
            m_window.draw(buttons[i]);

        if (currentDescrIndex < levelCount)
            m_window.draw(descriptions[currentDescrDiff +
                          (std::size_t)currentDescrIndex * diffCount]);
        else
            m_window.draw(chooseLevel);

        m_window.display();
    }
}


MainMenuCommand BlockSnake::mainMenu() {
    sf::Vector2f winSz(m_virtualWinSize);

    constexpr int TEXT_COUNT = 5;
    sf::Text texts[TEXT_COUNT];

    auto dstColFun = [this](ColorDst dst) {
        return getDestinationColor(dst);
    };

    for (int i = 0; i < TEXT_COUNT; ++i) {
        texts[i].setFont(getFont(FontType::Menu));
        texts[i].setCharacterSize(unsigned(winSz.x * 50 / 1920));
        texts[i].setPosition(winSz.x * 162 / 1920, winSz.y * (162 + 125 * i) / 1080);
        texts[i].setFillColor(dstColFun(ColorDst::MenuButtonPlain));
    }

    const auto lng = m_settings[(std::size_t)SettingEnum::LanguageIndex];

    texts[0].setString(getWord(lng, Word::PlayMainMenu));
    texts[1].setString(getWord(lng, Word::SettingsMainMenu));
    texts[2].setString(getWord(lng, Word::ManualMainMenu));
    texts[3].setString(getWord(lng, Word::LanguagesMainMenu));
    texts[4].setString(getWord(lng, Word::ExitFromMainMenu));

    int buttonPressed = TEXT_COUNT;
    
    sf::Vector2u oldSize = m_window.getSize();
    for (;;) {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Resized:
            {
                sf::Vector2u newSize(event.size.width, event.size.height);
                if (newSize.x == 0 && newSize.y == 0)
                    newSize = oldSize;
                else if (oldSize.x * newSize.y > newSize.x * oldSize.y) // new is looser
                    newSize.y = newSize.x * oldSize.y / oldSize.x;
                else
                    newSize.x = newSize.y * oldSize.x / oldSize.y;
                m_window.setSize(newSize);
                oldSize = newSize;
                break;
            }
            case sf::Event::Closed:
                return MainMenuCommand::Exit;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Enter || event.key.scancode == sf::Keyboard::Scancode::Space)
                    return MainMenuCommand::Play;
                else if (event.key.alt && event.key.scancode == sf::Keyboard::Scancode::Q) {
                    return MainMenuCommand::Exit;
                }
                break;
            case sf::Event::MouseMoved:
                if (buttonPressed == TEXT_COUNT) {
                    sf::Vector2i mouseMovePos{
                        event.mouseMove.x ,
                        event.mouseMove.y
                    };
                    for (int i = 0; i < TEXT_COUNT; ++i) {
                        if (texts[i].getGlobalBounds().contains(m_window.mapPixelToCoords(mouseMovePos))) {
                            texts[i].setFillColor(dstColFun(ColorDst::MenuButtonHover));
                        } else {
                            texts[i].setFillColor(dstColFun(ColorDst::MenuButtonPlain));
                        }
                    }
                } else {
                    auto& textButtonPr = texts[buttonPressed];
                    sf::Vector2i mouseMovePos{
                        event.mouseMove.x ,
                        event.mouseMove.y
                    };
                    if (textButtonPr.getGlobalBounds().contains(m_window.mapPixelToCoords(mouseMovePos))) {
                        textButtonPr.setFillColor(dstColFun(ColorDst::MenuButtonPressed));
                    } else {
                        textButtonPr.setFillColor(dstColFun(ColorDst::MenuButtonHover));
                    }
                }
                break;
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mouseMovePos{
                        (int)event.mouseButton.x ,
                        (int)event.mouseButton.y
                    };
                    for (int i = 0; i < TEXT_COUNT; ++i) {
                        if (texts[i].getGlobalBounds().contains(m_window.mapPixelToCoords(mouseMovePos))) {
                            texts[i].setFillColor(dstColFun(ColorDst::MenuButtonPressed));
                            buttonPressed = i;
                            break;
                        }
                    }
                }
                break;
            case sf::Event::MouseButtonReleased:
                if (buttonPressed != TEXT_COUNT &&
                    event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mouseMovePos{
                        (int)event.mouseButton.x ,
                        (int)event.mouseButton.y
                    };
                    if (texts[buttonPressed].getGlobalBounds().contains(m_window.mapPixelToCoords(mouseMovePos))) {
                        return static_cast<MainMenuCommand>(buttonPressed);
                    } else {
                        texts[buttonPressed].setFillColor(dstColFun(ColorDst::MenuButtonPlain));
                        buttonPressed = TEXT_COUNT;
                    }
                }
                break;
            default:
                break;
            }
        }

        m_window.clear();
        m_window.draw(m_background);
        for (int i = 0; i < TEXT_COUNT; ++i)
            m_window.draw(texts[i]);

        m_window.display();
    }
}


PauseMenuCommand BlockSnake::pauseMenu() {
    sf::Vector2f winSz(m_virtualWinSize);

    constexpr int TEXT_COUNT = 5;
    sf::Text texts[TEXT_COUNT];

    using Cdst = ColorDst;

    auto dstColFun = [this](Cdst dst) {
        return getDestinationColor(dst);
    };

    for (int i = 0; i < TEXT_COUNT; ++i) {
        texts[i].setFont(getFont(FontType::Menu));
        texts[i].setCharacterSize(unsigned(winSz.x * 50 / 1920));
        texts[i].setPosition(winSz.x * 125 / 1920, winSz.y * (125 + 125 * i) / 1080);
        texts[i].setFillColor(dstColFun(Cdst::MenuButtonPlain));
    }

    const auto lng = m_settings[(std::size_t)SettingEnum::LanguageIndex];

    texts[0].setString(getWord(lng, Word::ContinuePauseMenu));
    texts[1].setString(getWord(lng, Word::SettingsPauseMenu));
    texts[2].setString(getWord(lng, Word::ManualPauseMenu));
    texts[3].setString(getWord(lng, Word::OpenMainMenuPauseMenu));
    texts[4].setString(getWord(lng, Word::ExitFromBlockSnakePauseMenu));

    int buttonPressed = TEXT_COUNT;
    sf::Vector2u oldSize = m_window.getSize();
    for (;;) {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Resized:
            {
                sf::Vector2u newSize(event.size.width, event.size.height);
                if (newSize.x == 0 && newSize.y == 0)
                    newSize = oldSize;
                else if (oldSize.x * newSize.y > newSize.x * oldSize.y) // new is looser
                    newSize.y = newSize.x * oldSize.y / oldSize.x;
                else
                    newSize.x = newSize.y * oldSize.x / oldSize.y;
                m_window.setSize(newSize);
                oldSize = newSize;
                break;
            }
            case sf::Event::Closed:
                return PauseMenuCommand::Exit;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::Enter ||
                    event.key.scancode == sf::Keyboard::Scancode::W || event.key.scancode == sf::Keyboard::Scancode::A ||
                    event.key.scancode == sf::Keyboard::Scancode::S || event.key.scancode == sf::Keyboard::Scancode::D)
                    return PauseMenuCommand::Continue;
                break;
            case sf::Event::MouseMoved:
                if (buttonPressed == TEXT_COUNT) {
                    for (int i = 0; i < TEXT_COUNT; ++i) {
                        if (texts[i].getGlobalBounds().contains(m_window
                            .mapPixelToCoords(sf::Vector2i(event.mouseMove.x,
                            event.mouseMove.y)))) {
                            texts[i].setFillColor(dstColFun(Cdst::MenuButtonHover));
                        } else {
                            texts[i].setFillColor(dstColFun(Cdst::MenuButtonPlain));
                        }
                    }
                } else {
                    if (texts[buttonPressed].getGlobalBounds().contains(m_window
                        .mapPixelToCoords(sf::Vector2i(event.mouseMove.x,
                        event.mouseMove.y)))) {
                        texts[buttonPressed].setFillColor(dstColFun(Cdst::MenuButtonPressed));
                    } else {
                        texts[buttonPressed].setFillColor(dstColFun(Cdst::MenuButtonHover));
                    }
                }
                break;
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left) {
                    for (int i = 0; i < TEXT_COUNT; ++i) {
                        if (texts[i].getGlobalBounds().contains
                        (m_window.mapPixelToCoords
                        (sf::Vector2i(event.mouseButton.x,event.mouseButton.y)))) {
                            texts[i].setFillColor(dstColFun(Cdst::MenuButtonPressed));
                            buttonPressed = i;
                            break;
                        }
                    }
                }
                break;
            case sf::Event::MouseButtonReleased:
                if (buttonPressed != TEXT_COUNT &&
                    event.mouseButton.button == sf::Mouse::Left) {
                    if (texts[buttonPressed].getGlobalBounds().contains(m_window
                        .mapPixelToCoords(sf::Vector2i(event.mouseButton.x,
                        event.mouseButton.y)))) {
                        return static_cast<PauseMenuCommand>(buttonPressed);
                    } else {
                        texts[buttonPressed].setFillColor(dstColFun(Cdst::MenuButtonPlain));
                        buttonPressed = TEXT_COUNT;
                    }
                }
                break;
            default:
                break;
            }
        }

        m_window.clear();
        m_window.draw(m_background);
        for (int i = 0; i < TEXT_COUNT; ++i)
            m_window.draw(texts[i]);
        m_window.display();
    }
}


bool BlockSnake::settings() {
    sf::Vector2f winSz(m_virtualWinSize);

    using Cdst = ColorDst;

    auto dcf = [this](Cdst dst) {
        return getDestinationColor(dst);
    };

    sf::RectangleShape musicVolume(sf::Vector2f(winSz.x * 600 / 1920, 50));
    sf::RectangleShape soundVolume(sf::Vector2f(winSz.x * 600 / 1920, 50));
    sf::RectangleShape ambientVolume(sf::Vector2f(winSz.x * 600 / 1920, 50));

    musicVolume.setFillColor(dcf(Cdst::VolumeFill));
    soundVolume.setFillColor(dcf(Cdst::VolumeFill));
    ambientVolume.setFillColor(dcf(Cdst::VolumeFill));

    musicVolume.setOutlineColor(dcf(Cdst::VolumeOutline));
    soundVolume.setOutlineColor(dcf(Cdst::VolumeOutline));
    ambientVolume.setOutlineColor(dcf(Cdst::VolumeOutline));

    musicVolume.setOutlineThickness(1.f);
    soundVolume.setOutlineThickness(1.f);
    ambientVolume.setOutlineThickness(1.f);

    musicVolume.setPosition(winSz.x * 125 / 1920, winSz.y * 125 / 1080);
    soundVolume.setPosition(winSz.x * 125 / 1920, winSz.y * 250 / 1080);
    ambientVolume.setPosition(winSz.x * 125 / 1920, winSz.y * 375 / 1080);

    sf::RectangleShape fullscreenButton(sf::Vector2f(winSz.x * 50 / 1920, winSz.x * 50 / 1920));

    fullscreenButton.setFillColor(m_settings[(std::size_t)SettingEnum::FullscreenEnabled] ?
                                  dcf(Cdst::ButtonEnabled) :
                                  dcf(Cdst::ButtonDisabled));

    fullscreenButton.setOutlineColor(dcf(Cdst::BooleanButtonOutline));
    fullscreenButton.setOutlineThickness(1.f);
    fullscreenButton.setPosition(winSz.x * 125 / 1920, winSz.y * 500 / 1080);

    sf::RectangleShape musicVolumePtr(sf::Vector2f(10, 60));
    sf::RectangleShape soundVolumePtr(sf::Vector2f(10, 60));
    sf::RectangleShape ambientVolumePtr(sf::Vector2f(10, 60));

    musicVolumePtr.setOrigin(5, 5);
    soundVolumePtr.setOrigin(5, 5);
    ambientVolumePtr.setOrigin(5, 5);

    musicVolumePtr.setOutlineThickness(1.f);
    soundVolumePtr.setOutlineThickness(1.f);
    ambientVolumePtr.setOutlineThickness(1.f);

    musicVolumePtr.setOutlineColor(dcf(Cdst::FloatingPointerOutline));
    soundVolumePtr.setOutlineColor(dcf(Cdst::FloatingPointerOutline));
    ambientVolumePtr.setOutlineColor(dcf(Cdst::FloatingPointerOutline));

    musicVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFill));
    soundVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFill));
    ambientVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFill));

    musicVolumePtr.setPosition(winSz.x * (125 + (float)getSetting(SettingEnum::MusicVolumePer10000) /
                               10000 * 600) / 1920, winSz.y * 125 / 1080);

    soundVolumePtr.setPosition(winSz.x * (125 + (float)getSetting(SettingEnum::SoundVolumePer10000) /
                               10000 * 600) / 1920, winSz.y * 250 / 1080);

    ambientVolumePtr.setPosition(winSz.x * (125 + (float)getSetting(SettingEnum::AmbientVolumePer10000) /
                               10000 * 600) / 1920, winSz.y * 375 / 1080);

    const auto lng = getSetting(SettingEnum::LanguageIndex);

    sf::Text musicVolumeSign(getWord(lng, Word::MusicVolume),
                             getFont(FontType::Plain),
                             unsigned(winSz.x * 25 / 1920));

    sf::Text soundVolumeSign(getWord(lng, Word::SoundVolume),
                             getFont(FontType::Plain),
                             unsigned(winSz.x * 25 / 1920));

    sf::Text ambientVolumeSign(getWord(lng, Word::AmbientVolume),
                             getFont(FontType::Plain),
                             unsigned(winSz.x * 25 / 1920));

    sf::Text fullscreenButtonSign(getWord(lng, Word::Fullscreen),
                                  getFont(FontType::Plain),
                                  unsigned(winSz.x * 25 / 1920));

    musicVolumeSign.setPosition(winSz.x * 125 / 1920, winSz.y * (125 - 37) / 1080);
    soundVolumeSign.setPosition(winSz.x * 125 / 1920, winSz.y * (250 - 37) / 1080);
    ambientVolumeSign.setPosition(winSz.x * 125 / 1920, winSz.y * (375 - 37) / 1080);
    
    fullscreenButtonSign.setPosition(winSz.x * 125 / 1920, winSz.y * (500 - 37) / 1080);

    musicVolumeSign.setFillColor(dcf(Cdst::SettingSignFill));
    soundVolumeSign.setFillColor(dcf(Cdst::SettingSignFill));
    ambientVolumeSign.setFillColor(dcf(Cdst::SettingSignFill));

    fullscreenButtonSign.setFillColor(dcf(Cdst::SettingSignFill));

    sf::Text ok(getWord(lng, Word::OkSettings), getFont(FontType::Menu),
                unsigned(winSz.x * 50 / 1920));

    ok.setPosition(winSz.x * 125 / 1920, winSz.y * 850 / 1080);
    ok.setFillColor(dcf(Cdst::SettingOkFill));

    constexpr int SETTINGS_ELEMENT_COUNT = 5;
    int whatPressed = SETTINGS_ELEMENT_COUNT;
    sf::Vector2u oldSize = m_window.getSize();
    for (;;) {

        sf::Event event;
        while (m_window.pollEvent(event)) {

            switch (event.type) {
            case sf::Event::Resized:
            {
                sf::Vector2u newSize(event.size.width, event.size.height);
                if (newSize.x == 0 && newSize.y == 0)
                    newSize = oldSize;
                else if (oldSize.x * newSize.y > newSize.x * oldSize.y) // new is looser
                    newSize.y = newSize.x * oldSize.y / oldSize.x;
                else
                    newSize.x = newSize.y * oldSize.x / oldSize.y;
                m_window.setSize(newSize);
                oldSize = newSize;
                break;
            }

            case sf::Event::Closed:
                return false;

            case sf::Event::MouseMoved:
            {
                sf::Vector2i mouseMovePos{ (int)event.mouseMove.x,
                    (int)event.mouseMove.y };

                sf::Vector2f mouseMovePosCoords{ m_window.mapPixelToCoords(mouseMovePos) };

                if (whatPressed == SETTINGS_ELEMENT_COUNT) {
                    musicVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFill));
                    soundVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFill));
                    ambientVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFill));

                    if (m_settings[(std::size_t)SettingEnum::FullscreenEnabled]) {
                        fullscreenButton.setFillColor(dcf(Cdst::ButtonEnabled));
                    } else {
                        fullscreenButton.setFillColor(dcf(Cdst::ButtonDisabled));
                    }

                    ok.setFillColor(dcf(Cdst::SettingOkFill));

                    if (musicVolume.getGlobalBounds().contains(mouseMovePosCoords)) {
                        musicVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFillHover));

                    } else if (soundVolume.getGlobalBounds().contains(mouseMovePosCoords)) {
                        soundVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFillHover));

                    } else if (ambientVolume.getGlobalBounds().contains(mouseMovePosCoords)) {
                        ambientVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFillHover));

                    } else if (fullscreenButton.getGlobalBounds().contains(mouseMovePosCoords)) {

                        if (m_settings[(std::size_t)SettingEnum::FullscreenEnabled]) {
                            fullscreenButton.setFillColor(dcf(Cdst::ButtonEnabledHover));

                        } else {
                            fullscreenButton.setFillColor(dcf(Cdst::ButtonDisabledHover));
                        }
                    } else if (ok.getGlobalBounds().contains(mouseMovePosCoords)) {
                        ok.setFillColor(dcf(Cdst::SettingOkFillHover));
                    }
                } else {
                    switch (whatPressed) {
                    case 0: {
                        sf::Vector2f localCoords = musicVolume.getInverseTransform()
                            .transformPoint(mouseMovePosCoords);

                        float newValue = localCoords.x / musicVolume.getSize().x;

                        newValue = std::min(std::max(newValue, 0.f), 1.f);

                        m_settings[(std::size_t)SettingEnum::MusicVolumePer10000] =
                            (std::uint32_t)(newValue * 10000);

                        m_music.setVolume(newValue * 100);

                        musicVolumePtr.setPosition(winSz.x * 125 / 1920 +
                                                   newValue * musicVolume.getSize().x,
                                                   winSz.y * 125 / 1080);
                        break;
                    }
                    case 1: {
                        sf::Vector2f localCoords = soundVolume.getInverseTransform()
                            .transformPoint(mouseMovePosCoords);

                        float newValue = localCoords.x / soundVolume.getSize().x;

                        newValue = std::min(std::max(newValue, 0.f), 1.f);

                        m_settings[(std::size_t)SettingEnum::SoundVolumePer10000] =
                            (std::uint32_t)(newValue * 10000);

                        soundVolumePtr.setPosition(winSz.x * 125 / 1920 +
                                                   newValue * soundVolume.getSize().x,
                                                   winSz.y * 250 / 1080);
                        break;
                    }
                    case 2: {
                        sf::Vector2f localCoords = ambientVolume.getInverseTransform()
                            .transformPoint(mouseMovePosCoords);

                        float newValue = localCoords.x / ambientVolume.getSize().x;

                        newValue = std::min(std::max(newValue, 0.f), 1.f);

                        m_settings[(std::size_t)SettingEnum::AmbientVolumePer10000] =
                            (std::uint32_t)(newValue * 10000);

                        m_ambient.setVolume(newValue * 100);

                        ambientVolumePtr.setPosition(winSz.x * 125 / 1920 +
                                                   newValue * ambientVolume.getSize().x,
                                                   winSz.y * 375 / 1080);
                        break;
                    }
                    case 3:
                        if (fullscreenButton.getGlobalBounds().contains(mouseMovePosCoords)) {
                            if (m_settings[(std::size_t)SettingEnum::FullscreenEnabled]) {
                                fullscreenButton.setFillColor(dcf(Cdst::BooleanButtonPressed));
                            } else {
                                fullscreenButton.setFillColor(dcf(Cdst::BooleanButtonPressed));
                            }
                        } else {
                            if (m_settings[(std::size_t)SettingEnum::FullscreenEnabled]) {
                                fullscreenButton.setFillColor(dcf(Cdst::ButtonEnabledHover));
                            } else {
                                fullscreenButton.setFillColor(dcf(Cdst::ButtonDisabledHover));
                            }
                        }
                        break;
                    case 4:
                        if (ok.getGlobalBounds().contains(mouseMovePosCoords)) {
                            ok.setFillColor(dcf(Cdst::SettingOkPressed));
                        } else {
                            ok.setFillColor(dcf(Cdst::SettingOkFillHover));
                        }
                        break;
                    default:
                        break;
                    }
                }
                break;
            }
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mappedCoords{ m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x,
                        event.mouseButton.y)) };

                    if (musicVolume.getGlobalBounds().contains(mappedCoords)) {

                        sf::Vector2f localCoords =
                            musicVolume.getInverseTransform().transformPoint(mappedCoords);

                        float newValue = localCoords.x / musicVolume.getSize().x;
                        newValue = std::min(std::max(newValue, 0.f), 1.f);

                        m_settings[(std::size_t)SettingEnum::MusicVolumePer10000] =
                            (std::uint32_t)(newValue * 10000);

                        m_music.setVolume(newValue * 100);

                        musicVolumePtr.setPosition(winSz.x * 125 / 1920 +
                                                   newValue * musicVolume.getSize().x,
                                                   winSz.y * 125 / 1080);
                        whatPressed = 0;

                    } else if (soundVolume.getGlobalBounds().contains(mappedCoords)) {

                        sf::Vector2f localCoords = soundVolume.getInverseTransform()
                            .transformPoint(m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x,
                                            event.mouseButton.y)));

                        float newValue = localCoords.x / soundVolume.getSize().x;

                        newValue = std::min(std::max(newValue, 0.f), 1.f);

                        m_settings[(std::size_t)SettingEnum::SoundVolumePer10000] =
                            (std::uint32_t)(newValue * 10000);

                        soundVolumePtr.setPosition(winSz.x * 125 / 1920 +
                                                   newValue * soundVolume.getSize().x,
                                                   winSz.y * 250 / 1080);
                        whatPressed = 1;
                    } else if (ambientVolume.getGlobalBounds().contains(mappedCoords)) {

                        sf::Vector2f localCoords = ambientVolume.getInverseTransform()
                            .transformPoint(mappedCoords);

                        float newValue = localCoords.x / ambientVolume.getSize().x;

                        newValue = std::min(std::max(newValue, 0.f), 1.f);

                        m_settings[(std::size_t)SettingEnum::AmbientVolumePer10000] =
                            (std::uint32_t)(newValue * 10000);

                        m_ambient.setVolume(newValue * 100);

                        ambientVolumePtr.setPosition(winSz.x * 125 / 1920 +
                                                   newValue * ambientVolume.getSize().x,
                                                   winSz.y * 375 / 1080);
                        whatPressed = 2;
                    } else if (fullscreenButton.getGlobalBounds().contains(mappedCoords)) {
                        if (m_settings[(std::size_t)SettingEnum::FullscreenEnabled]) {
                            fullscreenButton.setFillColor(dcf(Cdst::BooleanButtonPressed));
                        } else {
                            fullscreenButton.setFillColor(dcf(Cdst::BooleanButtonPressed));
                        }
                        whatPressed = 3;
                    } else if (ok.getGlobalBounds().contains(mappedCoords)) {
                        ok.setFillColor(dcf(Cdst::SettingOkPressed));
                        whatPressed = 4;
                    } else {
                        whatPressed = SETTINGS_ELEMENT_COUNT;
                    }
                }
                break;
            case sf::Event::MouseButtonReleased:
                if (whatPressed != SETTINGS_ELEMENT_COUNT &&
                    event.mouseButton.button == sf::Mouse::Left) {

                    sf::Vector2f mappedCoords{ m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x,
                        event.mouseButton.y)) };

                    switch (whatPressed) {
                    case 0: {
                        musicVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFill));
                        break;
                    }
                    case 1: {
                      // sound
                        SoundThrower::Parameters param;
                        param.relativeToListener = true;
                        param.volume = (float)m_settings[(std::size_t)SettingEnum::SoundVolumePer10000] / 100;
                        m_soundPlayer.playSound(SoundType::ItemEat, param);

                        soundVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFill));
                        break;
                    }
                    case 2: {
                        ambientVolumePtr.setFillColor(dcf(Cdst::FloatingPointerFill));
                        break;
                    }
                    case 3:
                        if (fullscreenButton.getGlobalBounds().contains(
                            mappedCoords)) {
                            if (m_settings[(std::size_t)SettingEnum::FullscreenEnabled]) {
                                m_settings[(std::size_t)SettingEnum::FullscreenEnabled] = false;
                                createWindow();
                                oldSize = m_window.getSize();
                                fullscreenButton.setFillColor(dcf(Cdst::ButtonDisabledHover));
                            } else {
                                m_settings[(std::size_t)SettingEnum::FullscreenEnabled] = true;
                                createWindow();
                                oldSize = m_window.getSize();
                                fullscreenButton.setFillColor(dcf(Cdst::ButtonEnabledHover));
                            }
                        } else {
                            if (m_settings[(std::size_t)SettingEnum::FullscreenEnabled]) {
                                fullscreenButton.setFillColor(dcf(Cdst::ButtonEnabled));
                            } else {
                                fullscreenButton.setFillColor(dcf(Cdst::ButtonDisabled));
                            }
                        }
                        break;
                    case 4:
                        if (ok.getGlobalBounds().contains(mappedCoords)) {
                            return true;
                        } else {
                            ok.setFillColor(dcf(Cdst::SettingOkFill));
                        }
                        break;
                    default:
                        break;
                    }
                    whatPressed = SETTINGS_ELEMENT_COUNT;
                }
                break;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Escape || event.key.scancode == sf::Keyboard::Scancode::Q)
                    return true;
                break;
            default:
                break;
            }
        }

        m_window.clear();
        m_window.draw(m_background);
        m_window.draw(ok);
        m_window.draw(fullscreenButton);
        m_window.draw(musicVolume);
        m_window.draw(soundVolume);
        m_window.draw(ambientVolume);
        m_window.draw(musicVolumeSign);
        m_window.draw(soundVolumeSign);
        m_window.draw(ambientVolumeSign);
        m_window.draw(fullscreenButtonSign);
        m_window.draw(musicVolumePtr);
        m_window.draw(soundVolumePtr);
        m_window.draw(ambientVolumePtr);
        m_window.display();
    }
}


bool BlockSnake::manual() {
    sf::Vector2f winSz(m_virtualWinSize);

    const auto lng = m_settings[(std::size_t)SettingEnum::LanguageIndex];

    sf::Text text(getWord2fit(getWord(lng, Word::ManualText), winSz.x,
                  static_cast<unsigned>(winSz.x * 40 / 1920),
                  m_fonts[static_cast<std::size_t>(FontType::Manual)]),
                  m_fonts[static_cast<std::size_t>(FontType::Manual)],
                  static_cast<unsigned>(winSz.x * 40 / 1920));
    sf::Text ok(getWord(lng, Word::OkManual), m_fonts[static_cast<std::size_t>(FontType::Menu)],
                static_cast<unsigned>(winSz.x * 50 / 1920));

    text.setPosition(winSz.x * 10 / 1920, winSz.y * 10 / 1080);
    ok.setOrigin(ok.getGlobalBounds().width * 0.5f, ok.getGlobalBounds().height * 0.5f);
    ok.setPosition(winSz.x * 0.5f, winSz.y * 0.8f);
    sf::Vector2u oldSize = m_window.getSize();
    for (;;) {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Resized:
            {
                sf::Vector2u newSize(event.size.width, event.size.height);
                if (newSize.x == 0 && newSize.y == 0)
                    newSize = oldSize;
                else if (oldSize.x * newSize.y > newSize.x * oldSize.y) // new is looser
                    newSize.y = newSize.x * oldSize.y / oldSize.x;
                else
                    newSize.x = newSize.y * oldSize.x / oldSize.y;
                m_window.setSize(newSize);
                oldSize = newSize;
                break;
            }
            case sf::Event::Closed:
                return false;
            case sf::Event::MouseButtonPressed:
                if (ok.getGlobalBounds().contains(m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x,
                    event.mouseButton.y)))) {
                    return true;
                }
                break;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Escape || event.key.scancode == sf::Keyboard::Scancode::Q)
                    return true;
                else if (event.key.code == sf::Keyboard::C && event.key.control) {
                    sf::Clipboard::setString(getWord(lng, Word::ManualText));
                }
                break;
            default:
                break;
            }
        }

        m_window.clear();
        m_window.draw(m_background);
        m_window.draw(text);
        m_window.draw(ok);
        m_window.display();
    }
}


bool BlockSnake::languages() {
    auto& lng = m_settings[(std::size_t)SettingEnum::LanguageIndex];
    const std::size_t lngCount = m_languageTitles.size();

    sf::Vector2f winSz(m_virtualWinSize);
    std::vector<sf::Text> langNames(lngCount);

    for (std::size_t i = 0; i < lngCount; ++i) {
        langNames[i].setFont(m_fonts[static_cast<std::size_t>(FontType::Plain)]);
        langNames[i].setCharacterSize(static_cast<unsigned>(winSz.x * 50 / 1920));
        langNames[i].setString(getWord(i, Word::LanguageName));
        langNames[i].setOrigin(langNames[i].getGlobalBounds().width / 2.f,
                               langNames[i].getGlobalBounds().height / 2.f);
        langNames[i].setPosition(winSz.x * 0.5f, winSz.y * (0.1f + 0.08f * i));
    }
    sf::Vector2u oldSize = m_window.getSize();
    for (;;) {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Resized:
            {
                sf::Vector2u newSize(event.size.width, event.size.height);
                if (newSize.x == 0 && newSize.y == 0)
                    newSize = oldSize;
                else if (oldSize.x * newSize.y > newSize.x * oldSize.y) // new is looser
                    newSize.y = newSize.x * oldSize.y / oldSize.x;
                else
                    newSize.x = newSize.y * oldSize.x / oldSize.y;
                m_window.setSize(newSize);
                oldSize = newSize;
                break;
            }
            case sf::Event::Closed:
                return false;
            case sf::Event::MouseButtonPressed:
                for (unsigned i = 0; i < lngCount; ++i) {
                    if (langNames[i].getGlobalBounds().contains(m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x,
                        event.mouseButton.y)))) {
                        lng = i;
                        return true;
                    }
                }
                break;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Escape || event.key.scancode == sf::Keyboard::Scancode::Q)
                    return true;
                break;
            default:
                break;
            }
        }

        m_window.clear();
        m_window.draw(m_background);
        for (const auto& now : langNames)
            m_window.draw(now);
        m_window.display();
    }
}


StatisticMenu BlockSnake::statisticMenu(bool completed) {
    sf::Vector2f winSz(m_virtualWinSize);

    sf::Int64 timeConverted1[TimeUnitCount];
    sf::Int64 gameTime1 = m_currGameTimeElapsed;
    convertTime(gameTime1, timeConverted1);

    sf::Text countableText;
    countableText.setFont(m_fonts[static_cast<std::size_t>(FontType::LevelStatistics)]);
    countableText.setCharacterSize(static_cast<unsigned>(winSz.x * 30 / 1920));
    countableText.setPosition(winSz.x * 175 / 1920, winSz.y * 125 / 1080);
    countableText.setFillColor(getDestinationColor(ColorDst::LevelStats));

    int fruitCount = m_currFruitEatenCount;
    int bonusCount = m_currBonusEatenCount;
    int powerupCount = m_currPowerupEatenCount;
    int stepCount = m_currStepCount;

    const auto lng = m_settings[(std::size_t)SettingEnum::LanguageIndex];

    auto plotData = m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);

    sf::String countableStr{
        getWord(lng, Word::LevelStatsLS) + ":\n" +
        getWord(lng, Word::EatenLS) + ":\n" + std::to_string(fruitCount) + L' ' +
        getWord(lng, Word(int(Word::FruitsSingleLS) +
                int(linguisticCountType(fruitCount)))) +
        '\n' + std::to_string(bonusCount) + L' ' +
        getWord(lng, Word(int(Word::BonusesSingleLS) +
                int(linguisticCountType(bonusCount)))) + '\n' +
        std::to_string(powerupCount) + L' ' +
        getWord(lng, Word(int(Word::PowerupsSingleLS) +
                int(linguisticCountType(powerupCount)))) +
        '\n' +
        getWord(lng, Word::GoneLS) + ' ' + std::to_string(stepCount) +
        L' ' +
        getWord(lng, Word(int(Word::StepsSingleLS) +
                int(linguisticCountType(stepCount)))) +
        '\n' +
        getWord(lng, Word::ScoreLS) + ": " +
        std::to_string(std::min((std::uintmax_t)UINT32_MAX,
        (std::uintmax_t)plotData[(int)LevelPlotDataEnum::FruitScoreCoeff] *
                       m_currFruitEatenCount +
        (std::uintmax_t)plotData[(int)LevelPlotDataEnum::BonusScoreCoeff] *
                       m_currBonusEatenCount +
        (std::uintmax_t)plotData[(int)LevelPlotDataEnum::SuperbonusScoreCoeff] *
                       m_currPowerupEatenCount)) + '\n' +
        getWord(lng, Word::GameCountLS) + ": " +
        std::to_string(m_levelStatistics.getLevelGameCount(m_difficulty, m_levelIndex)) + '\n' +
        getWord(lng, Word::GameTimeLS) + ':' };

    if (timeConverted1[0]) {
        countableStr += ' ' + std::to_string(timeConverted1[0]) + ' ' +
            getWord(lng, Word(int(Word::WeeksSingleLS) + int(linguisticCountType(timeConverted1[0]))));
    }
    if (timeConverted1[1]) {
        countableStr += ' ' + std::to_string(timeConverted1[1]) + ' ' +
            getWord(lng, Word(int(Word::DaysSingleLS) + int(linguisticCountType(timeConverted1[1]))));
    }
    if (timeConverted1[2]) {
        countableStr += ' ' + std::to_string(timeConverted1[2]) + ' ' +
            getWord(lng, Word(int(Word::HoursSingleLS) + int(linguisticCountType(timeConverted1[2]))));
    }
    if (timeConverted1[3]) {
        countableStr += ' ' + std::to_string(timeConverted1[3]) + ' ' +
            getWord(lng, Word(int(Word::MinutesSingleLS) + int(linguisticCountType(timeConverted1[3]))));
    }

    if (timeConverted1[4] ||
        (
        !timeConverted1[0] &&
        !timeConverted1[1] &&
        !timeConverted1[2] &&
        !timeConverted1[3]
    )) {
        countableStr += ' ' + std::to_string(timeConverted1[4]) + ' ' +
            getWord(lng, Word(int(Word::SecondsSingleLS) +
                    int(linguisticCountType(timeConverted1[4]))));
    }

    countableStr += "\n\n" +
        getWord(lng, Word::TotalGameStatsLS) + ":\n" +
        getWord(lng, Word::ScoreSumLS) + ": " +
        std::to_string(m_levelStatistics.getTotalScore()) + '\n' +
        getWord(lng, Word::GameCountLS) + ": " +
        std::to_string(m_levelStatistics.getTotalGameCount()) + '\n' +
        getWord(lng, Word::GameTimeLS) + ':';

    convertTime((sf::Int64)m_levelStatistics.getWholeGameTime(), timeConverted1);

    if (timeConverted1[0]) {
        countableStr += ' ' + std::to_string(timeConverted1[0]) + ' ' +
            getWord(lng, Word(int(Word::WeeksSingleLS) +
                    int(linguisticCountType(timeConverted1[0]))));
    }
    if (timeConverted1[1]) {
        countableStr += ' ' + std::to_string(timeConverted1[1]) + ' ' +
            getWord(lng, Word(int(Word::DaysSingleLS) +
                    int(linguisticCountType(timeConverted1[1]))));
    }
    if (timeConverted1[2]) {
        countableStr += ' ' + std::to_string(timeConverted1[2]) + ' ' +
            getWord(lng, Word(int(Word::HoursSingleLS) +
                    int(linguisticCountType(timeConverted1[2]))));
    }
    if (timeConverted1[3]) {
        countableStr += ' ' + std::to_string(timeConverted1[3]) + ' ' +
            getWord(lng, Word(int(Word::MinutesSingleLS) +
                    int(linguisticCountType(timeConverted1[3]))));
    }
    if (timeConverted1[4] ||
        (
        !timeConverted1[0] &&
        !timeConverted1[1] &&
        !timeConverted1[2] &&
        !timeConverted1[3]
    )) {
        countableStr += ' ' + std::to_string(timeConverted1[4]) + ' ' +
            getWord(lng, Word(int(Word::SecondsSingleLS) +
                    int(linguisticCountType(timeConverted1[4]))));
    }

    if (completed)
        countableStr += "\n\n" + getWord(lng, Word::LevelCompletedLS);

    countableText.setString(countableStr);

    sf::Text buttons[3];

    for (int i = 0; i < 3; ++i) {
        buttons[i].setFont(m_fonts[static_cast<std::size_t>(FontType::Menu)]);
        buttons[i].setCharacterSize(static_cast<unsigned>(winSz.x * 40 / 1920));
        buttons[i].setPosition(winSz.x * (3 / 16.f + 3 / 8.f * static_cast<float>(i / 2)),
                               winSz.y * (3 / 4.f + 1 / 10.f * static_cast<float>(i % 2)));
    }

    buttons[0].setString(getWord(lng, Word::ExitFromBlockSnakeLS));
    buttons[1].setString(getWord(lng, Word::RestartTheLevelLS));
    buttons[2].setString(getWord(lng, Word::OpenMainMenuLS));

    sf::Vector2u oldSize = m_window.getSize();
    for (;;) {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Resized:
            {
                sf::Vector2u newSize(event.size.width, event.size.height);
                if (newSize.x == 0 && newSize.y == 0)
                    newSize = oldSize;
                else if (oldSize.x * newSize.y > newSize.x * oldSize.y) // new is looser
                    newSize.y = newSize.x * oldSize.y / oldSize.x;
                else
                    newSize.x = newSize.y * oldSize.x / oldSize.y;
                m_window.setSize(newSize);
                oldSize = newSize;
                break;
            }
            case sf::Event::Closed:
                return StatisticMenu::Exit;
            case sf::Event::MouseButtonPressed:

                for (int i = 0; i < 3; ++i)
                    if (buttons[i].getGlobalBounds().contains(m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x,
                        event.mouseButton.y))))
                        return StatisticMenu(i);

                break;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Escape || event.key.scancode == sf::Keyboard::Scancode::Q)
                    return StatisticMenu::ToLevelMenu;
                else if (event.key.scancode == sf::Keyboard::Scancode::Space)
                    return StatisticMenu::Again;
                break;
            default:
                break;
            }
        }

        m_window.clear();
        m_window.draw(m_background);
        m_window.draw(countableText);

        for (int i = 0; i < 3; ++i)
            m_window.draw(buttons[i]);

        m_window.display();
    }
}

}