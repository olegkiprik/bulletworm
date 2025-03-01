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

#include "CentralViewScreen.hpp"
#include <bw_ext/const/Orientation.hpp>
#include <bw_ext/ObjParamEnumUtility.hpp>
#include "Constants.hpp"
#include <bw_ext/GraphicalUtility.hpp>
#include <cassert>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
CentraViewScreen::CentraViewScreen() :
	vbscreens(SpriteArray::PrimitiveType, sf::VertexBuffer::Static),
	vbforegroundObjects(SpriteArray::PrimitiveType, sf::VertexBuffer::Dynamic),
	vbbackgroundObjects(SpriteArray::PrimitiveType, sf::VertexBuffer::Dynamic) {}


////////////////////////////////////////////////////////////////////////////////////////////////////
bool CentraViewScreen::init(unsigned int texSz,
							unsigned int texUnitWidth,
							const sf::Vector2i& thesize,
							const sf::Texture& texture,
							std::uint32_t foggColor) {
	assert(texSz && texUnitWidth);

	m_texSz = texSz;
	m_texUnitWidth = texUnitWidth;

	fogg.setSize(sf::Vector2f((float)m_texSz * thesize.x, 
				 (float)m_texSz * thesize.y));
	fogg.setFillColor(sf::Color(foggColor));
	fogg.setPosition(float(texSz), float(texSz));

	sf::IntRect screenCornerRect =
		getTextureUnitRect((int)TextureUnit::ScreenCorner +
						   m_screenTheme * TextureUnitCount,
						   m_texSz, m_texUnitWidth);
	sf::IntRect screenHorizontalRect =
		getTextureUnitRect((int)TextureUnit::ScreenHorizontal +
						   m_screenTheme * TextureUnitCount,
						   m_texSz, m_texUnitWidth);

	SpriteArray screensTemp;

	// corners
	screensTemp.push(screenCornerRect, sf::Vector2i(), Orientation::Identity);

	screensTemp.push(screenCornerRect, sf::Vector2i(m_texSz * (thesize.x + 1), 0),
					 Orientation::RotateClockwise);

	screensTemp.push(screenCornerRect, sf::Vector2i(0, m_texSz * (thesize.y + 1)), 
					 Orientation::RotateCounterClockwise);

	screensTemp.push(screenCornerRect, sf::Vector2i((m_texSz * (thesize.x + 1)), 
					 (m_texSz * (thesize.y + 1))), Orientation::Flip);

	// horizontally (without corners)
	for (int i = 1; i < thesize.x + 1; ++i) {
		screensTemp.push(screenHorizontalRect, sf::Vector2i(m_texSz * i, 0),
						 Orientation::Identity);
		screensTemp.push(screenHorizontalRect, sf::Vector2i(m_texSz * i,
						 m_texSz * (thesize.y + 1)),
						 Orientation::Identity);
	}

	// vertically (without corners)
	for (int i = 1; i < thesize.y + 1; ++i) {
		screensTemp.push(screenHorizontalRect, sf::Vector2i(0, m_texSz * i), 
						 Orientation::RotateClockwise);
		screensTemp.push(screenHorizontalRect, sf::Vector2i((m_texSz * (thesize.x + 1)), 
						 (m_texSz * i)), Orientation::RotateClockwise);
	}

	// fit
	int objVbSize = std::max((thesize.x + 1) * thesize.y, thesize.x * (thesize.y + 1));

	if (!vbscreens.create(screensTemp.getVertexCount()) ||
		!vbscreens.update(screensTemp.getVertices()) ||
		!vbbackgroundObjects.create((std::size_t)objVbSize * 6) ||
		!vbforegroundObjects.create((std::size_t)objVbSize * 6))
		return false;

	// other
	setTexture(texture);
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CentraViewScreen::pushFruit(const sf::Vector2i& position, Direction tailing,
								 const sf::Vector2i& innerViewSize) {
	pushItem(position, tailing, innerViewSize, EatableItem::Fruit, TextureUnit::Fruit, m_fruitTheme);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CentraViewScreen::pushBonus(const sf::Vector2i& position, Direction tailing,
								 const sf::Vector2i& innerViewSize) {
	pushItem(position, tailing, innerViewSize, EatableItem::Bonus, TextureUnit::Bonus, m_bonusTheme);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CentraViewScreen::pushPowerup(PowerupType powerup, const sf::Vector2i& position,
								   Direction tailing, const sf::Vector2i& innerViewSize) {
	TextureUnit texun = TextureUnit::Void;

	switch (powerup) {
	case PowerupType::SlowDown:
		texun = TextureUnit::PowerSlowDown;
		break;
	case PowerupType::TailHarmless:
		texun = TextureUnit::PowerTailHarmless;
		break;
	case PowerupType::InstantTailCut:
		texun = TextureUnit::PowerInstantTailCut;
		break;
	default:
		break;
	}

	pushItem(position, tailing, innerViewSize, EatableItem::Powerup, texun, m_superbonusTheme);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CentraViewScreen::pushUnknownPowerup(const sf::Vector2i& position, Direction tailing,
										  const sf::Vector2i& innerViewSize) {
	pushItem(position, tailing, innerViewSize, EatableItem::Powerup,
			 TextureUnit::PowerUnknown, m_superbonusTheme);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
bool CentraViewScreen::updateVBs() {
	vbvxcounttodrawbg = vabackgroundObjects.getVertexCount();
	vbvxcounttodrawfg = vaforegroundObjects.getVertexCount();
	bool succ1 = vbbackgroundObjects.update(vabackgroundObjects.getVertices(),
											vbvxcounttodrawbg, 0);
	bool succ2 = vbforegroundObjects.update(vaforegroundObjects.getVertices(),
											vbvxcounttodrawfg, 0);

	return succ1 && succ2;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CentraViewScreen::clear() noexcept {
	vabackgroundObjects.clear();
	vaforegroundObjects.clear();
	snakeDrawable.clear();

	std::for_each(items.begin(), items.end(),
				  [](SpriteArray& now) {
					  now.clear();
				  });

	std::for_each(screenItems.begin(), screenItems.end(),
				  [](SpriteArray& now) {
					  now.clear();
				  });
}


////////////////////////////////////////////////////////////////////////////////////////////////////
sf::Transform CentraViewScreen::getTransFromOrient(Orientation orientation,
												   unsigned texSz) noexcept {
	sf::Transformable trans;
	trans.setOrigin((float)texSz / 2, (float)texSz / 2);
	trans.setPosition((float)texSz / 2, (float)texSz / 2);

	switch (orientation) {
	case Orientation::FlipHorizontally:
		trans.setScale(-1, 1);
		break;
	case Orientation::RotateClockwise:
		trans.setRotation(90);
		break;
	case Orientation::InverseTranspose:
		trans.setRotation(90);
		trans.setScale(-1, 1);
		break;
	case Orientation::Flip:
		trans.setScale(-1, -1);
		break;
	case Orientation::FlipVertically:
		trans.setScale(1, -1);
		break;
	case Orientation::RotateCounterClockwise:
		trans.setRotation(270);
		break;
	case Orientation::MainTranspose:
		trans.setRotation(90);
		trans.setScale(1, -1);
		break;
	default:
		break;
	}

	return trans.getTransform();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
SpriteArray& CentraViewScreen::useItemArray(EatableItem item) noexcept {
	return items[static_cast<std::size_t>(item)];
}


////////////////////////////////////////////////////////////////////////////////////////////////////
const SpriteArray& CentraViewScreen::getItemArray(EatableItem item) const noexcept {
	return items[static_cast<std::size_t>(item)];
}


////////////////////////////////////////////////////////////////////////////////////////////////////
SpriteArray& CentraViewScreen::useScreenItemArray(EatableItem item, ScreenMode mode) noexcept {
	return screenItems[static_cast<int>(item) + 
		(std::size_t)ItemCount * static_cast<int>(mode)];
}


////////////////////////////////////////////////////////////////////////////////////////////////////
const SpriteArray& CentraViewScreen::getScreenItemArray(EatableItem item, 
														ScreenMode mode) const noexcept {
	return screenItems[static_cast<int>(item) + 
		(std::size_t)ItemCount * static_cast<int>(mode)];
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CentraViewScreen::setTexture(const sf::Texture& texture) noexcept {
	std::for_each(items.begin(), items.end(),
				  [&texture](SpriteArray& now) {
					  now.setTexture(texture);
				  });

	std::for_each(screenItems.begin(), screenItems.end(),
				  [&texture](SpriteArray& now) {
					  now.setTexture(texture);
				  });
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CentraViewScreen::pushBgObj(const sf::Vector2i& position,
								 TextureUnit unit, std::uint32_t theme,
								 Orientation orientation) {
	sf::IntRect texRect = getTextureUnitRect((int)unit + theme * TextureUnitCount,
											 m_texSz, m_texUnitWidth);
	vabackgroundObjects.push(texRect, sf::Vector2i(((position.x + 1) * m_texSz),
							 ((position.y + 1) * m_texSz)), orientation);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CentraViewScreen::pushFgObj(const sf::Vector2i& position,
								 TextureUnit unit, std::uint32_t theme, 
								 Orientation orientation) {
	sf::IntRect texRect = getTextureUnitRect((int)unit + theme * TextureUnitCount,
											 m_texSz, m_texUnitWidth);
	vaforegroundObjects.push(texRect, sf::Vector2i(((position.x + 1) * m_texSz),
							 ((position.y + 1) * m_texSz)), orientation);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CentraViewScreen::pushItem(const sf::Vector2i& position,
								Direction tailing,
								const sf::Vector2i& innerViewSize,
								EatableItem item, TextureUnit unit,
								std::uint32_t theme) {
	sf::IntRect texRect = getTextureUnitRect((int)unit +
											 theme * TextureUnitCount,
											 m_texSz, m_texUnitWidth);

	sf::Vector2i posToDraw = position;

	switch (tailing) {
	case Direction::Right:
	case Direction::Left:
		posToDraw.x = std::min(posToDraw.x, innerViewSize.x - 1);
		break;
	case Direction::Down:
	case Direction::Up:
		posToDraw.y = std::min(posToDraw.y, innerViewSize.y - 1);
		break;
	default:
		break;
	}

	if ((position.x == -1 || position.x == innerViewSize.x) &&
		(position.y == -1 || position.y == innerViewSize.y)) // corner
	{
		useScreenItemArray(item, ScreenMode::Corner).push(texRect,
														  sf::Vector2i(((posToDraw.x + 1) * m_texSz),
														  ((posToDraw.y + 1) * m_texSz)),
														  Orientation::Identity);
	} else if (position.x == -1 || position.x == innerViewSize.x) // vertical
	{
		useScreenItemArray(item, ScreenMode::Vertical).push(texRect,
															sf::Vector2i(((posToDraw.x + 1) * m_texSz),
															((posToDraw.y + 1) * m_texSz)),
															Orientation::Identity);
	} else if (position.y == -1 || position.y == innerViewSize.y) // horizontal
	{
		useScreenItemArray(item, ScreenMode::Horizontal).push(texRect, 
															  sf::Vector2i(((posToDraw.x + 1) * m_texSz),
															  ((posToDraw.y + 1) * m_texSz)),
															  Orientation::Identity);
	} else // plain
	{
		useItemArray(item).push(texRect, sf::Vector2i(((posToDraw.x + 1) * m_texSz),
								((posToDraw.y + 1) * m_texSz)),
								Orientation::Identity);
	}
}

}