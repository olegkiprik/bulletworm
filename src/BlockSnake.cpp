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
#include "FenwickTree.hpp"
#include "TextureLoader.hpp"
#include "Constants.hpp"
#include "Endianness.hpp"
#include "Word.hpp"
#include "Orientation.hpp"
#include "FilePaths.hpp"
#include "InterfaceEnums.hpp"
#include "FileOutputStream.hpp"
#include "ExternalConstants.hpp"
#include "ObjectBehaviorLoader.hpp"
#include "LanguageLoader.hpp"
#include "LinguisticUtility.hpp"
#include "ObjParamEnumUtility.hpp"
#include <SFML/System/FileInputStream.hpp>
#include <SFML/System/MemoryInputStream.hpp>
#include <SFML/Graphics/Text.hpp>
#include "MemoryOutputStream.hpp"
#include <SFML/Audio/Listener.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Clipboard.hpp>
#include "sha256.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <optional>
#include <cassert>
#include <iostream>
#include <filesystem>
#include <cstring>

namespace {

void reportDebug(const char* str) {
#ifndef NDEBUG
    std::cout << str << std::endl;
#endif // NDEBUG
}

// Word Wrap (shall be fixed!)
sf::String getWord2fit(sf::String src,
                       float fitWidth,
                       unsigned int charSize,
                       const sf::Font& font) {
    // text to test
    sf::Text text;

    // current line position
    std::size_t prevPos = 0;

    // current word position
    std::size_t prevWordPos = sf::String::InvalidPos;

    text.setFont(font);
    text.setCharacterSize(charSize);

    for (std::size_t i = 0; i <= src.getSize(); ++i) {
        // content
        if (i < src.getSize() && src[i] != ' ')
            continue;

        text.setString(src.substring(prevPos, i - prevPos));
        if (text.getLocalBounds().width > fitWidth) {
            // possible to wrap
            if (prevWordPos != sf::String::InvalidPos) {
                src[prevWordPos] = '\n';
                prevPos = prevWordPos + 1;
                prevWordPos = sf::String::InvalidPos;
            } else {
                prevPos = i + 1;
            }
        } else {
            prevWordPos = i;
        }
    }

    return src;
}

void fwkCreate(std::vector<std::uintmax_t>& vec, const std::uint32_t* values,
               std::size_t sz) {
    using fwt = Bulletworm::FenwickTree<std::vector<std::uintmax_t>::iterator,
        std::vector<std::uintmax_t>::const_iterator, std::ptrdiff_t, std::uintmax_t>;

    constexpr auto realsize = [](std::size_t val) {
        unsigned int bitlog = 0;
        std::size_t tval = (val ? (val - 1) : 0);
        while (tval) {
            tval >>= 1;
            ++bitlog;
        }
        return (std::size_t)1 + (val ? (((std::size_t)1u) << bitlog) : 0);
    };

    vec.resize(realsize(sz));

    std::copy(values, values + sz, vec.data() + 1);
    std::fill(vec.data() + sz + 1, vec.data() + vec.size(), 0);
    vec[0] = 0;
    fwt::init(vec.begin(), vec.end());
}

}

namespace Bulletworm {


bool BlockSnake::initTextures() {
    // generate tile map
    TextureLoader::Input data{};
    data.count = TextureUnitCount * ThemeCount;
    data.unitWidth = TexUnitWidth;
    data.width = TexSz;
    data.height = TexSz;

    m_textures = std::move(TextureLoader::load(data,
                           m_textureTitles.data()));
    if (m_textures) {
        return m_textures->generateMipmap();
    }

    return false;
}


void BlockSnake::createWindow(bool resetVirtual) {
    const auto& fullscreenModes = sf::VideoMode::getFullscreenModes();
    const auto& fullscreenMode = fullscreenModes.front();
    sf::VideoMode windowMode;

    // set fullscreen mode
    if (!getSetting(SettingEnum::FullscreenEnabled)) {
        windowMode = sf::VideoMode(fullscreenMode.width *
                                   WindowModeRatioNumerator /
                                   WindowModeRatioDenominator,

                                   fullscreenMode.height *
                                   WindowModeRatioNumerator /
                                   WindowModeRatioDenominator);
    } else {
        windowMode = fullscreenMode;
    }

    sf::ContextSettings contextSettings;

    // no antialiasing level because of edge artifacts
    // no srgb capable, seems to be unneccessary

    std::uint32_t smallWindowStyle = (sf::Style::Close |
                                      sf::Style::Resize |
                                      sf::Style::Titlebar);

    std::uint32_t fullscreen = sf::Style::Fullscreen;

    std::uint32_t style = (getSetting(SettingEnum::FullscreenEnabled) ?
                           fullscreen :
                           smallWindowStyle);

    m_window.create(windowMode, GameTitle, style, contextSettings);
    m_window.setKeyRepeatEnabled(false);   // to prevent freezer speedrun
    m_window.setVerticalSyncEnabled(true); // VSync

    m_window.setMouseCursor(m_cursor);
    m_window.setIcon(m_iconImg.getSize().x,
                     m_iconImg.getSize().y,
                     m_iconImg.getPixelsPtr());

    if (resetVirtual) {
        m_virtualWinSize = m_window.getSize();
    } else {
        sf::View view{ sf::FloatRect(0,0,m_virtualWinSize.x,m_virtualWinSize.y) };
        m_window.setView(view);
    }
}


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
        !timeConverted1[0] &&
        !timeConverted1[1] &&
        !timeConverted1[2] &&
        !timeConverted1[3]) {
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
        !timeConverted1[0] &&
        !timeConverted1[1] &&
        !timeConverted1[2] &&
        !timeConverted1[3]) {
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


BlockSnake::BlockSnake() :
    m_logger(LOG_PATH, std::ios::app) {
}


bool BlockSnake::loadStatus() {
    std::vector<std::uint32_t> dataInputDecrypted;

    {
        std::vector<std::uint32_t> dataInput;

        // decryption
        sf::FileInputStream finp;
        if (!finp.open((std::string)pwd + STATUS_PATH)) {
            /*m_logger << "Failed to open status.bin\n";
            return false;*/

            // LAST HACK
            m_settings[(int)SettingEnum::AmbientVolumePer10000] = 3000;
            m_settings[(int)SettingEnum::SoundVolumePer10000] = 3500;
            m_settings[(int)SettingEnum::MusicVolumePer10000] = 5000;
            m_settings[(int)SettingEnum::LanguageIndex] = 0;
            m_settings[(int)SettingEnum::FullscreenEnabled] = 0;
            m_settings[(int)SettingEnum::SnakeHeadPointerEnabled] = 1;

            int lvlcntprep;
            if constexpr (0) {
                lvlcntprep = 3;
            }
            else {
                lvlcntprep = 12;
            }

            m_levelStatistics.m_availableLevelCount = 1;
            m_levelStatistics.m_first[(int)FirstLevelStatisticsEnum::DiffCount] = 3;
            m_levelStatistics.m_first[(int)FirstLevelStatisticsEnum::LevelCount] = lvlcntprep;
            m_levelStatistics.m_first[(int)FirstLevelStatisticsEnum::TotalGametimeLeast32] = 0;
            m_levelStatistics.m_first[(int)FirstLevelStatisticsEnum::TotalGametimeMost32] = 0;

            m_levelStatistics.m_levelCompleted.resize(lvlcntprep * 3, 0);
            m_levelStatistics.m_levelGameCounts.resize(lvlcntprep * 3, 0);
            m_levelStatistics.m_levelScores.resize(lvlcntprep, 0);

            for (int i = 2; i < lvlcntprep; ++i) {
                m_levelStatistics.m_levelCompleted[i] = 2;
            }

            m_levelStatistics.m_totalGameCount = 0;
            m_levelStatistics.m_totalScore = 0;

            return true;
        }

        sf::Int64 sz = finp.getSize();
        if (sz % 32 != 0) { // 4 as uint32 * 8 as for decryption matrix
            m_logger << "status.bin is corrupted: wrong size\n";
            return false;
        }

        // 4 as uint32 * 4 is encryption redundancy factor
        dataInputDecrypted.resize(sz / 16, 0);
        dataInput.resize(sz / 4);
        sf::Int64 read = finp.read(dataInput.data(), sz);
        if (read != sz) {
            m_logger << "Failed to read status.bin\n";
            return false;
        }

        // endianness
        std::for_each(dataInput.begin(), dataInput.end(),
                      [](std::uint32_t& v) {
                          v = n2hl(v);
                      });

        static const std::uint64_t decrMatrix[]{
            53159 ,25843  ,9021 ,20417 ,31113 ,12430 ,26622, 64479,
     1257, 56731, 12394 ,55339 ,36655 , 7528, 27389, 58154,
    53685, 35556 ,21664 ,38741,  5591 ,23267,  7323, 29688,
    27749, 48557, 13589 ,13442, 27650, 63039, 40773, 33230,
    58442, 21503, 48387, 12865, 63032, 43978, 31652, 26584,
     9864, 47303 ,29556, 24419, 17008, 42048, 15144,  3315,
     4921, 40765 ,55227,  8778, 22571,  2738, 21693, 52417,
    50148, 61919 ,  834, 50421, 60698, 52212,  8550, 47579,
        };

        for (std::size_t i = 0; i < dataInput.size(); i += 8) {
            // multiply
            std::uint64_t temp[8]{ 0,0,0,0,0,0,0,0 };
            for (std::size_t j = 0; j < 8; ++j) {
                for (std::size_t k = 0; k < 8; ++k) {
                    temp[j] += (decrMatrix[j * 8 + k] * dataInput[i + k]) %
                        StatusHillEncryptionModulus;
                    temp[j] %= StatusHillEncryptionModulus;
                }
            }
            
            // only copy valuable data (without random salt)
            dataInputDecrypted[i / 4] |= temp[0] % 256;
            dataInputDecrypted[i / 4] |= ((temp[1] % 256) << 8);
            dataInputDecrypted[i / 4] |= ((temp[2] % 256) << 16);
            dataInputDecrypted[i / 4] |= ((temp[3] % 256) << 24);

            dataInputDecrypted[i / 4 + 1] |= temp[4] % 256;
            dataInputDecrypted[i / 4 + 1] |= ((temp[5] % 256) << 8);
            dataInputDecrypted[i / 4 + 1] |= ((temp[6] % 256) << 16);
            dataInputDecrypted[i / 4 + 1] |= ((temp[7] % 256) << 24);
        }

        // checksum (32 * 8 = 256)
        if (dataInput.size() < 8) {
            m_logger << "status.bin is corrupted: wrong size\n";
            return false;
        }

        const BYTE* inputHash = (const BYTE*)&dataInputDecrypted[dataInputDecrypted.size() - 8];
        BYTE buf[SHA256_BLOCK_SIZE];
        SHA256_CTX ctx;

        sha256_init(&ctx);
        sha256_update(&ctx, (BYTE*)dataInputDecrypted.data(), (dataInputDecrypted.size() - 8) * 4);
        sha256_final(&ctx, buf);
        bool pass = !std::memcmp(inputHash, buf, SHA256_BLOCK_SIZE);

        if (!pass) {
            m_logger << "status.bin is corrupted\n";
            /*for (std::size_t i = 0; i < SHA256_BLOCK_SIZE; ++i) {
                m_logger << (std::uint32_t)buf[i] << ", ";
            }
            m_logger << '\n';*/
            return false;
        }
    }

    // opening
    sf::MemoryInputStream minp;
    minp.open(dataInputDecrypted.data(), dataInputDecrypted.size() * 4);

    /*sf::FileInputStream minp;
    if (!minp.open((std::string)pwd + STATUS_PATH)) {
        m_logger << "Failed to open status.bin\n";
        return false;
    }*/

    // loading SETTINGS
    sf::Int64 ctntread =
        minp.read(m_settings.data(),
                  (sf::Int64)sizeof(std::uint32_t) * m_settings.size());

    if (ctntread !=
        (sf::Int64)sizeof(std::uint32_t) * (sf::Int64)m_settings.size())
        return false;

    // endianness!!!
    /*std::for_each(m_settings.begin(), m_settings.end(),
                  [](std::uint32_t& v) {
                      v = n2hl(v);
                  });*/

    // clamping
    if (getSetting(SettingEnum::AmbientVolumePer10000) > 10000)
        m_settings[(std::size_t)SettingEnum::AmbientVolumePer10000] = 10000;

    if (getSetting(SettingEnum::MusicVolumePer10000) > 10000)
        m_settings[(std::size_t)SettingEnum::MusicVolumePer10000] = 10000;

    if (getSetting(SettingEnum::SoundVolumePer10000) > 10000)
        m_settings[(std::size_t)SettingEnum::SoundVolumePer10000] = 10000;

    // loading STATISTICS
    if (!m_levelStatistics.loadFromStream(minp, false)) // !!!!!
        return false;

    return true;
}


bool BlockSnake::loadData() {

    std::vector<std::uint32_t> dataInput;

    {
        // checksum
        sf::FileInputStream finp;
        if (!finp.open((std::string)pwd + DATA_PATH)) {
            m_logger << "Failed to load " << (std::string)pwd + DATA_PATH << "\n";
            return false;
        }

        sf::Int64 sz = finp.getSize();
        if (sz % 4 != 0) {
            m_logger << "data.bin: wrong size\n";
            return false;
        }

        dataInput.resize(sz / 4);
        sf::Int64 read = finp.read(dataInput.data(), sz);
        if (read != sz) {
            m_logger << "Failed to read data.bin\n";
            return false;
        }

        // endianness
        std::for_each(dataInput.begin(), dataInput.end(),
                      [](std::uint32_t& v) {
                          v = n2hl(v);
                      });

        static const BYTE inputHash[SHA256_BLOCK_SIZE] = {
            81, 1, 195, 5, 130, 106, 49, 254, 114, 176, 135, 225,
            28, 249, 241, 154, 231, 100, 46, 77, 80, 76, 176,
            237, 127, 151, 33, 92, 66, 163, 163, 113 };

        BYTE buf[SHA256_BLOCK_SIZE];

        SHA256_CTX ctx;

        sha256_init(&ctx);
        sha256_update(&ctx, (BYTE*)dataInput.data(), dataInput.size() * 4);
        sha256_final(&ctx, buf);
        bool pass = !memcmp(inputHash, buf, SHA256_BLOCK_SIZE);

        if (!pass) {
            m_logger << "data.bin is corrupted\n";
            /*for (std::size_t i = 0; i < SHA256_BLOCK_SIZE; ++i) {
                m_logger << (std::uint32_t)buf[i] << ", ";
            }
            m_logger << '\n';*/
            return false;
        }
    }

    sf::MemoryInputStream minp;
    minp.open(dataInput.data(), dataInput.size() * 4);

    // COLORS
    sf::Int64 ctntread = minp.read(m_colors.data(),
                                      (sf::Int64)sizeof(std::uint32_t) * ColorDstCount);
    if (ctntread != (sf::Int64)sizeof(std::uint32_t) * ColorDstCount)
        return false;

    // endianness
    /*std::for_each(m_colors.begin(), m_colors.end(),
                  [](std::uint32_t& v) {
                      v = n2hl(v);
                  });*/

    // BEHAVIOR
    auto objlog{ ObjectBehaviorLoader::loadFromStream(m_objectBehaviors, minp, false) };
    if (objlog) {
        m_logger << *objlog;
        return false;
    }

    // BEHAVIOR MAP
    ctntread = minp.read(m_objectPreEffects.data(),
                         (sf::Int64)sizeof(std::uint32_t) * m_objectPreEffects.size());
    if (ctntread != (sf::Int64)sizeof(std::uint32_t) *
        (sf::Int64)m_objectPreEffects.size())
        return false;

    // endianness
    /*std::for_each(m_objectPreEffects.begin(), m_objectPreEffects.end(),
                  [](std::uint32_t& v) {
                      v = n2hl(v);
                  });*/

    ctntread = minp.read(m_objectPostEffects.data(),
                         (sf::Int64)sizeof(std::uint32_t) * m_objectPostEffects.size());

    if (ctntread != (sf::Int64)sizeof(std::uint32_t) *
        (sf::Int64)m_objectPostEffects.size())
        return false;

    // endianness
    /*std::for_each(m_objectPostEffects.begin(), m_objectPostEffects.end(),
                  [](std::uint32_t& v) {
                      v = n2hl(v);
                  });*/

    ctntread = minp.read(m_objectTailCapacities1.data(),
                         (sf::Int64)sizeof(std::uint32_t) * m_objectTailCapacities1.size());
    if (ctntread != (sf::Int64)sizeof(std::uint32_t) *
        (sf::Int64)m_objectTailCapacities1.size())
        return false;

    // endianness
    /*std::for_each(m_objectTailCapacities1.begin(), m_objectTailCapacities1.end(),
                  [](std::uint32_t& v) {
                      v = n2hl(v);
                  });*/

    unsigned int diffCount = m_levelStatistics.getDifficultyCount();
    unsigned int levelCount = m_levelStatistics.getLevelCount();

    // LEVELS
    if (!m_levels.loadFromStream(diffCount, levelCount, minp, false))
        return false;

    return true;
}

bool BlockSnake::loadLists() {
    // lists
    auto loadList = [](const std::string& listPath, const std::string& headPath,
                       std::back_insert_iterator<std::vector<std::filesystem::path>> iter) {
                           std::ifstream listfin(listPath);
                           if (listfin) {
                               std::string content;
                               while (std::getline(listfin, content)) {
                                   if (content.empty())
                                       continue;

                                   iter = (std::filesystem::path)headPath / content;
                               }
                           }
    };

    m_soundTitles.reserve(SoundTypeCount);
    m_musicTitles.reserve(3);
    m_shaderTitles.reserve(VisualEffectCount);
    m_textureTitles.reserve((std::size_t)TextureUnitCount * ThemeCount);
    m_fontTitles.reserve(FontCount);
    m_languageTitles.reserve(1);
    m_wallpaperTitles.reserve(1);

    //loadList((std::string)pwd + MUSIC_LIST_PATH, (std::string)pwd + MUSIC_PATH, std::back_inserter(m_musicTitles));
    //loadList((std::string)pwd + SOUND_LIST_PATH, (std::string)pwd + SOUND_PATH, std::back_inserter(m_soundTitles));
    //loadList((std::string)pwd + TEXTURE_LIST_PATH, (std::string)pwd + TEXTURE_PATH, std::back_inserter(m_textureTitles));
    //loadList((std::string)pwd + SHADER_LIST_PATH, (std::string)pwd + SHADER_PATH, std::back_inserter(m_shaderTitles));
    //loadList((std::string)pwd + FONT_LIST_PATH, (std::string)pwd + FONT_PATH, std::back_inserter(m_fontTitles));
    //loadList((std::string)pwd + LANGUAGE_LIST_PATH, (std::string)pwd + LANGUAGE_PATH, std::back_inserter(m_languageTitles));
    //loadList((std::string)pwd + WALLPAPER_LIST_PATH, (std::string)pwd + WALLPAPER_PATH, std::back_inserter(m_wallpaperTitles));

    // TODO: lists don't work well on Linux

    m_fontTitles = {
"font.ttf"
    };

    m_languageTitles = {
"en.lng"
    };

    m_wallpaperTitles = {
"dawn_dark_red.jpg",
"sky_shiny_red_gray.jpg",
"horizon_green.jpg",
"space_blue.jpg",
"space_black.jpg",
"galaxy_blue.jpg",
"galaxy_light_blue.jpg",
"fog.png",
"fog.png",
"underwater.png",
"ocean_darkness.jpg",
"red_clouds_during_sunset.jpg"
    };

    m_soundTitles = {
        "stop_hit.flac",
"critical_error.flac",
"bonus_appearing.flac",
"bonus_disappearing.flac",
"item_eating.flac",
"accelerating_up.flac",
"dying.flac",
"victory.flac",
"time_limit_exceed_signal.flac",
"effect_ending.flac",
"effect_starting.flac",
"level_completing.flac",
"superbonus_appearing.flac",
"superbonus_disappearing.flac",
"accelerating_down.flac",
"instant_superbonus_eating.flac",
"accelerating_default.flac",
"spike_opening.flac",
"forced_rotating.flac",
"failure.flac",
    };

    m_musicTitles = { 
	"-", "-", "-", "-", "-", "-",
	"-", "-", "-", "-", "-", "-",
	"-", "-", "-", "-", "-", "-",
	"-", "-", "-", "-", "-", "-",
	"-", "-", "-", "-", "-", "-",
	"-", "-", "-", "-", "-", "-",
        "-" 
    };

    m_shaderTitles = {
"snake_default.frag",
"snake_slow.frag",
"snake_fast.frag",
"snake_slow_down.frag",
"snake_tail_harmless.frag",
"snake_stopped.frag",
"snake_time_limit_exceed.frag",
"screen_default.frag",
"screen_time_limit_exceed.frag",
"screen_time_limit_warning.frag",
"fruit_default.frag",
"fruit_screen.frag",
"bonus_default.frag",
"bonus_screen.frag",
"bonus_warning.frag",
"bonus_screen_warning.frag",
"superbonus_default.frag",
"superbonus_screen.frag",
"superbonus_warning.frag",
"superbonus_screen_warning.frag",
"challenge_bar_default.frag",
"challenge_bar_completed.frag"
    };

    m_textureTitles = {
"0/screen_lt_corner.png",
"0/screen_horizontal.png",
"0/fruit.png",
"0/bonus.png",
"0/superbonus_unknown.png",
"0/superbonus_slowdown.png",
"0/superbonus_tailharmless.png",
"0/superbonus_tailcollapse.png",
"0/obstacle.png",
"0/spikes_closed.png",
"0/spikes_opened.png",
"0/rotor_weak_t.png",
"0/rotor_strong_t.png",
"0/stopper.png",
"0/accelerator_default.png",
"0/accelerator_down.png",
"0/accelerator_up.png",
"0/bridge.png",
"0/tube_vertical.png",
"0/tube_rt_rotated.png",
"0/combined_tube_cross.png",
"0/combined_tube_lt_rb.png",
"0/void.png",
"0/pointer_t.png",
"0/combined_pointer_lt_rb.png",
"0/combined_rotor_strong_cross.png",
"0/combined_rotor_strong_lt_rb.png",
"0/accelerator_random.png",
"0/accelerator_dihotomic_random.png",

"1/screen_lt_corner.png",
"1/screen_horizontal.png",
"1/fruit.png",
"1/bonus.png",
"1/superbonus_unknown.png",
"1/superbonus_slowdown.png",
"1/superbonus_tailharmless.png",
"1/superbonus_tailcollapse.png",
"1/obstacle.png",
"1/spikes_closed.png",
"1/spikes_opened.png",
"1/rotor_weak_t.png",
"1/rotor_strong_t.png",
"1/stopper.png",
"1/accelerator_default.png",
"1/accelerator_down.png",
"1/accelerator_up.png",
"1/bridge.png",
"1/tube_vertical.png",
"1/tube_rt_rotated.png",
"1/combined_tube_cross.png",
"1/combined_tube_lt_rb.png",
"1/void.png",
"1/pointer_t.png",
"1/combined_pointer_lt_rb.png",
"1/combined_rotor_strong_cross.png",
"1/combined_rotor_strong_lt_rb.png",
"1/accelerator_random.png",
"1/accelerator_dihotomic_random.png",

"2/screen_lt_corner.png",
"2/screen_horizontal.png",
"2/fruit.png",
"2/bonus.png",
"2/superbonus_unknown.png",
"2/superbonus_slowdown.png",
"2/superbonus_tailharmless.png",
"2/superbonus_tailcollapse.png",
"2/obstacle.png",
"2/spikes_closed.png",
"2/spikes_opened.png",
"2/rotor_weak_t.png",
"2/rotor_strong_t.png",
"2/stopper.png",
"2/accelerator_default.png",
"2/accelerator_down.png",
"2/accelerator_up.png",
"2/bridge.png",
"2/tube_vertical.png",
"2/tube_rt_rotated.png",
"2/combined_tube_cross.png",
"2/combined_tube_lt_rb.png",
"2/void.png",
"2/pointer_t.png",
"2/combined_pointer_lt_rb.png",
"2/combined_rotor_strong_cross.png",
"2/combined_rotor_strong_lt_rb.png",
"2/accelerator_random.png",
"2/accelerator_dihotomic_random.png",

"3/screen_lt_corner.png",
"3/screen_horizontal.png",
"3/fruit.png",
"3/bonus.png",
"3/superbonus_unknown.png",
"3/superbonus_slowdown.png",
"3/superbonus_tailharmless.png",
"3/superbonus_tailcollapse.png",
"3/obstacle.png",
"3/spikes_closed.png",
"3/spikes_opened.png",
"3/rotor_weak_t.png",
"3/rotor_strong_t.png",
"3/stopper.png",
"3/accelerator_default.png",
"3/accelerator_down.png",
"3/accelerator_up.png",
"3/bridge.png",
"3/tube_vertical.png",
"3/tube_rt_rotated.png",
"3/combined_tube_cross.png",
"3/combined_tube_lt_rb.png",
"3/void.png",
"3/pointer_t.png",
"3/combined_pointer_lt_rb.png",
"3/combined_rotor_strong_cross.png",
"3/combined_rotor_strong_lt_rb.png",
"3/accelerator_random.png",
"3/accelerator_dihotomic_random.png"
    };

       for (auto& now : m_soundTitles) {
        now = SOUND_PATH + now.string();
    }

    for (auto& now : m_musicTitles) {
        now = MUSIC_PATH + now.string();
    }
for (auto& now : m_shaderTitles) {
        now = SHADER_PATH + now.string();
    }
for (auto& now : m_textureTitles) {
        now = TEXTURE_PATH + now.string();
    }
for (auto& now : m_fontTitles) {
        now = FONT_PATH + now.string();
    }
for (auto& now : m_languageTitles) {
        now = LANGUAGE_PATH + now.string();
    }
for (auto& now : m_wallpaperTitles) {
        now = WALLPAPER_PATH + now.string();
    }

    if (m_soundTitles.size() < SoundTypeCount)
        return false;
    if (m_textureTitles.size() < (std::size_t)TextureUnitCount * ThemeCount)
        return false;
    if (m_shaderTitles.size() < VisualEffectCount)
        return false;
    if (m_fontTitles.size() < FontCount)
        return false;
    if (m_languageTitles.empty())
        return false;
    if (m_wallpaperTitles.empty())
        return false;

    return true;
}


bool BlockSnake::loadWallpapers() {
    // wallpapers
    m_menuWallpaper = std::make_shared<sf::Texture>();

    if (!m_menuWallpaper->loadFromFile(m_wallpaperTitles.front().string()))
        return false;

    m_menuWallpaper->setSmooth(true);

    m_secondCachedWallpaper = m_menuWallpaper;
    m_2cachedWallpaperIndex = 0;
    return true;
}


bool BlockSnake::loadCursor() {
    sf::Image cursorImg;
    if (!cursorImg.loadFromFile((std::string)pwd + CURSOR_PATH)) {
        m_logger << "Cursor loading failure\n";
        return false;
    }

    if (!m_cursor.loadFromPixels(cursorImg.getPixelsPtr(), cursorImg.getSize(),
        sf::Vector2u()))
        return false;

    return true;
}


bool BlockSnake::loadLanguages() {
    unsigned int diffCount = m_levelStatistics.getDifficultyCount();
    unsigned int levelCount = m_levelStatistics.getLevelCount();

    std::size_t prevWordSize = 0;
    for (std::size_t i = 0; i < m_languageTitles.size(); ++i) {
        sf::FileInputStream finp;
        if (!finp.open(m_languageTitles[i].string()))
            return false;
        std::optional<std::string> langLog{
            LanguageLoader::loadFromStream(std::back_inserter(m_words), finp) };
        if (langLog) {
            m_logger << *langLog;
            return false;
        }
        if (m_words.size() - prevWordSize !=
            WordCount + (std::size_t)diffCount * levelCount)
            return false;
        prevWordSize = m_words.size();
    }
    
    return true;
}


void BlockSnake::setupMusic() {
    m_music.setVolume(
        (float)m_settings[(std::size_t)SettingEnum::MusicVolumePer10000] / 100);
    m_music.setRelativeToListener(true);
    m_music.setLoop(true);

    m_ambient.setVolume(
        (float)m_settings[(std::size_t)SettingEnum::AmbientVolumePer10000] / 100);
    m_ambient.setRelativeToListener(true);
    m_ambient.setLoop(true);
}


bool BlockSnake::setupRandomizer() noexcept {
    std::uint64_t random_seed =
        std::chrono::duration_cast<std::chrono::seconds>
        (std::chrono::system_clock::now()
         .time_since_epoch()).count()
        ^
        std::chrono::duration_cast<std::chrono::microseconds>
        (std::chrono::high_resolution_clock::now()
         .time_since_epoch()).count();

#if defined(_MSC_VER) || defined(__GNUC__)
    try {
        random_seed ^= std::random_device()();
    } catch (std::exception& exc) {
        m_logger << "std::random_device: [" << exc.what() << "]. "
            "Contact developers." << std::endl;
        return false;
    }
#endif

    m_randomizer.setSeed(random_seed);
    return true;
}


bool BlockSnake::start() {

    if (!setupRandomizer())
        return false;

    // AVAILIBILITY

    // shader
    if (!sf::Shader::isAvailable()) {
        m_logger << "Shaders are not available!" << std::endl;
        return false;
    }

    // vertex buffer
    if (!sf::VertexBuffer::isAvailable()) {
        m_logger << "Vertex buffers are not available!" << std::endl;
        return false;
    }

    if (!loadStatus())
        return false;
    if (!loadData())
        return false;
    if (!loadLists())
        return false;

    if (getSetting(SettingEnum::LanguageIndex) >= m_languageTitles.size()) {
        m_settings[(std::size_t)SettingEnum::LanguageIndex] = 0;
    }

    // textures (separated thread)
    bool textureSuccess = false;
    std::thread textureLoader([&textureSuccess, this]() {
        textureSuccess = initTextures();
                              });

    if (!m_digitTexture.loadFromFile((std::string)pwd + DIGITS_PATH))
        return false;
    if (!loadWallpapers())
        return false;
    if (!loadCursor())
        return false;
    
    // icon
    if (!m_iconImg.loadFromFile((std::string)pwd + ICON_PATH)) {
        m_logger << "Icon loading failure\n";
        return false;
    }

    // FONTS
    for (int i = 0; i < FontCount; ++i) {
        if (!m_fonts[i].loadFromFile(m_fontTitles[i].string())) {
            m_logger << "Font " << i << "loading failure\n";
            return false;
        }
        //m_fonts[i].setSmooth(true);
    }

    if (!loadLanguages()) {
        return false;
    }

    // init shaders
    for (int i = 0; i < VisualEffectCount; ++i) {
        if (!m_shaders[i].loadFromFile(m_shaderTitles[i].string(), sf::Shader::Fragment)) {
            m_logger << "Shader loading failure (nr " << i << ")\n";
            return false;
        }
    }

    // init sounds
    if (!m_soundPlayer.loadSounds(m_soundTitles.data())) {
        m_logger << "Sound loading failure\n";
        return false;
    }

    setupMusic();

    // std::rand is used for effects, so pure randomness is unneccessary
    std::srand((unsigned int)std::time(NULL));

    // setup the shaders with the texture

    auto shaderInit = [](sf::Shader& shader) {
        shader.setUniform("texture", sf::Shader::CurrentTexture);
    };

    // not all shaders have texture!
    std::for_each(m_shaders.begin(), m_shaders.end(), shaderInit);

    m_background.setColor(getDestinationColor(ColorDst::Background));

    // creating the window right before the main loop
    createWindow(true);

    textureLoader.join();
    if (!textureSuccess) {
        m_logger << "Texture loading failure\n";
        return false;
    }

    changeWallpaper(0, sf::Vector2f(m_virtualWinSize));

    if (MenuMusicId < m_musicTitles.size() &&
        m_music.openFromFile(m_musicTitles[MenuMusicId].string())) {
        m_music.play();
    }

    m_ambient.stop();

    // the main processes
    mainLoop();
    
    // ended
    if (!saveStatus()) {
        return false;
    }

    return true; // success
}


bool BlockSnake::saveStatusSub() const {
    std::vector<std::uint8_t> dataOutput;
    MemoryOutputStream moutp(dataOutput);

    sf::Int64 ctntwrtitten =
        moutp.write(m_settings.data(),
                    (sf::Int64)sizeof(std::uint32_t) * m_settings.size());

    if (ctntwrtitten != (sf::Int64)sizeof(std::uint32_t) *
        (sf::Int64)m_settings.size())
        return false; // TODO

    if (!m_levelStatistics.saveToStream(moutp, false)) {
        return false; // pity
    }

    dataOutput.resize(((dataOutput.size() + SHA256_BLOCK_SIZE + 7) / 8) * 8);

    // sha256
    // checksum (32 * 8 = 256)
    BYTE buf[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;

    sha256_init(&ctx);
    sha256_update(&ctx, (BYTE*)dataOutput.data(), dataOutput.size() - SHA256_BLOCK_SIZE);
    sha256_final(&ctx, buf);

    std::memcpy(dataOutput.data() + dataOutput.size() - SHA256_BLOCK_SIZE, buf, SHA256_BLOCK_SIZE);

    std::vector<std::uint32_t> dataOutputRedundant(dataOutput.size());
    std::copy(dataOutput.begin(), dataOutput.end(), dataOutputRedundant.begin());

    std::for_each(dataOutputRedundant.begin(), dataOutputRedundant.end(),
                  [](std::uint32_t& v) {
                      // TODO rand() function
                      std::uint32_t rnd = std::rand() % 256;
                      rnd <<= 8;
                      v |= rnd;
                  });

    static const std::uint64_t encrMatrix[]{

        56090, 61794, 45987, 29516, 34927, 45430, 52120, 9950,
            48516, 42162, 32238, 4480, 50349, 11960, 44198, 32197,
            17576, 61425, 60052, 40382, 57017, 29627, 1802, 52337,
            7058, 42863, 10493, 7891, 57687, 62805, 6312, 23381,
            4665, 37463, 49672, 14889, 48033, 60641, 19507, 36184,
            22893, 7020, 36016, 37643, 18495, 6603, 40894, 59865,
            14007, 50647, 52360, 26895, 33620, 45878, 43403, 26459,
            11025, 22914, 17603, 35785, 26814, 55503, 65395, 56252,
    };

    for (std::size_t i = 0; i < dataOutputRedundant.size(); i += 8) {
        // multiply
        std::uint64_t temp[8]{ 0,0,0,0,0,0,0,0 };
        for (std::size_t j = 0; j < 8; ++j) {
            for (std::size_t k = 0; k < 8; ++k) {
                temp[j] += (encrMatrix[j * 8 + k] * dataOutputRedundant[i + k]) %
                    StatusHillEncryptionModulus;
                temp[j] %= StatusHillEncryptionModulus;
            }
        }

        for (std::size_t j = 0; j < 8; ++j)
            dataOutputRedundant[i + j] = (std::uint32_t)temp[j];
    }

    // endianness
    std::for_each(dataOutputRedundant.begin(), dataOutputRedundant.end(),
                  [](std::uint32_t& v) {
                      v = h2nl(v);
                  });

    FileOutputStream foutp;

    if (!foutp.open((std::string)pwd + STATUS_PATH)) {
        m_logger << STATUS_PATH << " access denied :(" << std::endl;
        return false;
    }

    if (foutp.write(dataOutputRedundant.data(), dataOutputRedundant.size() * 4) !=
        (sf::Int64)dataOutputRedundant.size() * 4) {
        m_logger << "Failed to save status.bin!\n";
        return false;
    }

    return true;
}


bool BlockSnake::saveStatus() {
    if (!saveStatusSub()) {
        SoundThrower::Parameters param;
        param.relativeToListener = true;
        param.volume = (float)m_settings[(std::size_t)SettingEnum::SoundVolumePer10000] / 100;
        m_soundPlayer.playSound(SoundType::CriticalError, param);
        return false;
    }
    return true;
}


void BlockSnake::changeWallpaper(unsigned int id,
                                 const sf::Vector2f& windowSize) {
  // m_menuWallpaper
  // m_background
  // m_wallpaperTitles
  // m_2cachedWallpaperIndex
  // m_secondCachedWallpaper

  // zero -> zero
    if (id == 0 && m_menuWallpaper.get() == m_background.getTexture())
        return;

      // is it possible?
    if (id >= m_wallpaperTitles.size())
        return;

    bool changed = false;

    if (id == 0) // non-zero -> zero
    {
      // just reset
        m_background.setTexture(*m_menuWallpaper, true);
        changed = true;
    } else // ? -> non-zero
    {
        if (id == m_2cachedWallpaperIndex) // to cached non-zero
        {
            if (m_background.getTexture() != m_secondCachedWallpaper.get()) {
                m_background.setTexture(*m_secondCachedWallpaper, true);
                changed = true;
            }
        } else // load new
        {
            if (m_2cachedWallpaperIndex == 0) {
                m_secondCachedWallpaper.reset();
                assert(m_menuWallpaper.use_count() == 1);
                m_secondCachedWallpaper = std::make_shared<sf::Texture>();
            }

            if (m_secondCachedWallpaper->loadFromFile(m_wallpaperTitles[id].string())) {
                m_secondCachedWallpaper->setSmooth(true);
                m_2cachedWallpaperIndex = id;
                m_background.setTexture(*m_secondCachedWallpaper, true);
                changed = true;
            } else {
                if (m_2cachedWallpaperIndex == 0)
                    m_secondCachedWallpaper = m_menuWallpaper;
            }
        }
    }

    if (changed) {
        sf::Vector2f imageSize(float(m_background.getTextureRect().width),
                               float(m_background.getTextureRect().height));
        sf::Vector2f ratios(windowSize.x / imageSize.x, windowSize.y / imageSize.y);
        float ratio = std::max(ratios.x, ratios.y);

        m_background.setOrigin(imageSize.x / 2, imageSize.y / 2);
        m_background.setPosition(windowSize.x / 2, windowSize.y / 2);
        m_background.setScale(ratio, ratio);
    }
}

const sf::String& BlockSnake::getWord(std::size_t lang, Word word) const noexcept {
    return m_words[lang * ((std::size_t)WordCount +
                           (std::size_t)
                           m_levelStatistics.getDifficultyCount() *
                           m_levelStatistics.getLevelCount()) +
        (int)word];
}

const sf::String& BlockSnake::getLevelDescr(unsigned int lang,
                                            unsigned int level,
                                            unsigned int diff) const noexcept {
    return m_words[(std::size_t)lang * ((std::size_t)WordCount +
                                        (std::size_t)m_levelStatistics.getDifficultyCount() *
                                        m_levelStatistics.getLevelCount()) +
        WordCount + diff + (std::size_t)level *
        m_levelStatistics.getDifficultyCount()];
}


void BlockSnake::mainLoop() {
    bool mainAgain = true;
    do {
        switch (mainMenu()) {
        case MainMenuCommand::Play:
            mainAgain = selectLevelProcessing(); // the main part
            break;
        case MainMenuCommand::Settings:
            mainAgain = settings(); // little branch
            break;
        case MainMenuCommand::Manual:
            mainAgain = manual(); // little branch
            break;
        case MainMenuCommand::Languages:
            mainAgain = languages(); // little branch
            break;
        case MainMenuCommand::Exit:
        default:
            mainAgain = false;
            break;
        }

    } while (mainAgain);
}


bool BlockSnake::selectLevelProcessing() {
    switch (selectLevel()) // the branch with level selecting menu
    {
    case LevelMenuCommand::Back:
        return true;
    case LevelMenuCommand::Selected:
        if (playGame()) // level selected, start the game
            return true;
        break;
    case LevelMenuCommand::Exit:
    default:
        break;
    }

    return false;
}


bool BlockSnake::playGame() {

    // check snake full view size

    const std::uint32_t* plotPtr =
        m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);

    sf::Vector2u snakeFullViewSize;

    using Lpde = LevelPlotDataEnum;

    snakeFullViewSize.x = plotPtr[(int)Lpde::SnakeSightX] * 2 + 1;
    snakeFullViewSize.y = plotPtr[(int)Lpde::SnakeSightY] * 2 + 1;

    sf::Vector2i snakeScreenViewSize;
    snakeScreenViewSize.x = plotPtr[(int)Lpde::SnakeSightX] * 2 + 3;
    snakeScreenViewSize.y = plotPtr[(int)Lpde::SnakeSightY] * 2 + 3;

    const sf::Vector2u& mapSize = m_levels.getMapSize(m_difficulty, m_levelIndex);

    if (snakeFullViewSize.x > mapSize.x)
        return false;
    if (snakeFullViewSize.y > mapSize.y)
        return false;

    // setup themes

    m_gameDrawable.centralView.setupThemes(plotPtr[(int)Lpde::ScreenTheme],
                                           plotPtr[(int)Lpde::FruitTheme],
                                           plotPtr[(int)Lpde::BonusTheme],
                                           plotPtr[(int)Lpde::SuperbonusTheme]);

    // change wallpaper

    // must be joined
    std::thread wallpaperChanging(&BlockSnake::changeWallpaper,
                                  this,
                                  plotPtr[(int)Lpde::BackgroundIndex],
                                  sf::Vector2f(m_virtualWinSize));

    sf::Vector2f windowSizef = static_cast<sf::Vector2f>(m_virtualWinSize);

    if (!m_gameDrawable.initConfig(windowSizef,
        snakeFullViewSize, *m_textures, m_digitTexture,
        getDestinationIntColor(ColorDst::SnakeBodyFill),
        getDestinationIntColor(ColorDst::SnakeBodyOutline),
        getDestinationIntColor(ColorDst::SnakePointerFill),
        getDestinationIntColor(ColorDst::SnakePointerOutline),
        getDestinationIntColor(ColorDst::Score),
        getDestinationIntColor(ColorDst::HighestScore),
        plotPtr[(std::size_t)Lpde::FoggColor])) {
        
        wallpaperChanging.join(); // !
        return false;
    }

    createChallVisual();

    m_toReturn = true;
    m_gameAgain = true;

    prepareGame();

    bool wallpaperChangingJoined = false;

    do // levels' loop
    {
        m_levelComplete = false;

        m_game.restart(m_initialObjectMemory.data());
        playGameMusic();

        sf::Listener::setPosition((float)m_game.getImpl()
                                  .getSnakeWorld()
                                  .getCurrentSnakePosition().x,
                                  (float)m_game.getImpl()
                                  .getSnakeWorld()
                                  .getCurrentSnakePosition().y, 
                                  0);

        m_toExit = false;
        m_currBonusEatenCount = 0;
        m_currFruitEatenCount = 0;
        m_currPowerupEatenCount = 0;
        m_currStepCount = 0;
        m_rotatedPostEffect = false;

        m_snakeTailEndVisible = false;
        m_snakeTailPreendVisible = false;

        m_window.setMouseCursorVisible(false);

        m_gameClock.stop<sf::Int64, std::micro>();

        updateGame();

        m_gameDrawable.highestScore
            .setNumber(m_levelStatistics
                       .getLevelHighestScore(m_levelIndex));

        m_currScore = 0;

        if (!wallpaperChangingJoined) {
            wallpaperChanging.join();
            wallpaperChangingJoined = true;
        }

        m_gameClock.restart<sf::Int64, std::micro>();

        /*sf::Clock responseRatioClock;
        long long debugRRC = 0;
        sf::Time time2;
        sf::Time time1;*/

        // GAME LOOP
        while (!m_toExit) {
            m_nowTime = m_gameClock.getElapsedTime<
                sf::Int64,
                std::micro>();

            processEvents();

            m_game.update(m_nowTime);
            processGameEvents();
            scaleUpdate();
            drawWindow();
        }

        endGame();

    } while (m_gameAgain);

    // final

    // to menu or exit?
    if (m_toReturn) {

        // music
        if (MenuMusicId < m_musicTitles.size() &&
            m_music.openFromFile(m_musicTitles[MenuMusicId].string())) {
        #if !0
            m_music.play();
        #endif
        }
        
        // ambience
        m_ambient.stop();
        
        // wallpaper
        changeWallpaper(0, sf::Vector2f(m_virtualWinSize));
    }

    return m_toReturn;
}


void BlockSnake::createChallVisual() {
  // Some links
    const std::uint32_t* plotPtr = m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);
    const std::uint32_t* attribPtr = m_levels.getLevelAttribPtr(m_difficulty, m_levelIndex);

    auto dstintcol = [this](ColorDst dst) {return getDestinationIntColor(dst); };
    auto dstcol = [this](ColorDst dst) {return getDestinationColor(dst); };

    if (plotPtr[(int)LevelPlotDataEnum::ChallengeCount] >= 1) {
        m_gameDrawable.challengeVisual.setCount(100);
        m_gameDrawable.challengeVisualOutline.setPointCount(100);
        m_gameDrawable.challengeVisualOutline.setOutlineThickness(5.f);

        int challIndex = static_cast<int>(plotPtr[(int)LevelPlotDataEnum::Challenge]);

        switch (challIndex) {
        case 0:
            m_gameDrawable.challengeVisual.setColor(dstintcol(ColorDst::FruitChallengeVisual));
            break;
        case 1:
            m_gameDrawable.challengeVisual.setColor(dstintcol(ColorDst::BonusChallengeVisual));
            break;
        case 2:
            m_gameDrawable.challengeVisual.setColor(dstintcol(ColorDst::SuperbonusChallengeVisual));
            break;
        default:
            break;
        }

        m_gameDrawable.challengeVisualOutline.setOutlineColor(dstcol(ColorDst::ChallengeVisualOutline));
        m_gameDrawable.challengeVisualOutline.setFillColor(dstcol(ColorDst::ChallengeVisualOutlineFill));

        m_gameDrawable.challengeVisual.setPosition(5.f, 5.f);
        m_gameDrawable.challengeVisualOutline.setPosition(5.f, 5.f);
    }

    unsigned int fruitCountToBonus = attribPtr[(int)LevelAttribEnum::FruitCountToBonus];

    unsigned int bonusCountToPowerup = attribPtr[(int)LevelAttribEnum::BonusCountToSuperbonus];

    if (fruitCountToBonus >= 1) {
        m_gameDrawable.fruitCountToBonusVisual.setCount(100);
        m_gameDrawable.fruitCountToBonusVisualOutline.setPointCount(100);
        m_gameDrawable.fruitCountToBonusVisualOutline.setOutlineThickness(5.f);
        m_gameDrawable.fruitCountToBonusVisual.setColor(dstintcol(ColorDst::F2Bvisual));
        m_gameDrawable.fruitCountToBonusVisualOutline.setOutlineColor(dstcol(ColorDst::F2BvisualOutline));
        m_gameDrawable.fruitCountToBonusVisualOutline.setFillColor(dstcol(ColorDst::F2BvisualOutlineFill));

        float radius = m_gameDrawable.fruitCountToBonusVisual.getRadius();
        m_gameDrawable.fruitCountToBonusVisual.setOrigin(0, radius * 2);
        m_gameDrawable.fruitCountToBonusVisualOutline.setOrigin(0, radius * 2);
        m_gameDrawable.fruitCountToBonusVisual.setPosition(0, float(m_virtualWinSize.y));
        m_gameDrawable.fruitCountToBonusVisualOutline.setPosition(0, float(m_virtualWinSize.y));

        m_gameDrawable.fruitCountToBonusVisual.move(5.f, -5.f);
        m_gameDrawable.fruitCountToBonusVisualOutline.move(5.f, -5.f);
    }

    if (bonusCountToPowerup >= 1) {
        m_gameDrawable.bonusCountToPowerupVisual.setCount(100);
        m_gameDrawable.bonusCountToPowerupVisualOutline.setPointCount(100);
        m_gameDrawable.bonusCountToPowerupVisualOutline.setOutlineThickness(5.f);
        m_gameDrawable.bonusCountToPowerupVisual.setColor(dstintcol(ColorDst::B2Svisual));
        m_gameDrawable.bonusCountToPowerupVisualOutline.setOutlineColor(dstcol(ColorDst::B2SvisualOutline));
        m_gameDrawable.bonusCountToPowerupVisualOutline.setFillColor(dstcol(ColorDst::B2SvisualOutlineFill));

        float radius = m_gameDrawable.bonusCountToPowerupVisual.getRadius();
        m_gameDrawable.bonusCountToPowerupVisual.setOrigin(radius * 2, radius * 2);
        m_gameDrawable.bonusCountToPowerupVisualOutline.setOrigin(radius * 2,
                                                                  radius * 2);
        m_gameDrawable.bonusCountToPowerupVisual.setPosition(float(m_virtualWinSize.x),
                                                             float(m_virtualWinSize.y));
        m_gameDrawable.bonusCountToPowerupVisualOutline.setPosition(float(m_virtualWinSize.x),
                                                                    float(m_virtualWinSize.y));

        m_gameDrawable.bonusCountToPowerupVisual.move(-5.f, -5.f);
        m_gameDrawable.bonusCountToPowerupVisualOutline.move(-5.f, -5.f);
    }
}


void BlockSnake::prepareGame() {
  // Some links

    GameImpl::LevelPointers levelPtrs;
    levelPtrs.attribArray = m_levels.getLevelAttribPtr(m_difficulty, m_levelIndex);
    levelPtrs.effectDurations = m_levels.getEffectDurationPtr(m_difficulty, m_levelIndex);
    levelPtrs.powerupProbs = &m_levels.getPowerupProbs(m_difficulty, m_levelIndex);

    levelPtrs.objectBehs = m_objectBehaviors.data();
    levelPtrs.postEffectBehIndices = m_objectPostEffects.data();
    levelPtrs.preEffectBehIndices = m_objectPreEffects.data();
    levelPtrs.tailCapacities1 = m_objectTailCapacities1.data();

    const sf::Vector2u& mapSize = m_levels.getMapSize(m_difficulty, m_levelIndex);
    std::size_t area{ (std::size_t)mapSize.x * mapSize.y };

    m_currentObjPairIndices.resize(area);
    m_currentObjParams.resize(area);
    m_currentThemes.resize(area);

    std::vector<std::uint32_t> forProbs(area);

    auto cmfunc = [&area](std::vector<std::uint32_t>& vect, const std::uint32_t* cm) {
        std::size_t cmi = 0;
        for (std::size_t ii = 0; cmi < area; ii += 2) {
            std::uint32_t what = cm[ii + 1];
            for (std::uint32_t j = 0; j < cm[ii]; ++j, ++cmi)
                vect[cmi] = what;
        }
    };

    m_initialObjectMemory.resize(area);

    cmfunc(m_currentThemes, m_levels.getLevelCountMap(LevelCountMap::Theme,
           m_difficulty, m_levelIndex));
    cmfunc(m_currentObjPairIndices, m_levels.getLevelCountMap(LevelCountMap::ObjPair,
           m_difficulty, m_levelIndex));
    cmfunc(m_currentObjParams, m_levels.getLevelCountMap(LevelCountMap::Param,
           m_difficulty, m_levelIndex));
    cmfunc(m_initialObjectMemory, m_levels.getLevelCountMap(LevelCountMap::Memory,
           m_difficulty, m_levelIndex));
    cmfunc(forProbs, m_levels.getLevelCountMap(LevelCountMap::SnakeStartPos,
           m_difficulty, m_levelIndex));

    fwkCreate(m_currentSnakePosProbs, forProbs.data(), forProbs.size());

    for (int i = 0; i < ItemCount; ++i) {
        cmfunc(forProbs, m_levels.getItemProbCountMap(EatableItem(i),
               m_difficulty, m_levelIndex));
        m_currentItemProbabilities[i].create(mapSize, forProbs.data());
    }

    levelPtrs.objectPairIndices = m_currentObjPairIndices.data();
    levelPtrs.objectParams = m_currentObjParams.data();
    levelPtrs.snakePositionProbs = &m_currentSnakePosProbs;

    std::array<Randomizer*, RandomTypeCount> allRands{};
    allRands.fill(&m_randomizer);

    std::array<const Map<std::uint32_t>*, ItemCount> itemProbPtrs{};
    std::transform(m_currentItemProbabilities.begin(),
                   m_currentItemProbabilities.end(),
                   itemProbPtrs.begin(),
                   [](const Map<std::uint32_t>& src) { return &src; });

    m_game.restart(
        GameImpl{ levelPtrs, allRands.data(), m_initialObjectMemory.data(), itemProbPtrs.data() });
}


void BlockSnake::playGameMusic() {
  // Some links
    const std::uint32_t* plotPtr =
        m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);

    if (plotPtr[(int)LevelPlotDataEnum::MusicEnabled] &&
        plotPtr[(int)LevelPlotDataEnum::MusicIndex] < m_musicTitles.size() &&
        m_music.openFromFile(m_musicTitles[plotPtr[(int)LevelPlotDataEnum::MusicIndex]].string())) {
    #if !0
        m_music.play();
    #endif
    }

    if (plotPtr[(int)LevelPlotDataEnum::AmbientEnabled] &&
        plotPtr[(int)LevelPlotDataEnum::AmbientIndex] < m_musicTitles.size() &&
        m_ambient.openFromFile(m_musicTitles[plotPtr[(int)LevelPlotDataEnum::AmbientIndex]].string())) {
    #if !0
        m_ambient.play();
    #endif
    }
}


void BlockSnake::updateGame() {
    m_gameDrawable.centralView.clear();

    updateUnits();
    updateItems(EatableItem::Fruit);
    updateItems(EatableItem::Bonus);
    updateItems(EatableItem::Powerup);
    updateSnakeDrawable();

    (void)m_gameDrawable.centralView.updateVBs();
}


sf::IntRect BlockSnake::getInnerVisibleZone() const {
  // Some links (constant!)
    const std::uint32_t* plotPtr =
        m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);
    sf::Vector2i mapSize{ m_levels.getMapSize(m_difficulty, m_levelIndex) };

    const GameImpl& gameImpl = m_game.getImpl();
    const SnakeWorld& snakeWorld = gameImpl.getSnakeWorld();
    sf::Vector2i snakePos = snakeWorld.getCurrentSnakePosition();

    ///////////////////////////////////////////////////////////////////////////

    // corners

    sf::Vector2i leftTopInMap = snakePos;
    leftTopInMap.x -= plotPtr[(int)LevelPlotDataEnum::SnakeSightX];
    leftTopInMap.y -= plotPtr[(int)LevelPlotDataEnum::SnakeSightY];

    sf::Vector2i rightDownInMap = snakePos;
    rightDownInMap.x += plotPtr[(int)LevelPlotDataEnum::SnakeSightX];
    rightDownInMap.y += plotPtr[(int)LevelPlotDataEnum::SnakeSightY];

    bool cameraStopped = isCameraStopped(m_nowTime);

    if (!cameraStopped) {
        switch (m_game.getImpl().getSnakeWorld().getPreviousDirection()) {
        case Direction::Up:
            ++rightDownInMap.y;
            break;
        case Direction::Down:
            --leftTopInMap.y;
            break;
        case Direction::Left:
            ++rightDownInMap.x;
            break;
        case Direction::Right:
            --leftTopInMap.x;
            break;
        default:
            break;
        }
    }

    // prevent camera overshift

    // x
    if (leftTopInMap.x < 0) {
        rightDownInMap.x -= leftTopInMap.x;
        leftTopInMap.x = 0;
    } else if (rightDownInMap.x >= mapSize.x) {
        sf::Vector2i previousRightDown = rightDownInMap;
        rightDownInMap.x = mapSize.x - 1;
        leftTopInMap += rightDownInMap - previousRightDown;
    }

    // y
    if (leftTopInMap.y < 0) {
        rightDownInMap.y -= leftTopInMap.y;
        leftTopInMap.y = 0;
    } else if (rightDownInMap.y >= mapSize.y) {
        sf::Vector2i previousRightDown = rightDownInMap;
        rightDownInMap.y = mapSize.y - 1;
        leftTopInMap += rightDownInMap - previousRightDown;
    }

    return sf::IntRect(leftTopInMap, rightDownInMap +
                       sf::Vector2i(1, 1) - leftTopInMap);
}


bool BlockSnake::isCameraStopped(sf::Int64 nowTime) const {
    // detect whether the camera has stopped

    // some info
    const std::uint32_t* plotPtr =
        m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);

    const sf::Vector2u& mapSize = m_levels.getMapSize(m_difficulty, m_levelIndex);

    const GameImpl& gameImpl = m_game.getImpl();
    const SnakeWorld& snakeWorld = gameImpl.getSnakeWorld();

    Direction prevDir = snakeWorld.getPreviousDirection();

    // snake stands
    if (prevDir == Direction::Count)
        return true;

    // snake delaying
    //sf::Int64 delta = nowTime - m_lastMoveEventTimePoint;
    //sf::Int64 factualPer = gameImpl.getFactualSnakePeriod();

    //// period exceeded
    //if (delta >= factualPer)
    //    return true;

    // camera collided with border?

    const sf::Vector2i& snakePosition = snakeWorld.getCurrentSnakePosition();
    sf::Vector2i mapSizei(mapSize);

    using Lpde = LevelPlotDataEnum;

    switch (prevDir) {
    case Direction::Up:
        return (snakePosition.y <
                (std::int64_t)plotPtr[(int)Lpde::SnakeSightY]) ||
            (snakePosition.y + 1 >=
             mapSizei.y - (std::int64_t)plotPtr[(int)Lpde::SnakeSightY]);
    case Direction::Right:
        return (snakePosition.x <
                (std::int64_t)plotPtr[(int)Lpde::SnakeSightX] + 1) ||
            (snakePosition.x >=
             mapSizei.x - (std::int64_t)plotPtr[(int)Lpde::SnakeSightX]);
    case Direction::Down:
        return (snakePosition.y <
                (std::int64_t)plotPtr[(int)Lpde::SnakeSightY] + 1) ||
            (snakePosition.y >=
             mapSizei.y - (std::int64_t)plotPtr[(int)Lpde::SnakeSightY]);
    case Direction::Left:
        return (snakePosition.x <
                (std::int64_t)plotPtr[(int)Lpde::SnakeSightX]) ||
            (snakePosition.x + 1 >=
             mapSizei.x - (std::int64_t)plotPtr[(int)Lpde::SnakeSightX]);
    default:
        break;
    }


    assert(false);
    return false;
}


void BlockSnake::updateUnits() {
    const sf::Vector2u& mapSize = m_levels.getMapSize(m_difficulty, m_levelIndex);

    sf::IntRect innerZone = getInnerVisibleZone();
    sf::Vector2i leftTopInMap(innerZone.left, innerZone.top);
    sf::Vector2i rightDownInMap = leftTopInMap +
        sf::Vector2i(innerZone.width, innerZone.height) - sf::Vector2i(1, 1);

    for (int x = leftTopInMap.x; x <= rightDownInMap.x; ++x) {
        for (int y = leftTopInMap.y; y <= rightDownInMap.y; ++y) {
            sf::Vector2i currentInInnerView(x, y);
            currentInInnerView -= leftTopInMap;
            ObjectPair theelem = (ObjectPair)m_game.getImpl()
                .getLevelPointers().objectPairIndices[x + y * mapSize.x];
            std::uint32_t theparam =
                m_game.getImpl().getLevelPointers().objectParams[x + y * mapSize.x];
            std::uint32_t thetheme = m_currentThemes[x + (std::size_t)y * mapSize.x];

            using Orn = Orientation;
            using Txut = TextureUnit;
            
            switch (theelem) {
            case ObjectPair::Spikes:
                if (m_game.getImpl().getObjectMemory(x, y))
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::SpikesOpened,
                                                         thetheme, Orn::Identity);
                else
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::SpikesClosed,
                                                         thetheme, Orn::Identity);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);

                break;
            case ObjectPair::Bridge:
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Bridge,
                                                     thetheme, Orn::Identity);
                m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            case ObjectPair::Obstacle:
                m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                     Txut::Obstacle,
                                                     thetheme, Orn::Identity);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            case ObjectPair::RotorWeak: {
                Orn orient{};
                switch (theparam) {
                case 0: orient = Orn::Identity; break;
                case 1: orient = Orn::RotateClockwise; break;
                case 2: orient = Orn::Flip; break;
                case 3: orient = Orn::RotateCounterClockwise; break;
                default: break;
                }
                m_gameDrawable.centralView.pushBgObj(
                    currentInInnerView, Txut::RotorWeak, thetheme, orient);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            }
            case ObjectPair::RotorStrong: {
                Orn orient{};
                switch (theparam) {
                case 0: orient = Orn::Identity; break;
                case 1: orient = Orn::RotateClockwise;    break;
                case 2:  orient = Orn::Flip;     break;
                case 3: orient = Orn::RotateCounterClockwise;   break;
                default:     break;
                }
                m_gameDrawable.centralView.pushBgObj(
                    currentInInnerView, Txut::RotorStrong,
                    thetheme, orient);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            }
            case ObjectPair::Tube: {
                Orn orient{};
                switch (theparam) {
                case 0:   orient = Orn::Identity;  break;
                case 1:   orient = Orn::Identity;  break;
                case 2:   orient = Orn::RotateCounterClockwise;  break;
                case 3:  orient = Orn::RotateClockwise;  break;
                case 4:  orient = Orn::RotateClockwise;  break;
                case 5:   orient = Orn::Flip;  break;
                default:   break;
                }
                if (theparam == 1 || theparam == 4)
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::TubeStraight,
                                                         thetheme, orient);
                else
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::TubeRotated,
                                                         thetheme, orient);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            }
            case ObjectPair::CombinedTube: {
                Orn orient{};
                switch (theparam) {
                case 0:  orient = Orn::Identity;  break;
                case 1:  orient = Orn::Identity;  break;
                case 2:  orient = Orn::RotateClockwise;  break;
                default:  break;
                }
                if (theparam == 1)
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::CombinedTubeCross,
                                                         thetheme, orient);
                else
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::CombinedTubeRotated,
                                                         thetheme, orient);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            }
            case ObjectPair::Void:
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::Void,
                                                         thetheme, Orn::Identity);
                    m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                         Txut::Void,
                                                         thetheme, Orn::Identity);
                break;
            case ObjectPair::Stopper:
                m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                     Txut::Stopper,
                                                     thetheme, Orn::Identity);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            case ObjectPair::Accelerator: {
                switch (theparam) {
                case 0:
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::AccDefault,
                                                         thetheme, Orn::Identity);
                    break;
                case 1:
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::AccDown,
                                                         thetheme, Orn::Identity);
                    break;
                case 2:
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::AccUp,
                                                         thetheme, Orn::Identity);
                    break;
                default:
                    break;
                }
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            }
            case ObjectPair::Pointer: {
                Orn orient{};
                switch (theparam) {
                case 0: orient = Orn::Identity; break;
                case 1: orient = Orn::RotateClockwise;  break;
                case 2: orient = Orn::Flip;  break;
                case 3:  orient = Orn::RotateCounterClockwise; break;
                default: break;
                }
                m_gameDrawable.centralView.pushBgObj(
                    currentInInnerView, Txut::Pointer, thetheme, orient);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            }
            case ObjectPair::CombinedPointer: {
                Orn orient{};
                switch (theparam) {
                case 0:          orient = Orn::Identity;          break;
                case 1:          orient = Orn::Identity;          break;
                case 2:          orient = Orn::RotateClockwise;   break;
                default: break;
                }
                if (theparam == 1)
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::Void,
                                                         thetheme, Orn::Identity);
                else
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::CombinedPointerRotated,
                                                         thetheme, orient);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            }
            case ObjectPair::CombinedRotorStrong: {
                Orn orient{};
                switch (theparam) {
                case 0:          orient = Orn::Identity;          break;
                case 1:          orient = Orn::Identity;          break;
                case 2:          orient = Orn::RotateClockwise;
                break;        default:          break;
                }
                if (theparam == 1)
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::CombinedRotorStrongCross,
                                                         thetheme, orient);
                else
                    m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                         Txut::CombinedRotorStrongRotated,
                                                         thetheme, orient);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            }
            case ObjectPair::RandomAccelerator:
                m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                     Txut::RandomAccelerator,
                                                     thetheme, Orn::Identity);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            case ObjectPair::RandomDihotomicAccelerator:
                m_gameDrawable.centralView.pushBgObj(currentInInnerView,
                                                     Txut::RandomDihotomicAccelerator,
                                                     thetheme, Orn::Identity);
                m_gameDrawable.centralView.pushFgObj(currentInInnerView,
                                                     Txut::Void,
                                                     thetheme, Orn::Identity);
                break;
            default:
                break;
            }
        }
    }
}


void BlockSnake::updateSnakeDrawable() {
    const sf::Vector2u& mapSize = m_levels.getMapSize(m_difficulty, m_levelIndex);

    sf::IntRect innerZone = getInnerVisibleZone();
    sf::Vector2i leftTopInMap(innerZone.left, innerZone.top);
    sf::Vector2i rightDownInMap =
        leftTopInMap + sf::Vector2i(innerZone.width, innerZone.height) -
        sf::Vector2i(1, 1);

    std::uint64_t harmlessLeastId = m_game.getImpl().getHarmlessLessStepID();
    std::uint64_t stepCount = m_game.getImpl().getSnakeWorld().getStepCount();
    std::uint64_t snakeTailSize = m_game.getImpl().getSnakeWorld().getTailSize();

    std::uint64_t lastHarmfulStep =
        std::max(stepCount - snakeTailSize, harmlessLeastId);

    m_snakeTailEndVisible = false;
    m_snakeTailPreendVisible = false;

    for (int x = leftTopInMap.x; x <= rightDownInMap.x; ++x) {
        for (int y = leftTopInMap.y; y <= rightDownInMap.y; ++y) {
            sf::Vector2i currentInInnerView(x, y);
            currentInInnerView -= leftTopInMap;

            for (const auto& now :
                 m_game.getImpl().getSnakeWorld().getTailIDs(sf::Vector2i(x,y))) {
                std::uint64_t stepId = now.first;

                if (stepId > lastHarmfulStep + 1 &&
                    stepId + 1 !=
                    stepCount) // just the tail without the 2 ends nor the neck
                {
                    m_gameDrawable.centralView.push2snakeDrawable(
                        currentInInnerView, now.second.tdentry,
                        now.second.tdexit,
                        getDestinationIntColor(ColorDst::SnakeBodyFill),
                        getDestinationIntColor(ColorDst::SnakeBodyOutline));
                } else if (stepId == lastHarmfulStep) {
                    m_snakeTailEnd.x = x;
                    m_snakeTailEnd.y = y;
                    m_snakeTailEndVisible = true;
                } else if (stepId == lastHarmfulStep + 1) {
                    m_snakeTailPreend.x = x;
                    m_snakeTailPreend.y = y;
                    m_snakeTailPreendVisible = true;
                }
            }
        }
    }
}


void BlockSnake::scaleUpdate() {
  // Some links
    const std::uint32_t* attribPtr =
        m_levels.getLevelAttribPtr(m_difficulty, m_levelIndex);
    const Game::GameEventProcessor& evProc = m_game.getEventProcessor();
    const GameImpl& gameImpl = m_game.getImpl();
    const SnakeWorld& snakeWorld = gameImpl.getSnakeWorld();

    using Lae = LevelAttribEnum;

    if (!snakeWorld.getBonusPositions().empty()) {
        std::size_t bonuslostev = (std::size_t)(MainGameEvent::BonusExceed);
        sf::Int64 timeev = evProc.getTimeToEvent(bonuslostev);
        float bonusLtNorm = float(timeev) / attribPtr[(int)Lae::BonusLifetime];
        m_gameDrawable.setBonusScale(bonusLtNorm);
    }

    if (!snakeWorld.getPowerups().empty()) {
        std::size_t powlostev = (std::size_t)(MainGameEvent::PowerupExceed);
        sf::Int64 timeev = evProc.getTimeToEvent(powlostev);
        float powerupLtNorm = float(timeev) / attribPtr[(int)Lae::SuperbonusLifetime];
        m_gameDrawable.setPowerupScale(powerupLtNorm);
    }

    if (gameImpl.getEffect() != EffectTypeAl::NoEffect) {
        std::size_t efflostev = (std::size_t)(MainGameEvent::EffectEnded);
        sf::Int64 timeev = evProc.getTimeToEvent(efflostev);
        float effectLtNorm = float(timeev) /
            m_levels.getEffectDurationPtr(m_difficulty, m_levelIndex)[(int)gameImpl.getEffect()];
        m_gameDrawable.setEffectScale(effectLtNorm);
    }

    {
        std::size_t timelostev = (std::size_t)(MainGameEvent::TimeLimitExceed);
        sf::Int64 timeev = evProc.getTimeToEvent(timelostev);
        float timeLimitNorm = float(timeev) / attribPtr[(int)Lae::TimeLimit];
        m_gameDrawable.setTimeLimitScale(timeLimitNorm);
    }
}


void BlockSnake::checkLevelCompleted() {
  // Some links
    const std::uint32_t* plotPtr =
        m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);

    unsigned int whatCount = 0;

    switch ((ChallengeType)plotPtr[(int)LevelPlotDataEnum::Challenge]) {
    case ChallengeType::Bonuses:
        whatCount = m_currBonusEatenCount;
        break;
    case ChallengeType::Fruits:
        whatCount = m_currFruitEatenCount;
        break;
    case ChallengeType::Powerups:
        whatCount = m_currPowerupEatenCount;
        break;
    default:
        break;
    }

    if (whatCount >= plotPtr[(int)LevelPlotDataEnum::ChallengeCount]) {
        if (!m_levelComplete) {
            SoundThrower::Parameters soundParam;
            soundParam.relativeToListener = true;
            soundParam.volume =
                m_settings[(std::size_t)SettingEnum::SoundVolumePer10000] / 100.f;
            m_soundPlayer.playSound(SoundType::Victory, soundParam);

            m_gameDrawable.particles
                .awake(10, 100,
                       sf::Vector2f(),
                       getDestinationIntColor(ColorDst::LevelCompletedParticleFirst),
                       getDestinationIntColor(ColorDst::LevelCompletedParticleSecond),
                       5, 130, sf::microseconds(500000),
                       sf::microseconds(750000), 0.1f, -1000, 1200,
                       1400);
            m_particleNeedUpdatePosition = true;
        }

        m_levelComplete = true;
    }
}


sf::Vector2f
BlockSnake::getPositionOfCircleExit(Direction dir,
                                    const sf::Vector2i& pos) noexcept {
    sf::Vector2f newPos;

    switch (dir) {
    case Direction::Up:
        newPos.x = float(pos.x * TexSz * 2 + TexSz) / 2;
        newPos.y = float(pos.y * TexSz * 4 + TexSz) / 4;
        break;
    case Direction::Down:
        newPos.x = float(pos.x * TexSz * 2 + TexSz) / 2;
        newPos.y = float(pos.y * TexSz * 4 + TexSz * 3) / 4;
        break;
    case Direction::Left:
        newPos.y = float(pos.y * TexSz * 2 + TexSz) / 2;
        newPos.x = float(pos.x * TexSz * 4 + TexSz) / 4;
        break;
    case Direction::Right:
        newPos.y = float(pos.y * TexSz * 2 + TexSz) / 2;
        newPos.x = float(pos.x * TexSz * 4 + TexSz * 3) / 4;
        break;
    default:
        newPos.y = float(pos.y * TexSz * 2 + TexSz) / 2;
        newPos.x = float(pos.x * TexSz * 2 + TexSz) / 2;
        break;
    }

    return newPos;
}


sf::Vector2f
BlockSnake::getPositionOfCircleEntry(Direction dir,
                                     const sf::Vector2i& pos) noexcept {
    sf::Vector2f newPos;

    switch (dir) {
    case Direction::Up:
        newPos.x = float(pos.x * TexSz * 2 + TexSz) / 2;
        newPos.y = float(pos.y * TexSz * 4 + TexSz * 3) / 4;
        break;
    case Direction::Down:
        newPos.x = float(pos.x * TexSz * 2 + TexSz) / 2;
        newPos.y = float(pos.y * TexSz * 4 + TexSz) / 4;
        break;
    case Direction::Left:
        newPos.y = float(pos.y * TexSz * 2 + TexSz) / 2;
        newPos.x = float(pos.x * TexSz * 4 + TexSz * 3) / 4;
        break;
    case Direction::Right:
        newPos.y = float(pos.y * TexSz * 2 + TexSz) / 2;
        newPos.x = float(pos.x * TexSz * 4 + TexSz) / 4;
        break;
    default:
        newPos.y = float(pos.y * TexSz * 2 + TexSz) / 2;
        newPos.x = float(pos.x * TexSz * 2 + TexSz) / 2;
        break;
    }

    return newPos;
}


void BlockSnake::drawWindow() {
    const auto& evProc = m_game.getEventProcessor();
    const auto& gameImpl = m_game.getImpl();
    const auto& mapSize = m_levels.getMapSize(m_difficulty, m_levelIndex);
    const auto* attribPtr = m_levels.getLevelAttribPtr(m_difficulty, m_levelIndex);
    const auto& snakeWorld = gameImpl.getSnakeWorld();
    Direction previousDirection = snakeWorld.getPreviousDirection();
    auto& snakeCrc = m_gameDrawable.snakeCircle;

    float shaderSecs = m_shaderClock.getElapsedTime().asSeconds();
    sf::RenderStates states;

    m_window.clear();
    m_window.draw(m_background, states);

    const sf::Transform& centralBasicTransform = m_gameDrawable.centralTransform;

    // hack
    sf::Vector2f cameraBias =
        //m_game.getImpl().isSnakeMoving() ?
        getCameraBias(m_nowTime) 
        /*:
        getCameraBias(m_lastMoveEventTimePoint)*/
        ;

    sf::Vector2f lastUpdateCameraBias 
        //= getCameraBias(m_lastMoveEventTimePoint)
        ;

    sf::Transform verticalBiasTr = centralBasicTransform;
    verticalBiasTr.translate(0, cameraBias.y);

    sf::Transform horizontalBiasTr = centralBasicTransform;
    horizontalBiasTr.translate(cameraBias.x, 0);

    sf::Transform biasedTr = centralBasicTransform;
    sf::Transform lastUpdBsTr = centralBasicTransform;
    biasedTr.translate(cameraBias);
    lastUpdBsTr.translate(lastUpdateCameraBias);

    states.transform = biasedTr;

    states.texture = m_textures.get();
    m_window.draw(m_gameDrawable.centralView.getvbbackgroundObjects(),
                  0,
                  m_gameDrawable.centralView.getVbvxcountbg(), states);

    using Ve = VisualEffect;
    using Ei = EatableItem;

    {
        sf::Shader& innfrsh =
            m_shaders[static_cast<std::size_t>(Ve::FruitDefault)];
        innfrsh.setUniform("time", shaderSecs);

        states.shader = &innfrsh;
        m_window.draw(m_gameDrawable.centralView.getItemArray(Ei::Fruit), states);
    }

    if (evProc.getTimeToEvent((std::size_t)(MainGameEvent::BonusExceed)) * 5 <
        attribPtr[(int)LevelAttribEnum::BonusLifetime]) {
        sf::Shader& innbnwsh = m_shaders[static_cast<std::size_t>(Ve::BonusWarning)];
        innbnwsh.setUniform("time", shaderSecs);
        states.shader = &innbnwsh;
    } else {
        sf::Shader& innbnsh = m_shaders[static_cast<std::size_t>(Ve::BonusDefault)];
        innbnsh.setUniform("time", shaderSecs);
        states.shader = &innbnsh;
    }

    m_window.draw(m_gameDrawable.centralView.getItemArray(Ei::Bonus),
                  states);

    if (evProc.getTimeToEvent((std::size_t)(MainGameEvent::PowerupExceed)) * 5 <
        attribPtr[(int)LevelAttribEnum::SuperbonusLifetime]) {
        sf::Shader& innpwwsh = m_shaders[static_cast<std::size_t>(Ve::PowerupWarning)];
        innpwwsh.setUniform("time", shaderSecs);
        states.shader = &innpwwsh;
    } else {
        sf::Shader& innpwsh = m_shaders[static_cast<std::size_t>(Ve::PowerupDefault)];
        innpwsh.setUniform("time", shaderSecs);
        states.shader = &innpwsh;
    }

    m_window.draw(m_gameDrawable.centralView.getItemArray(Ei::Powerup), states);

    snakeCrc.setScale(1, 1);

    // draw position pointer

    states.texture = nullptr;
    states.shader = nullptr;

    using Vci = sf::Vector2i;

    const Vci& snakePosition = snakeWorld.getCurrentSnakePosition();
    sf::IntRect innerZone = getInnerVisibleZone();
    Vci leftTopInMap(innerZone.left, innerZone.top);

    Vci rightDownInMap = leftTopInMap +
        Vci(innerZone.width, innerZone.height) -
        Vci(1, 1);

    Vci snakePositionInViewBiased = snakePosition - leftTopInMap + Vci(1, 1);

    sf::Vector2f currentSnakePosPtrPos;
    currentSnakePosPtrPos.x = float(snakePositionInViewBiased.x * TexSz * 2 + TexSz) / 2;
    currentSnakePosPtrPos.y = float(snakePositionInViewBiased.y * TexSz * 2 + TexSz) / 2;

    {
        const Vci& backPosition = m_snakeTailEnd;
        Vci backPositionInViewBiased = backPosition - leftTopInMap + Vci(1, 1);

        if (m_snakeTailEndVisible && innerZone.contains(backPosition)) {

            sf::Vector2f currentBackSnakePosPtrPos;
            currentBackSnakePosPtrPos.x = float(backPositionInViewBiased.x * TexSz * 2 + TexSz) / 2;
            currentBackSnakePosPtrPos.y = float(backPositionInViewBiased.y * TexSz * 2 + TexSz) / 2;

            sf::CircleShape& backPosPtr = m_gameDrawable.snakeEndPositionPointer;
            backPosPtr.setPosition(currentBackSnakePosPtrPos);
            m_window.draw(backPosPtr, states);
        }

        if (m_settings[(std::size_t)SettingEnum::SnakeHeadPointerEnabled]) {
            sf::CircleShape& snakePosPtr =
                m_gameDrawable.snakePositionPointer;
            snakePosPtr.setPosition(currentSnakePosPtrPos);
            m_window.draw(snakePosPtr, states);
        }
    }

    Ve snakeDrawVe;

    if (evProc.getTimeToEvent((std::size_t)(MainGameEvent::TimeLimitExceed)) <= 0)
        snakeDrawVe = Ve::SnakeTimeLimitExceed;
    else if (gameImpl.getEffect() == EffectTypeAl::SlowDown)
        snakeDrawVe = Ve::SnakeSlowDown;
    else if (gameImpl.getEffect() == EffectTypeAl::TailHarmless)
        snakeDrawVe = Ve::SnakeTailHarmless;
    else if (!gameImpl.isSnakeMoving())
        snakeDrawVe = Ve::SnakeStopped;
    else if (gameImpl.getSnakeAcceleration() == Acceleration::Down)
        snakeDrawVe = Ve::SnakeSlow;
    else if (gameImpl.getSnakeAcceleration() == Acceleration::Up)
        snakeDrawVe = Ve::SnakeFast;
    else
        snakeDrawVe = VisualEffect::SnakeDefault;

    sf::Shader& snakeShad = m_shaders[static_cast<std::size_t>(snakeDrawVe)];
    snakeShad.setUniform("time", shaderSecs);
    states.shader = &snakeShad;

    ///////////////////////

    sf::Vector2f currentCirclePos;

    if (previousDirection != Direction::Count) {
      // 2 ends
        Vci mapSizei(mapSize);
        const Vci& backPosition = m_snakeTailEnd;
        const Vci& frontEndPos = m_snakeTailPreend;

        Vci backPositionInViewBiased =
            backPosition - leftTopInMap + Vci(1, 1);
        Vci frontEndInViewBiased =
            frontEndPos - leftTopInMap + Vci(1, 1);

        // head & neck
        Vci neckPosition = snakePosition;
        moveOnModulus(neckPosition, oppositeDirection(previousDirection), mapSizei);

        Vci neckPositionInViewBiased = neckPosition - leftTopInMap + Vci(1, 1);

        sf::Int64 delta = m_nowTime - m_lastMoveEventTimePoint;
        sf::Int64 factualPeriod = gameImpl.getFactualSnakePeriod();
        delta = std::min(delta, factualPeriod);

        float ratio = float(delta) / factualPeriod;
        float firstRatio = std::min(ratio * 2, 1.f);
        float secondRatio = std::max(ratio * 2 - 1.f, 0.f);

        float descendingRatio = 1 - ratio;
        float descendingFirstRatio = 1 - firstRatio;
        float descendingSecondRatio = 1 - secondRatio;

        // TODO: pls fix bug with stopper

        bool tmpMovingReserved = m_game.getImpl().isSnakeMoving();
        if (!m_movingReserved && tmpMovingReserved) {
            m_movingReserved2 = true;
        }

        if (delta >= factualPeriod 
            && 
            (previousDirection == Direction::Down ||
            previousDirection == Direction::Right) && m_game.getImpl().isSnakeMoving() && !m_movingReserved2
            ) {
            states.transform = lastUpdBsTr;
        }

        if (snakeWorld.getTailSize() == 0) {
            currentCirclePos.x = float(neckPositionInViewBiased.x * TexSz * 2 + TexSz) / 2;
            currentCirclePos.y = float(neckPositionInViewBiased.y * TexSz * 2 + TexSz) / 2;

            snakeCrc.setPosition(currentCirclePos);
            snakeCrc.setScale(descendingRatio, descendingRatio);
            m_window.draw(snakeCrc, states);

            currentCirclePos.x = float(snakePositionInViewBiased.x * TexSz * 2 + TexSz) / 2;
            currentCirclePos.y = float(snakePositionInViewBiased.y * TexSz * 2 + TexSz) / 2;

            snakeCrc.setPosition(currentCirclePos);
            snakeCrc.setScale(ratio, ratio);
            m_window.draw(snakeCrc, states);

        } else {
            if (m_snakeTailEndVisible && innerZone.contains(backPosition) &&
                !snakeWorld.getTailIDs(backPosition).empty()) {
                Direction theSecondEndDir = snakeWorld.getTailIDs(backPosition).begin()->second.tdexit;

                currentCirclePos = getPositionOfCircleExit(theSecondEndDir,
                                                           backPositionInViewBiased);

                snakeCrc.setPosition(currentCirclePos);
                snakeCrc.setScale(descendingFirstRatio, descendingFirstRatio);
                m_window.draw(snakeCrc, states);
            }

            if (m_snakeTailPreendVisible && innerZone.contains(frontEndPos) &&
                !snakeWorld.getTailIDs(frontEndPos).empty()) {
                const auto& taildir = snakeWorld.getTailIDs(frontEndPos).begin()->second;

                currentCirclePos = getPositionOfCircleEntry(taildir.tdentry,
                                                            frontEndInViewBiased);

                snakeCrc.setPosition(currentCirclePos);
                snakeCrc.setScale(descendingSecondRatio,
                                  descendingSecondRatio);
                m_window.draw(snakeCrc, states);

                currentCirclePos = getPositionOfCircleExit(taildir.tdexit,
                                                           frontEndInViewBiased);

                snakeCrc.setPosition(currentCirclePos);
                snakeCrc.setScale(1, 1);
                m_window.draw(snakeCrc, states);
            }

            states.transform = biasedTr;

            // tail
            m_window.draw(m_gameDrawable.centralView.getSnakeDrawable(), states);

            if (delta >= factualPeriod && (previousDirection == Direction::Down ||
                previousDirection == Direction::Right) && m_game.getImpl().isSnakeMoving() && !m_movingReserved2
                ) {
                states.transform = lastUpdBsTr;
            }

            // HACK
            m_movingReserved = tmpMovingReserved;

            if (innerZone.contains(neckPosition) && !snakeWorld.getTailIDs(neckPosition).empty()) {
                Direction neckEntryDir = snakeWorld.getTailIDs(neckPosition).begin()->second.tdentry;

                currentCirclePos = getPositionOfCircleEntry(neckEntryDir,
                                                            neckPositionInViewBiased);

                snakeCrc.setScale(1, 1);
                snakeCrc.setPosition(currentCirclePos);
                m_window.draw(snakeCrc, states);

                currentCirclePos = getPositionOfCircleExit(previousDirection,
                                                           neckPositionInViewBiased);

                snakeCrc.setPosition(currentCirclePos);
                snakeCrc.setScale(firstRatio, firstRatio);
                m_window.draw(snakeCrc, states);
            }

            if (innerZone.contains(snakePosition) && snakeWorld.getTailSize() != 0) {
                currentCirclePos = getPositionOfCircleEntry(previousDirection,
                                                            snakePositionInViewBiased);

                snakeCrc.setPosition(currentCirclePos);
                snakeCrc.setScale(secondRatio, secondRatio);
                m_window.draw(snakeCrc, states);
            }
        }

    } else if (innerZone.contains(snakePosition)) {
        currentCirclePos.x = float(snakePositionInViewBiased.x * TexSz * 2 + TexSz) / 2;
        currentCirclePos.y = float(snakePositionInViewBiased.y * TexSz * 2 + TexSz) / 2;

        snakeCrc.setPosition(currentCirclePos);
        snakeCrc.setScale(1, 1);
        m_window.draw(snakeCrc, states);
    }

    states.texture = m_textures.get();
    states.shader = nullptr;
    states.transform = biasedTr;
    m_window.draw(m_gameDrawable.centralView.getvbforegroundObjects(),
                  0, m_gameDrawable.centralView.getVbvxcountfg(), states);

    states.transform = centralBasicTransform;
    drawScreens(states, shaderSecs);

    {
        sf::Shader& scrfrsh =
            m_shaders[static_cast<std::size_t>(Ve::FruitScreen)];
        scrfrsh.setUniform("time", shaderSecs);

        states.shader = &scrfrsh;

        states.transform = centralBasicTransform;
        m_window.draw(m_gameDrawable.centralView.getScreenItemArray(Ei::Fruit,
                      ScreenMode::Corner), states);
        states.transform = verticalBiasTr;
        m_window.draw(m_gameDrawable.centralView.getScreenItemArray(Ei::Fruit,
                      ScreenMode::Vertical), states);
        states.transform = horizontalBiasTr;
        m_window.draw(m_gameDrawable.centralView.getScreenItemArray(Ei::Fruit,
                      ScreenMode::Horizontal), states);
    }

    if (evProc.getTimeToEvent((std::size_t)(MainGameEvent::BonusExceed)) * 5 <
        attribPtr[(int)LevelAttribEnum::BonusLifetime]) {
        sf::Shader& scrbnwsh =
            m_shaders[static_cast<std::size_t>(Ve::BonusScreenWarning)];
        scrbnwsh.setUniform("time", shaderSecs);
        states.shader = &scrbnwsh;
    } else {
        sf::Shader& scrbnsh =
            m_shaders[static_cast<std::size_t>(Ve::BonusScreen)];
        scrbnsh.setUniform("time", shaderSecs);
        states.shader = &scrbnsh;
    }

    states.transform = centralBasicTransform;
    m_window.draw(m_gameDrawable.centralView.getScreenItemArray(Ei::Bonus,
                  ScreenMode::Corner), states);
    states.transform = verticalBiasTr;
    m_window.draw(m_gameDrawable.centralView.getScreenItemArray(Ei::Bonus,
                  ScreenMode::Vertical), states);
    states.transform = horizontalBiasTr;
    m_window.draw(m_gameDrawable.centralView.getScreenItemArray(Ei::Bonus,
                  ScreenMode::Horizontal), states);

    if (evProc.getTimeToEvent((std::size_t)(MainGameEvent::PowerupExceed)) * 5 <
        attribPtr[(int)LevelAttribEnum::SuperbonusLifetime]) {
        sf::Shader& scrpwwsh =
            m_shaders[static_cast<std::size_t>(Ve::PowerupScreenWarning)];
        scrpwwsh.setUniform("time", shaderSecs);
        states.shader = &scrpwwsh;
    } else {
        sf::Shader& scrpwsh =
            m_shaders[static_cast<std::size_t>(Ve::PowerupScreen)];
        scrpwsh.setUniform("time", shaderSecs);
        states.shader = &scrpwsh;
    }

    states.transform = centralBasicTransform;
    m_window.draw(m_gameDrawable.centralView.getScreenItemArray(Ei::Powerup,
                  ScreenMode::Corner), states);
    states.transform = verticalBiasTr;
    m_window.draw(m_gameDrawable.centralView.getScreenItemArray(Ei::Powerup,
                  ScreenMode::Vertical), states);
    states.transform = horizontalBiasTr;
    m_window.draw(m_gameDrawable.centralView.getScreenItemArray(Ei::Powerup,
                  ScreenMode::Horizontal), states);

    states.transform = centralBasicTransform;
    states.texture = nullptr;
    states.shader = nullptr;

    using Lpde = LevelPlotDataEnum;

    states.blendMode.alphaDstFactor = (sf::BlendMode::Factor)m_levels
        .getLevelPlotDataPtr(m_difficulty,
                             m_levelIndex)[(std::size_t)Lpde::FoggBlendDstAlpha];
    states.blendMode.alphaSrcFactor = (sf::BlendMode::Factor)m_levels
        .getLevelPlotDataPtr(m_difficulty,
                             m_levelIndex)[(std::size_t)Lpde::FoggBlendSrcAlpha];
    states.blendMode.alphaEquation = (sf::BlendMode::Equation)m_levels
        .getLevelPlotDataPtr(m_difficulty,
                             m_levelIndex)[(std::size_t)Lpde::FoggBlendAlphaEq];
    states.blendMode.colorDstFactor = (sf::BlendMode::Factor)m_levels
        .getLevelPlotDataPtr(m_difficulty,
                             m_levelIndex)[(std::size_t)Lpde::FoggBlendDstColor];
    states.blendMode.colorSrcFactor = (sf::BlendMode::Factor)m_levels
        .getLevelPlotDataPtr(m_difficulty,
                             m_levelIndex)[(std::size_t)Lpde::FoggBlendSrcColor];
    states.blendMode.colorEquation = (sf::BlendMode::Equation)m_levels
        .getLevelPlotDataPtr(m_difficulty,
                             m_levelIndex)[(std::size_t)Lpde::FoggBlendColorEq];

    m_window.draw(m_gameDrawable.centralView.getFogg(), states);

    states.blendMode = sf::BlendAlpha;
    states.transform = sf::Transform::Identity;

    drawScales();
    drawChallVis(shaderSecs);

    states.transform = biasedTr;
    states.texture = nullptr;

    {
        if (m_particleNeedUpdatePosition) {
            m_gameDrawable.particles.setPosition(currentSnakePosPtrPos);
            m_particleSystemTransform = states.transform;
            m_particleNeedUpdatePosition = false;
        }
        m_gameDrawable.particles.update(m_particleClock.restart());

        sf::RenderStates particleRS{ states };
        particleRS.transform = m_particleSystemTransform;
        m_window.draw(m_gameDrawable.particles, particleRS);
    }

    m_window.display();
}


sf::Vector2f BlockSnake::getCameraBias(sf::Int64 now) const {

    // snake delaying
    sf::Int64 delta = now - m_lastMoveEventTimePoint;
    sf::Int64 factualSnakePeriod = m_game.getImpl().getFactualSnakePeriod();

    if (!m_game.getImpl().isSnakeMoving()
        && !isCameraStopped(now)) {
        
        if (delta >= factualSnakePeriod) {
            switch (m_game.getImpl().getSnakeWorld().getPreviousDirection()) {
            case Direction::Up:
                return sf::Vector2f(0, 0*-(float)TexSz);
            case Direction::Down:
                return sf::Vector2f(0, -(float)TexSz);
            case Direction::Left:
                return sf::Vector2f(0 * -(float)TexSz, 0);
            case Direction::Right:
                return sf::Vector2f(-(float)TexSz, 0);
            default:
                break;
            }
            return sf::Vector2f();
        } else {
            float bias = float((factualSnakePeriod - delta) * TexSz) / factualSnakePeriod - TexSz;
            switch (m_game.getImpl().getSnakeWorld().getPreviousDirection()) {
            case Direction::Up:
                return sf::Vector2f(0, -bias - TexSz);
            case Direction::Down:
                return sf::Vector2f(0, bias);
            case Direction::Left:
                return sf::Vector2f(-bias - TexSz, 0);
            case Direction::Right:
                return sf::Vector2f(bias, 0);
            default:
                break;
            }
            return sf::Vector2f();
        }
    }
    
    // some info
    const std::uint32_t* plotPtr = m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);
    sf::Vector2i mapSize{ m_levels.getMapSize(m_difficulty, m_levelIndex) };
    const sf::Vector2i& snakePosition = m_game.getImpl().getSnakeWorld().getCurrentSnakePosition();

    if (isCameraStopped(now)) {
        if (delta >= factualSnakePeriod) 
        {
            switch (m_game.getImpl().getSnakeWorld().getPreviousDirection()) {
            case Direction::Down: {
                bool cond = (snakePosition.y < (int)plotPtr[(int)LevelPlotDataEnum::SnakeSightY] + 1) ||
                    (snakePosition.y >= mapSize.y - (int)plotPtr[(int)LevelPlotDataEnum::SnakeSightY]);
                return (
                    cond ? 
                        sf::Vector2f() 
                        : -sf::Vector2f(0, TexSz)
                        );
            }
            case Direction::Right: {
                bool cond = (snakePosition.x < (int)plotPtr[(int)LevelPlotDataEnum::SnakeSightX] + 1) ||
                    (snakePosition.x >= mapSize.x - (int)plotPtr[(int)LevelPlotDataEnum::SnakeSightX]);
                return (
                    cond ? 
                        sf::Vector2f() 
                    : -sf::Vector2f(TexSz, 0)
                    );
            }
            default:
                break;
            }
        }
        return sf::Vector2f();
    }

    // (...) camera not stopped

    delta = std::min(factualSnakePeriod, delta);

    float bias = float((factualSnakePeriod - delta) * TexSz) / factualSnakePeriod - TexSz;
    bool moving = 
        //true;
        false;
        //m_game.getImpl().isSnakeMoving();

    switch (m_game.getImpl().getSnakeWorld().getPreviousDirection()) {
    case Direction::Up:
        return sf::Vector2f(0, -bias - TexSz);
    case Direction::Down:
        return !moving ? sf::Vector2f(0, bias) : sf::Vector2f();
    case Direction::Left:
        return sf::Vector2f(-bias - TexSz, 0);
    case Direction::Right:
        return !moving ? sf::Vector2f(bias, 0) : sf::Vector2f();
    default:
        break;
    }

    assert(false);
    return sf::Vector2f();
}


void BlockSnake::updateItems(EatableItem item) {
    const std::uint32_t* plotPtr = m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);

    const GameImpl& gameImpl = m_game.getImpl();
    const SnakeWorld& snakeWorld = gameImpl.getSnakeWorld();

    sf::Vector2i snakeFullViewSize;
    snakeFullViewSize.x = plotPtr[(int)LevelPlotDataEnum::SnakeSightX] * 2 + 1;
    snakeFullViewSize.y = plotPtr[(int)LevelPlotDataEnum::SnakeSightY] * 2 + 1;

    bool cameraStopped = isCameraStopped(m_nowTime);
    sf::IntRect innerZone = getInnerVisibleZone();

    Direction tailing = Direction::Count;
    if (!cameraStopped)    tailing = snakeWorld.getPreviousDirection();

    sf::Vector2i leftTopInMap(innerZone.left, innerZone.top);
    sf::Vector2i snakeRelativeLeftTop = leftTopInMap;

    if (!cameraStopped) {
        switch (tailing) {
        case Direction::Right:
            ++snakeRelativeLeftTop.x;
            break;
        case Direction::Down:
            ++snakeRelativeLeftTop.y;
            break;
        default:
            break;
        }
    }

    sf::Vector2i rightDownInMap = leftTopInMap +
        sf::Vector2i(innerZone.width, innerZone.height) -
        sf::Vector2i(1, 1);

    sf::Vector2i innerZoneSize(innerZone.width, innerZone.height);

    std::vector<int> existingScreenItems(((std::size_t)innerZoneSize.x + innerZoneSize.y) * 2 + 4);

    auto roundLambda = [&innerZoneSize](const sf::Vector2i& pos) {
        if (pos.y == -1 && pos.x >= -1 && pos.x <= innerZoneSize.x)
            return pos.x + 1;
        else if (pos.x == innerZoneSize.x && pos.y > -1 && pos.y <= innerZoneSize.y)
            return innerZoneSize.x + 2 + pos.y;
        else if (pos.y == innerZoneSize.y && pos.x >= -1 && pos.x < innerZoneSize.x)
            return innerZoneSize.x * 2 + innerZoneSize.y + 2 - pos.x;

          // else if (pos.x == -1 && pos.y >= -1 && pos.y <= snakeFullViewSize.y)
        return (innerZoneSize.x + innerZoneSize.y) * 2 + 3 - pos.y;
    };

    if (item == EatableItem::Fruit || item == EatableItem::Bonus) {
        const SnakeWorld::ItemSet& posset = ((item == EatableItem::Fruit) ?
                                             snakeWorld.getFruitPositions() :
                                             snakeWorld.getBonusPositions());

        for (const sf::Vector2i& now : posset) {
          // to view!!!
            sf::Vector2i newnow = now;
            newnow -= snakeRelativeLeftTop;
            sf::Vector2i newnowInner = now;
            newnowInner -= leftTopInMap;

            int screenDistanceSignedXLeft = -newnow.x;
            int screenDistanceSignedXRight = newnow.x - snakeFullViewSize.x + 1;

            int screenDistanceSignedYTop = -newnow.y;
            int screenDistanceSignedYBottom = newnow.y - snakeFullViewSize.y + 1;

            unsigned int projDist = ((item == EatableItem::Fruit) ?
                                     plotPtr[(int)LevelPlotDataEnum::FruitScreenProjectionDistance] :
                                     plotPtr[(int)LevelPlotDataEnum::BonusScreenProjectionDistance]);
            int screenDistanceMaxSigned = projDist;
            bool visible = (screenDistanceSignedXLeft <= screenDistanceMaxSigned &&
                            screenDistanceSignedXRight <= screenDistanceMaxSigned &&
                            screenDistanceSignedYTop <= screenDistanceMaxSigned &&
                            screenDistanceSignedYBottom <= screenDistanceMaxSigned);
            bool screen = false;

            // bounds!
            if (newnowInner.x < -1) {
                screen = true;
                newnowInner.x = -1;
            } else if (newnowInner.x > innerZoneSize.x) {
                screen = true;
                newnowInner.x = innerZoneSize.x;
            }

            if (newnowInner.y < -1) {
                screen = true;
                newnowInner.y = -1;
            } else if (newnowInner.y > innerZoneSize.y) {
                screen = true;
                newnowInner.y = innerZoneSize.y;
            }

            bool screenAndExisting = false;

            if (screen)
                screenAndExisting = static_cast<bool>(existingScreenItems[roundLambda(newnowInner)]);

            if (visible && !screenAndExisting) {
                existingScreenItems[roundLambda(newnowInner)] = 1;

                if (item == EatableItem::Fruit)
                    m_gameDrawable.centralView.pushFruit(newnowInner, tailing, innerZoneSize);
                else
                    m_gameDrawable.centralView.pushBonus(newnowInner, tailing, innerZoneSize);
            }
        }
    } else {
        for (const auto& nowp : snakeWorld.getPowerups()) {
            const sf::Vector2i& now = nowp.first;

            // to view!!!
            sf::Vector2i newnow = now;
            newnow -= snakeRelativeLeftTop;
            sf::Vector2i newnowInner = now;
            newnowInner -= leftTopInMap;

            int screenDistanceSignedXLeft = -newnow.x;
            int screenDistanceSignedXRight = newnow.x - snakeFullViewSize.x + 1;

            int screenDistanceSignedYTop = -newnow.y;
            int screenDistanceSignedYBottom = newnow.y - snakeFullViewSize.y + 1;

            unsigned int projDist = plotPtr[(int)LevelPlotDataEnum::SuperbonusScreenProjectionDistance];
            int screenDistanceMaxSigned = projDist;
            bool visible = (screenDistanceSignedXLeft <= screenDistanceMaxSigned &&
                            screenDistanceSignedXRight <= screenDistanceMaxSigned &&
                            screenDistanceSignedYTop <= screenDistanceMaxSigned &&
                            screenDistanceSignedYBottom <= screenDistanceMaxSigned);
            bool screen = false;

            // bounds!
            if (newnowInner.x < -1) {
                screen = true;
                newnowInner.x = -1;
            } else if (newnowInner.x > innerZoneSize.x) {
                screen = true;
                newnowInner.x = innerZoneSize.x;
            }

            if (newnowInner.y < -1) {
                screen = true;
                newnowInner.y = -1;
            } else if (newnowInner.y > innerZoneSize.y) {
                screen = true;
                newnowInner.y = innerZoneSize.y;
            }

            bool screenAndExisting = false;

            if (screen)
                screenAndExisting = static_cast<bool>(existingScreenItems[roundLambda(newnowInner)]);

            if (visible && !screenAndExisting) {
                existingScreenItems[roundLambda(newnowInner)] = 1;

                if (plotPtr[(int)LevelPlotDataEnum::SuperbonusVisible])
                    m_gameDrawable.centralView.pushPowerup(nowp.second, newnowInner, tailing, innerZoneSize);
                else
                    m_gameDrawable.centralView.pushUnknownPowerup(newnowInner, tailing, innerZoneSize);
            }
        }
    }
}


void BlockSnake::drawScreens(sf::RenderStates states, float shaderSecs) {
    const Game::GameEventProcessor evProc = m_game.getEventProcessor();
    const std::uint32_t* attribPtr =
        m_levels.getLevelAttribPtr(m_difficulty, m_levelIndex);

    VisualEffect screenve;

    if (evProc.getTimeToEvent((std::size_t)(MainGameEvent::TimeLimitExceed)) <= 0)
        screenve = VisualEffect::ScreenTimeLimitExceed;
    else if (evProc.getTimeToEvent((std::size_t)(MainGameEvent::TimeLimitExceed)) * 5 <
             attribPtr[(int)LevelAttribEnum::TimeLimit])
        screenve = VisualEffect::ScreenTimeLimitWarning;
    else
        screenve = VisualEffect::ScreenDefault;

    sf::Shader& screenshad = m_shaders[static_cast<std::size_t>(screenve)];
    screenshad.setUniform("time", shaderSecs);
    states.shader = &screenshad;
    states.texture = m_textures.get();
    m_window.draw(m_gameDrawable.centralView.getVbScreens(), states);
}


void BlockSnake::drawScales() {
    const std::uint32_t* plotPtr =
        m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);
    const SnakeWorld& snakeWorld = m_game.getImpl().getSnakeWorld();

    if (plotPtr[(int)LevelPlotDataEnum::BonusScaleVisible] && !snakeWorld.getBonusPositions().empty())
        m_window.draw(m_gameDrawable.bonusScale);
    if (plotPtr[(int)LevelPlotDataEnum::SuperbonusScaleVisible] && !snakeWorld.getPowerups().empty())
        m_window.draw(m_gameDrawable.powerupScale);
    if (plotPtr[(int)LevelPlotDataEnum::EffectScaleVisible] && m_game.getImpl().getEffect() != EffectTypeAl::NoEffect)
        m_window.draw(m_gameDrawable.effectScale);
    if (plotPtr[(int)LevelPlotDataEnum::TimeLimitScaleVisible])
        m_window.draw(m_gameDrawable.timeLimitScale);
}


void BlockSnake::drawChallVis(float shaderSecs) {
    const std::uint32_t* plotPtr =
        m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);
    const std::uint32_t* attribPtr =
        m_levels.getLevelAttribPtr(m_difficulty, m_levelIndex);

    std::uint32_t fruitCountToBonus =
        attribPtr[(int)LevelAttribEnum::FruitCountToBonus];

    std::uint32_t bonusCountToPowerup =
        attribPtr[(int)LevelAttribEnum::BonusCountToSuperbonus];

    if (plotPtr[(int)LevelPlotDataEnum::FruitCountToBonusVisible]) {
        if (m_fruit2bonusVisualCount < (std::size_t)(fruitCountToBonus -
            m_game.getImpl().getFruitCountToBonus()) * 100 / fruitCountToBonus)
            m_fruit2bonusVisualCount = (std::size_t)
            std::min(((std::uintmax_t)m_fruit2bonusVisualCount * 1 +
                     (std::uintmax_t)std::min(m_fruit2bonusVisualClock.restart()
                     .asMicroseconds(),
                     (sf::Int64)1)) / 1,
                     (std::uintmax_t)(fruitCountToBonus -
                     m_game.getImpl().getFruitCountToBonus()) * 100 /
                     fruitCountToBonus);
        else if (m_fruit2bonusVisualCount > (std::size_t)(fruitCountToBonus -
                 m_game.getImpl().getFruitCountToBonus()) * 100 / fruitCountToBonus)
            m_fruit2bonusVisualCount = (std::size_t)
            std::max(((std::intmax_t)m_fruit2bonusVisualCount * 1 -
                     (std::intmax_t)std::min(m_fruit2bonusVisualClock.restart()
                     .asMicroseconds(),
                     (sf::Int64)10)) / 1,
                     (std::intmax_t)(fruitCountToBonus -
                     m_game.getImpl().getFruitCountToBonus()) * 100 /
                     fruitCountToBonus);

        m_gameDrawable.fruitCountToBonusVisual.setVisibleCount(std::min(m_fruit2bonusVisualCount,
                                                               (std::size_t)100));

        m_window.draw(m_gameDrawable.fruitCountToBonusVisual);
        m_window.draw(m_gameDrawable.fruitCountToBonusVisualOutline);
    }

    if (plotPtr[(int)LevelPlotDataEnum::BonusCountToSuperbonusVisible]) {
        if (m_bonus2superbonusVisualCount < (std::size_t)(bonusCountToPowerup -
            m_game.getImpl().getBonusCountToPowerup()) * 100 / bonusCountToPowerup)
            m_bonus2superbonusVisualCount = (std::size_t)
            std::min(((std::uintmax_t)m_bonus2superbonusVisualCount * 1 +
                     (std::uintmax_t)std::min(m_bonus2superbonusClock.restart()
                     .asMicroseconds(),
                     (sf::Int64)1)) / 1,
                     (std::uintmax_t)(bonusCountToPowerup -
                     m_game.getImpl().getBonusCountToPowerup()) * 100 /
                     bonusCountToPowerup);
        else if (m_bonus2superbonusVisualCount > (std::size_t)(bonusCountToPowerup -
                 m_game.getImpl().getBonusCountToPowerup()) * 100 / bonusCountToPowerup)
            m_bonus2superbonusVisualCount = (std::size_t)
            std::max(((std::intmax_t)m_bonus2superbonusVisualCount * 1 -
                     (std::intmax_t)std::min(m_bonus2superbonusClock.restart()
                     .asMicroseconds(),
                     (sf::Int64)10)) / 1,
                     (std::intmax_t)(bonusCountToPowerup -
                     m_game.getImpl().getBonusCountToPowerup()) * 100 /
                     bonusCountToPowerup);

        m_gameDrawable.bonusCountToPowerupVisual.setVisibleCount(std::min(m_bonus2superbonusVisualCount,
                                                                 (std::size_t)100));

        m_window.draw(m_gameDrawable.bonusCountToPowerupVisual);
        m_window.draw(m_gameDrawable.bonusCountToPowerupVisualOutline);
    }

    std::size_t cnt = 0;

    switch ((ChallengeType)plotPtr[(int)LevelPlotDataEnum::Challenge]) {
    case ChallengeType::Bonuses:
        cnt = m_currBonusEatenCount;
        break;
    case ChallengeType::Fruits:
        cnt = m_currFruitEatenCount;
        break;
    case ChallengeType::Powerups:
        cnt = m_currPowerupEatenCount;
        break;
    default:
        break;
    }

    if (m_challengeVisualCount < (std::size_t)cnt * 100 /
        plotPtr[(int)LevelPlotDataEnum::ChallengeCount])
        m_challengeVisualCount = (std::size_t)
        std::min(((std::uintmax_t)m_challengeVisualCount * 1 +
                 (std::uintmax_t)std::min(m_challengeVisualClock.restart()
                 .asMicroseconds(),
                 (sf::Int64)1)) / 1,
                 (std::uintmax_t)cnt * 10000 /
                 plotPtr[(int)LevelPlotDataEnum::ChallengeCount]);
    else if (m_challengeVisualCount > (std::size_t)cnt * 100 /
             plotPtr[(int)LevelPlotDataEnum::ChallengeCount])
        m_challengeVisualCount = (std::size_t)
        std::max(((std::intmax_t)m_challengeVisualCount * 1 -
                 (std::intmax_t)std::min(m_challengeVisualClock.restart()
                 .asMicroseconds(),
                 (sf::Int64)10)) / 1,
                 (std::intmax_t)cnt * 10000 /
                 plotPtr[(int)LevelPlotDataEnum::ChallengeCount]);

    m_gameDrawable.challengeVisual.setVisibleCount(std::min(m_challengeVisualCount, (std::size_t)100));

    if (m_levelComplete) {
        sf::Shader& complsh = m_shaders[static_cast<std::size_t>(VisualEffect::ChallengeVisualComplete)];

        complsh.setUniform("time", shaderSecs);
        m_window.draw(m_gameDrawable.challengeVisual, &complsh);
        m_window.draw(m_gameDrawable.challengeVisualOutline, &complsh);
    } else {
        sf::Shader& defchvissh = m_shaders[static_cast<std::size_t>(VisualEffect::ChallengeVisualDefault)];

        defchvissh.setUniform("time", shaderSecs);
        m_window.draw(m_gameDrawable.challengeVisual, &defchvissh);
        m_window.draw(m_gameDrawable.challengeVisualOutline, &defchvissh);
    }

    if (m_visualScore < m_currScore)
        m_visualScore = (std::size_t)
        std::min(((std::uintmax_t)m_visualScore * 10 +
                 (std::uintmax_t)std::min(m_scoreVisualClock.restart().asMicroseconds(),
                 (sf::Int64)100)) / 10, m_currScore);
    else if (m_visualScore > m_currScore)
        m_visualScore = (std::size_t)
        std::max(((std::intmax_t)m_visualScore * 10 -
                 (std::intmax_t)std::min(m_scoreVisualClock.restart().asMicroseconds(),
                 (sf::Int64)1000)) / 10, (std::intmax_t)m_currScore);

    m_gameDrawable.digits.setNumber(m_visualScore);
    m_window.draw(m_gameDrawable.digits);

    if (m_levelStatistics.getLevelHighestScore(m_levelIndex) >= m_currScore)
        m_window.draw(m_gameDrawable.highestScore);
}


void BlockSnake::processEvents() {
    sf::Event event;
    sf::Vector2u oldSize = m_window.getSize();
    while (m_window.pollEvent(event)) {
        switch (event.type) {
        case sf::Event::Closed:
            m_gameClock.pause();
            m_toReturn = false;
            m_toExit = true;
            break;
        case sf::Event::KeyPressed:
            if (event.key.code == sf::Keyboard::Enter ||
                event.key.scancode == sf::Keyboard::Scancode::G) {
                m_gameClock.pause();
                m_toReturn = true;
                m_toExit = true;
            } else if (event.key.code == sf::Keyboard::Escape || event.key.scancode == sf::Keyboard::Scancode::R) {
                pauseGame();
            } else if (event.key.scancode == sf::Keyboard::Scancode::W ||
                        event.key.code == sf::Keyboard::Up ||
                       event.key.scancode == sf::Keyboard::Scancode::Numpad8) {
                m_game.pushCommand(m_nowTime, Direction::Up);
                m_rotatedPostEffect = false;
            } else if (event.key.scancode == sf::Keyboard::Scancode::A ||
                        event.key.code == sf::Keyboard::Left ||
                       event.key.scancode == sf::Keyboard::Scancode::Numpad4) {
                m_game.pushCommand(m_nowTime, Direction::Left);
                m_rotatedPostEffect = false;
            } else if (event.key.scancode == sf::Keyboard::Scancode::S ||
                        event.key.code == sf::Keyboard::Down ||
                       event.key.scancode == sf::Keyboard::Scancode::Numpad5 ||
                       event.key.scancode == sf::Keyboard::Scancode::Numpad2) {
                m_game.pushCommand(m_nowTime, Direction::Down);
                m_rotatedPostEffect = false;
            } else if (event.key.scancode == sf::Keyboard::Scancode::D ||
                        event.key.code == sf::Keyboard::Right ||
                       event.key.scancode == sf::Keyboard::Scancode::Numpad6) {
                m_game.pushCommand(m_nowTime, Direction::Right);
                m_rotatedPostEffect = false;
            } else if (event.key.code == sf::Keyboard::P) {
                m_settings[(std::size_t)SettingEnum::SnakeHeadPointerEnabled] =
                    (std::uint32_t)!static_cast<bool>(
                    m_settings[(std::size_t)SettingEnum::SnakeHeadPointerEnabled]);
            }
            break;
        case sf::Event::LostFocus:
            pauseGame();
            break;
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
        default:
            break;
        }
    }
}


void BlockSnake::processGameEvents() {

    auto dic = [this](ColorDst dst) {return getDestinationIntColor(dst); };

    Game::Event gameEvent;
    bool anyGameEvent = false;
    while (m_game.pollEvent(gameEvent)) {
        anyGameEvent = true;

        SoundThrower::Parameters soundParam;
        soundParam.volume = m_settings[(std::size_t)SettingEnum::SoundVolumePer10000] / 100.f;
        soundParam.relativeToListener = true;

        float rand0_1 = float(std::rand()) / RAND_MAX;
        rand0_1 -= 0.5f;
        soundParam.pitch = std::exp(rand0_1 / 15.f);

        bool rotatedPostEffectOccured = false;

        if (gameEvent.isMain) {
            switch (gameEvent.mainGameEvent) {
            case MainGameEvent::BonusExceed:
                soundParam.relativeToListener = false;
                soundParam.position = sf::Vector3f((float)gameEvent.bonusLostEvent.x,
                                                   (float)gameEvent.bonusLostEvent.y, 0);
                m_soundPlayer.playSound(SoundType::BonusDisappear, soundParam);
                break;
            case MainGameEvent::EffectEnded:
                m_soundPlayer.playSound(SoundType::EffectEnded, soundParam);

                m_gameDrawable.particles
                    .awake(9, 40, sf::Vector2f(),
                           dic(ColorDst::EffectEndedParticleFirst),
                           dic(ColorDst::EffectEndedParticleSecond), 30, 80,
                           sf::microseconds(200000), sf::microseconds(400000), 0.2f, -300, 300, 400);
                m_particleNeedUpdatePosition = true;

                break;
            case MainGameEvent::Moved:
                if (m_rotatedPostEffect)
                    m_soundPlayer.playSound(SoundType::ForcedRotating, soundParam);

                sf::Listener::setPosition((float)m_game.getImpl().getSnakeWorld().getCurrentSnakePosition().x,
                                          (float)m_game.getImpl().getSnakeWorld().getCurrentSnakePosition().y, 0);

                m_rotatedPostEffect = false;
                m_currStepCount++;
                m_lastMoveEventTimePoint = gameEvent.time;

                // HACK
                m_movingReserved2 = false;

                // spikes...
                if (!gameEvent.unpredMemory &&
                    m_game.getImpl().getObjectMemory(m_game.getImpl().getSnakeWorld().getCurrentSnakePosition().x,
                    m_game.getImpl().getSnakeWorld().getCurrentSnakePosition().y)) {
                    m_soundPlayer.playSound(SoundType::ActivateSpikes, soundParam);

                    m_gameDrawable.particles
                        .awake(12, 10, sf::Vector2f(),
                               dic(ColorDst::SpikesParticleFirst),
                               dic(ColorDst::SpikesParticleSecond),
                               5, 80, sf::microseconds(100000),
                               sf::microseconds(150000), 0.05f, -3000, 200, 600);
                    m_particleNeedUpdatePosition = true;
                }

                break;
            case MainGameEvent::PowerupExceed:
                soundParam.relativeToListener = false;
                soundParam.position = sf::Vector3f((float)gameEvent.powerupLostEvent.x,
                                                   (float)gameEvent.powerupLostEvent.y, 0);
                m_soundPlayer.playSound(SoundType::PowerupDisappear, soundParam);
                break;
            case MainGameEvent::TimeLimitExceed:
                m_gameClock.pause();
                m_soundPlayer.playSound(SoundType::TimeLimitExceedSignal, soundParam);
                m_gameDrawable.particles
                    .awake(9, 20, sf::Vector2f(),
                           dic(ColorDst::TimeLimitExceedParticleFirst),
                           dic(ColorDst::TimeLimitExceedParticleSecond),
                           30, 80, sf::microseconds(200000),
                           sf::microseconds(400000), 0.1f, -300, 300, 400);
                m_particleNeedUpdatePosition = true;

                break;
            default:
                break;
            }
        } else {
            switch (gameEvent.subevent) {
            case GameSubevent::Accelerated:
                switch (m_game.getImpl().getSnakeAcceleration()) {
                case Acceleration::Default:
                    m_soundPlayer.playSound(SoundType::AccelerateDefault, soundParam);

                    m_gameDrawable.particles
                        .awake(7, 40, sf::Vector2f(),
                               dic(ColorDst::AcceleratedDefaultParticleFirst),
                               dic(ColorDst::AcceleratedDefaultParticleSecond),
                               40, 90, sf::microseconds(200000),
                               sf::microseconds(250000), 0.1f, -1000, 300, 450);
                    m_particleNeedUpdatePosition = true;

                    break;
                case Acceleration::Down:
                    m_soundPlayer.playSound(SoundType::AccelerateDown, soundParam);

                    m_gameDrawable.particles
                        .awake(9, 50, sf::Vector2f(),
                               dic(ColorDst::AcceleratedDownParticleFirst),
                               dic(ColorDst::AcceleratedDownParticleSecond),
                               50, 100, sf::microseconds(300000),
                               sf::microseconds(450000), 0.1f, -300, 100, 150);
                    m_particleNeedUpdatePosition = true;

                    break;
                case Acceleration::Up:
                    m_soundPlayer.playSound(SoundType::AccelerateUp, soundParam);

                    m_gameDrawable.particles
                        .awake(5, 100, sf::Vector2f(),
                               dic(ColorDst::AcceleratedUpParticleFirst),
                               dic(ColorDst::AcceleratedUpParticleSecond),
                               10, 100, sf::microseconds(150000),
                               sf::microseconds(200000), 0.1f, -2000, 600, 850);
                    m_particleNeedUpdatePosition = true;

                    break;
                default:
                    break;
                }
                break;
            case GameSubevent::BonusAppended:
                soundParam.relativeToListener = false;
                soundParam.position =
                    sf::Vector3f((float)m_game.getImpl().getSnakeWorld().getBonusPositions().begin()->x,
                                 (float)m_game.getImpl().getSnakeWorld().getBonusPositions().begin()->y, 0);
                m_soundPlayer.playSound(SoundType::BonusAppear, soundParam);
                break;
            case GameSubevent::BonusEaten:
                m_soundPlayer.playSound(SoundType::ItemEat, soundParam);

                m_gameDrawable.particles
                    .awake(7, 30, sf::Vector2f(),
                           dic(ColorDst::BonusEatenParticleFirst),
                           dic(ColorDst::BonusEatenParticleSecond),
                           20, 80, sf::microseconds(300000),
                           sf::microseconds(500000), 0.2f, -1000, 600, 600);
                m_particleNeedUpdatePosition = true;

                m_currBonusEatenCount++;
                m_currScore += m_levels.getLevelPlotDataPtr(m_difficulty,
                                                            m_levelIndex)[(int)LevelPlotDataEnum::BonusScoreCoeff];
                break;
            case GameSubevent::EffectAppended:
                m_soundPlayer.playSound(SoundType::EffectStarted, soundParam);
                break;
            case GameSubevent::FruitEaten:
                m_soundPlayer.playSound(SoundType::ItemEat, soundParam);

                m_gameDrawable.particles
                    .awake(5, 20, sf::Vector2f(),
                           dic(ColorDst::FruitEatenParticleFirst),
                           dic(ColorDst::FruitEatenParticleSecond),
                           10, 50, sf::microseconds(200000),
                           sf::microseconds(250000), 0.1f, -2000, 600, 600);
                m_particleNeedUpdatePosition = true;

                m_currFruitEatenCount++;
                m_currScore += m_levels.getLevelPlotDataPtr(m_difficulty,
                                                            m_levelIndex)[(int)LevelPlotDataEnum::FruitScoreCoeff];
                break;
            case GameSubevent::Killed:

                if (m_levelComplete)
                    m_soundPlayer.playSound(SoundType::LevelComplete, soundParam);
                else
                    m_soundPlayer.playSound(SoundType::Death, soundParam);

                m_toExit = true;
                m_toReturn = true;
                break;
            case GameSubevent::PowerupAppended:
                soundParam.relativeToListener = false;
                soundParam.position =
                    sf::Vector3f((float)m_game.getImpl().getSnakeWorld().getPowerups().begin()->first.x,
                                 (float)m_game.getImpl().getSnakeWorld().getPowerups().begin()->first.y, 0);
                m_soundPlayer.playSound(SoundType::PowerupAppear, soundParam);
                break;
            case GameSubevent::PowerupEaten:
                if (gameEvent.powerupEatenEvent.powerup >= PowerupType::EffectCount)
                    m_soundPlayer.playSound(SoundType::InstantPowerupChoke, soundParam);

                m_gameDrawable.particles
                    .awake(9, 50, sf::Vector2f(),
                           dic(ColorDst::SuperbonusEatenParticleFirst),
                           dic(ColorDst::SuperbonusEatenParticleSecond),
                           30, 100, sf::microseconds(400000),
                           sf::microseconds(650000), 0.2f, -800, 600, 600);
                m_particleNeedUpdatePosition = true;

                m_currPowerupEatenCount++;
                m_currScore +=
                    m_levels.getLevelPlotDataPtr(m_difficulty,
                                                 m_levelIndex)[(int)LevelPlotDataEnum::SuperbonusScoreCoeff];
                break;
            case GameSubevent::RotatedPostEffect:
                rotatedPostEffectOccured = true;
                break;
            case GameSubevent::RotatedPreEffect:
                m_soundPlayer.playSound(SoundType::ForcedRotating, soundParam);
                break;
            case GameSubevent::Stopped:
                m_soundPlayer.playSound(SoundType::StopHit, soundParam);

                m_gameDrawable.particles
                    .awake(6, 15, sf::Vector2f(),
                           dic(ColorDst::StoppedParticleFirst),
                           dic(ColorDst::StoppedParticleSecond),
                           40, 70, sf::microseconds(200000),
                           sf::microseconds(250000), 0.1f, -1000, 300, 400);
                m_particleNeedUpdatePosition = true;
                break;
            default:
                break;
            }
        }

        if (rotatedPostEffectOccured)
            m_rotatedPostEffect = true;
    }

    if (anyGameEvent) {
      // update all drawables by game event

        updateGame();
        checkLevelCompleted();
    }
}


void BlockSnake::endGame() {
  // Some links
    const std::uint32_t* plotPtr = m_levels.getLevelPlotDataPtr(m_difficulty, m_levelIndex);

    m_currGameTimeElapsed = m_gameClock.getElapsedTime<sf::Int64, std::micro>();

    bool levelCompl = true;

    unsigned int whatCount = 0;

    switch ((ChallengeType)plotPtr[(int)LevelPlotDataEnum::Challenge]) {
    case ChallengeType::Bonuses:
        whatCount = m_currBonusEatenCount;
        break;
    case ChallengeType::Fruits:
        whatCount = m_currFruitEatenCount;
        break;
    case ChallengeType::Powerups:
        whatCount = m_currPowerupEatenCount;
        break;
    default:
        break;
    }

    levelCompl = whatCount >= plotPtr[(int)LevelPlotDataEnum::ChallengeCount];

    LevelStatistics::StatisticsToAdd statToAddTemp;
    statToAddTemp.difficulty = m_difficulty;
    statToAddTemp.levelIndex = m_levelIndex;
    statToAddTemp.levelCompleted = levelCompl;
    statToAddTemp.gameTime = m_currGameTimeElapsed;
    statToAddTemp.score =
        (std::uint32_t)
        std::min((std::uintmax_t)UINT32_MAX,
                 (std::uintmax_t)plotPtr[(int)LevelPlotDataEnum::FruitScoreCoeff] *
                 m_currFruitEatenCount +
                 (std::uintmax_t)plotPtr[(int)LevelPlotDataEnum::BonusScoreCoeff] *
                 m_currBonusEatenCount +
                 (std::uintmax_t)plotPtr[(int)LevelPlotDataEnum::SuperbonusScoreCoeff] *
                 m_currPowerupEatenCount);

    m_levelStatistics.addStatistics(statToAddTemp);
    saveStatus();

    if (m_toReturn) {
        if (LevelStatsMusicId < m_musicTitles.size() &&
            m_music.openFromFile(m_musicTitles[LevelStatsMusicId].string())) {
        #if !0
            m_music.play();
        #endif
        }

        m_window.setMouseCursorVisible(true);

        switch (statisticMenu(levelCompl)) {
        case StatisticMenu::Again:
          // gameAgain = true;
            break;
        case StatisticMenu::Exit:
            m_gameAgain = false;
            m_toReturn = false;
            break;
        case StatisticMenu::ToLevelMenu:
            m_toReturn = true;
            m_gameAgain = false;
            break;
        default:
            break;
        }
    } else {
        m_gameAgain = false;
    }
}


void BlockSnake::pauseGame() {
    m_gameClock.pause();
    m_window.setMouseCursorVisible(true);
    bool pauseMenuAgain = true;

    do {
        switch (pauseMenu()) {
        case PauseMenuCommand::Continue:
            pauseMenuAgain = false;
            break;
        case PauseMenuCommand::Manual:
            m_toReturn = pauseMenuAgain = manual();
            m_toExit = !m_toReturn;
            break;
        case PauseMenuCommand::Settings:
            m_toReturn = pauseMenuAgain = settings();
            m_toExit = !m_toReturn;
            break;
        case PauseMenuCommand::ToMain:
            pauseMenuAgain = false;
            m_toExit = m_toReturn = true;
            break;
        case PauseMenuCommand::Exit:
        default:
            pauseMenuAgain = false;
            m_toReturn = false;
            m_toExit = true;
            break;
        }

    } while (pauseMenuAgain);

    if (!m_toExit && m_game.getEventProcessor()
        .getTimeToEvent((std::size_t)(MainGameEvent::TimeLimitExceed)) > 0) // patch
    {
        m_window.setMouseCursorVisible(false);
        m_gameClock.resume();
    }
}

} // namespace Bulletworm
