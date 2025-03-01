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

#ifndef BLOCK_SNAKE_HPP
#define BLOCK_SNAKE_HPP
#include "Game.hpp"
#include "Levels.hpp"
#include "LevelStatistics.hpp"
#include "GameDrawable.hpp"
#include <bw_ext/PausableClock.hpp>
#include <bw_ext/random/RandomizerImpl.hpp>
#include "SoundPlayer.hpp"
#include "engine/ObjectBehavior.hpp"
#include "LevelElements.hpp"
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <fstream>
#include <memory>
#include <filesystem>

namespace Bulletworm {

enum class MainMenuCommand;
enum class LevelMenuCommand;
enum class PauseMenuCommand;
enum class StatisticMenu;
enum class Word;
enum class ColorDst;

class BlockSnake {
public:

    // Word Wrap (shall be fixed!)
    static sf::String getWord2fit(sf::String src,
        float fitWidth,
        unsigned int charSize,
        const sf::Font& font);

    BlockSnake();

    [[nodiscard]] bool start();

private:

    // basic init func

    bool initTextures(); // textures
    void createWindow(bool resetVirtual=false); // window

    bool loadStatus();
    bool loadData();

    bool loadLists();
    bool loadWallpapers();

    bool loadCursor();
    bool loadLanguages();

    void setupMusic();
    bool setupRandomizer() noexcept;

    bool saveStatusSub() const;
    bool saveStatus();

    // menu automaton

    MainMenuCommand mainMenu();
    LevelMenuCommand selectLevel();
    PauseMenuCommand pauseMenu();
    StatisticMenu statisticMenu(bool completed);
    bool settings();
    bool manual();
    bool languages();

    // main loop

    void changeWallpaper(unsigned int id, const sf::Vector2f& windowSize);

    void mainLoop();
    [[nodiscard]] bool selectLevelProcessing();

    [[nodiscard]] bool playGame();

    void createChallVisual();
    void prepareGame();
    void playGameMusic();

    // change central view and challenge visual after move
    void updateGame();

    sf::IntRect getInnerVisibleZone() const;
    bool isCameraStopped() const;

    // inner camera bias
    sf::Vector2f getCameraBias(sf::Int64 nowTime) const;

    void updateItems(EatableItem item);
    void updateUnits();
    void updateSnakeDrawable();

    // change scales (frequently!)
    void scaleUpdate();

    void checkLevelCompleted();

    static sf::Vector2f getPositionOfCircleEntry(
        Direction dir,
        const sf::Vector2i& pos) noexcept;

    static sf::Vector2f getPositionOfCircleExit(Direction dir,
                                                const sf::Vector2i& pos) noexcept;

    void drawWindow();
    void drawScreens(sf::RenderStates states, float shaderSecs);
    void drawScales();
    void drawChallVis(float shaderSecs);

    void processEvents();
    void processGameEvents();
    void endGame();

    void pauseGame();

    // MEMBERS

    GameDrawable m_gameDrawable; // game graphics    
    std::array<sf::Shader, VisualEffectCount> m_shaders;
    // random
    RandomizerImpl m_randomizer;
    SoundPlayer m_soundPlayer;
    // main game states
    Game m_game;                 // game manager
    std::array<sf::Font, FontCount> m_fonts;
    sf::Cursor m_cursor; // destroy the window before destroying the cursor
    sf::RenderWindow m_window; // Window
    // error management
    mutable std::ofstream m_logger;
    sf::Sprite m_background;
    std::array<std::uint32_t, ColorDstCount> m_colors;   // Colors
    sf::Music m_music;
    sf::Music m_ambient;
    Levels m_levels;
    LevelStatistics m_levelStatistics;
    // current loaded map layers
    std::array<Map<std::uint32_t>, ItemCount> m_currentItemProbabilities;
    sf::Transform m_particleSystemTransform;
    std::array<std::uint32_t, ObjectPairCount> m_objectPreEffects{};
    std::array<std::uint32_t, ObjectPairCount> m_objectPostEffects{};
    std::array<std::uint32_t, ObjectPairCount> m_objectTailCapacities1{};
    sf::Texture m_digitTexture;
    std::array<std::uint32_t, SettingCount> m_settings{};
public:
    std::string pwd;
private:
    sf::Image m_iconImg;
    std::vector<ObjectBehavior> m_objectBehaviors;
    std::vector<std::uint32_t> m_initialObjectMemory;
    // localization
    std::vector<sf::String> m_words;
    std::vector<std::filesystem::path>
        m_musicTitles, 
        m_soundTitles,
        m_textureTitles,
        m_shaderTitles, 
        m_fontTitles, 
        m_languageTitles, 
        m_wallpaperTitles;
    std::vector<std::uintmax_t> m_currentSnakePosProbs;
    std::vector<std::uint32_t> m_currentObjPairIndices;
    std::vector<std::uint32_t> m_currentObjParams;
    std::vector<std::uint32_t> m_currentThemes;
    PausableClock m_gameClock;   // game clock
    std::shared_ptr<sf::Texture> m_menuWallpaper; // 'zero'
    std::shared_ptr<sf::Texture> m_secondCachedWallpaper;
    // textures
    std::unique_ptr<sf::Texture> m_textures;
    // sector graphs
    sf::Clock m_challengeVisualClock;
    sf::Clock m_fruit2bonusVisualClock;
    sf::Clock m_bonus2superbonusClock;
    sf::Clock m_scoreVisualClock;
    sf::Clock m_shaderClock;
    sf::Clock m_particleClock;
    sf::Int64 m_nowTime = 0;
    std::size_t m_challengeVisualCount = 0;
    std::size_t m_fruit2bonusVisualCount = 0;
    std::size_t m_bonus2superbonusVisualCount = 0;
    std::size_t m_visualScore = 0;
    sf::Vector2u m_virtualWinSize;
    // window
    // (graphics)
    // to determinate snake tail end
    sf::Vector2i m_snakeTailEnd;
    sf::Vector2i m_snakeTailPreend;
    
    sf::Int64 m_currGameTimeElapsed = 0;
    std::uintmax_t m_currScore = 0;

    // Recorded time of now. The frequently used parameter
    

    // (graphics)
    // to set camera position
    sf::Int64 m_lastMoveEventTimePoint = 0;
    unsigned int m_2cachedWallpaperIndex = 0;
    // current selected level
    unsigned int m_levelIndex = 0;
    unsigned int m_difficulty = 0;
    unsigned int m_currFruitEatenCount = 0;
    unsigned int m_currBonusEatenCount = 0;
    unsigned int m_currPowerupEatenCount = 0;
    unsigned int m_currStepCount = 0;
    // is current level completed for this moment
    bool m_levelComplete = false;
    bool m_particleNeedUpdatePosition = false;
    bool m_snakeTailEndVisible = false;
    bool m_snakeTailPreendVisible = false;

    // for implementing forced snake turn
    bool m_rotatedPostEffect = false;
// dirty hack
    bool m_movingReserved = false;
    bool m_movingReserved2 = false;
    // menu navigation
    bool m_toReturn = false;
    bool m_toExit = false;
    bool m_gameAgain = false;

    // aliases

    std::uint32_t getSetting(SettingEnum st) const noexcept {
        return m_settings[(std::size_t)st];
    }

    const sf::Font& getFont(FontType type) const noexcept {
        return m_fonts[(std::size_t)type];
    }

    const sf::String& getWord(std::size_t lang, Word word) const noexcept;

    const sf::String& getLevelDescr(unsigned int lang,
                                    unsigned int level,
                                    unsigned int diff) const noexcept;

    std::uint32_t getDestinationIntColor(ColorDst dst) const noexcept {
        return m_colors[(std::size_t)dst];
    }

    sf::Color getDestinationColor(ColorDst dst) const noexcept {
        return (sf::Color)getDestinationIntColor(dst);
    }
};

} // Bulletworm

#endif // !BLOCK_SNAKE_HPP
