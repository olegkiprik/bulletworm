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

#include "GameDrawable.hpp"
#include "Constants.hpp"
#include <bw_ext/GraphicalUtility.hpp>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
bool GameDrawable::initConfig(const sf::Vector2f& windowSize,
                              const sf::Vector2u& innerViewSize, 
                              const sf::Texture& texture, const sf::Texture& digitTex,
                              std::uint32_t snakeBodyFill,
                              std::uint32_t snakeBodyOutline,
                              std::uint32_t snakePointerFill,
                              std::uint32_t snakePointerOutline,
                              std::uint32_t scoreColor,
                              std::uint32_t highestScoreColor,
                              std::uint32_t foggColor) {
    if (!centralView.init(TexSz, TexUnitWidth, (sf::Vector2i)innerViewSize,
        texture, foggColor))
        return false;

    particles.init(NrParticles);

    snakeCircle.setRadius(float(TexSz) / 4);
    snakeCircle.setFillColor((sf::Color)snakeBodyFill);
    snakeCircle.setOutlineThickness(float(TexSz) / 10 / 4);
    snakeCircle.setOutlineColor((sf::Color)snakeBodyOutline);
    snakeCircle.setOrigin(float(TexSz) / 4, float(TexSz) / 4);

    snakePositionPointer.setRadius(float(TexSz) / 7);
    snakePositionPointer.setFillColor((sf::Color)snakePointerFill);
    snakePositionPointer.setOutlineThickness(float(TexSz) / 10 / 5);
    snakePositionPointer.setOutlineColor((sf::Color)snakePointerOutline);
    snakePositionPointer.setOrigin(float(TexSz) / 7, float(TexSz) / 7);

    snakeEndPositionPointer.setRadius(float(TexSz) / 10);
    snakeEndPositionPointer.setFillColor((sf::Color)snakePointerFill);
    snakeEndPositionPointer.setOutlineThickness(float(TexSz) / 10 / 5);
    snakeEndPositionPointer.setOutlineColor((sf::Color)snakePointerOutline);
    snakeEndPositionPointer.setOrigin(float(TexSz) / 7,
                                      float(TexSz) / 7);

    sf::Vector2f winSizeWoSc = windowSize;

    sf::Vector2u outerViewSize(innerViewSize.x + 2, innerViewSize.y + 2);
    sf::Vector2f leftTopOnWin;
    float mainScale;

    // check the orientation
    verticalOrientation = (windowSize.y > windowSize.x);

    if (verticalOrientation)
        winSizeWoSc.x = windowSize.x * ScaleYieldNumerator / ScaleYieldDenominator;
    else
        winSizeWoSc.y = windowSize.y * ScaleYieldNumerator / ScaleYieldDenominator;

    const float fullSightAspect = float(outerViewSize.x) / outerViewSize.y; // local
    const float winSizeWSaspect = winSizeWoSc.x / winSizeWoSc.y; // local
    float elementSize;

    if (fullSightAspect < winSizeWSaspect) {
        leftTopOnWin = sf::Vector2f(windowSize.x - fullSightAspect * winSizeWoSc.y,
                                    (windowSize.y - winSizeWoSc.y)) * 0.5f;
        elementSize = winSizeWoSc.y / outerViewSize.y;
    } else {
        leftTopOnWin = sf::Vector2f((windowSize.x - winSizeWoSc.x),
                                    windowSize.y - winSizeWoSc.x / fullSightAspect) * 0.5f;
        elementSize = winSizeWoSc.x / outerViewSize.x;
    }

    mainScale = elementSize / TexSz;

    sf::Transformable trans;
    trans.setPosition(leftTopOnWin);
    trans.setScale(mainScale, mainScale);
    centralTransform = trans.getTransform();

    challengeVisual.setRadius(windowSize.x * 75 / 1920);
    challengeVisualOutline.setRadius(windowSize.x * 75 / 1920);
    fruitCountToBonusVisual.setRadius(windowSize.x * 75 / 1920);
    fruitCountToBonusVisualOutline.setRadius(windowSize.x * 75 / 1920);
    bonusCountToPowerupVisual.setRadius(windowSize.x * 75 / 1920);
    bonusCountToPowerupVisualOutline.setRadius(windowSize.x * 75 / 1920);

    digits.setTexture(digitTex);
    digits.setDigitCount(10);
    digits.setZeroDigitRect(sf::IntRect(0, 0, 32, 64));
    for (int i = 0; i < 10; ++i)
        digits.setColor(scoreColor, i);
    digits.setOrigin(320, 0);
    digits.setPosition(windowSize.x, 0);
    digits.setScale(0.7f, 0.7f);

    highestScore.setTexture(digitTex);
    highestScore.setDigitCount(10);
    highestScore.setZeroDigitRect(sf::IntRect(0, 0, 32, 64));
    for (int i = 0; i < 10; ++i)
        highestScore.setColor(highestScoreColor, i);
    highestScore.setOrigin(320, 0);
    highestScore.setPosition(windowSize.x, 50);
    highestScore.setScale(0.7f, 0.7f);

    if (verticalOrientation) {
        scaleWidth = (windowSize.x - winSizeWoSc.x) / 2;
        scaleMaxLength = windowSize.y / 2;

        bonusScale.setOrigin(0, 0);
        powerupScale.setOrigin(0, 0);
        effectScale.setOrigin(scaleWidth, 0);
        timeLimitScale.setOrigin(scaleWidth, 0);

        bonusScale.setPosition(0, 0);
        powerupScale.setPosition(0, windowSize.y / 2);
        effectScale.setPosition(windowSize.x, 0);
        timeLimitScale.setPosition(windowSize.x, windowSize.y / 2);
    } else {
        scaleWidth = (windowSize.y - winSizeWoSc.y) / 2;
        scaleMaxLength = windowSize.x / 2;

        bonusScale.setOrigin(0, 0);
        powerupScale.setOrigin(0, 0);
        effectScale.setOrigin(0, scaleWidth);
        timeLimitScale.setOrigin(0, scaleWidth);

        bonusScale.setPosition(0, 0);
        powerupScale.setPosition(windowSize.x / 2, 0);
        effectScale.setPosition(0, windowSize.y);
        timeLimitScale.setPosition(windowSize.x / 2, windowSize.y);
    }

    return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void GameDrawable::setBonusScale(float level) {
    setScale(bonusScale, level);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void GameDrawable::setPowerupScale(float level) {
    setScale(powerupScale, level);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void GameDrawable::setEffectScale(float level) {
    setScale(effectScale, level);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void GameDrawable::setTimeLimitScale(float level) {
    setScale(timeLimitScale, level);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void GameDrawable::setScale(sf::RectangleShape& shape, float level) {
    if (verticalOrientation)
        shape.setSize(sf::Vector2f(scaleWidth, scaleMaxLength * level));
    else
        shape.setSize(sf::Vector2f(scaleMaxLength * level, scaleWidth));

    shape.setFillColor((sf::Color)scaleColor(level));
}

}