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

#include <bw_ext/SpriteArray.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <bw_ext/const/Orientation.hpp>
#include <cstring>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
SpriteArray::SpriteArray() noexcept :
	m_vertices(10000),
	m_texture(nullptr) {}


////////////////////////////////////////////////////////////////////////////////////////////////////
SpriteArray::SpriteArray(const sf::Texture& texture) noexcept :
	m_vertices(10000),	
	m_texture(&texture) {}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteArray::push(const sf::IntRect& textureRect, const sf::Vector2i& ltposition, Orientation orientation) {
	sf::Vertex vertices[6];

	vertices[0].texCoords = sf::Vector2f((float)textureRect.left, (float)textureRect.top);
	vertices[1].texCoords = sf::Vector2f((float)(textureRect.left + textureRect.width), (float)textureRect.top);

	vertices[2].texCoords =
		sf::Vector2f((float)(textureRect.left + textureRect.width), (float)(textureRect.top + textureRect.height));
	vertices[3].texCoords =
		sf::Vector2f((float)(textureRect.left + textureRect.width), (float)(textureRect.top + textureRect.height));

	vertices[4].texCoords = sf::Vector2f((float)textureRect.left, (float)(textureRect.top + textureRect.height));
	vertices[5].texCoords = sf::Vector2f((float)textureRect.left, (float)textureRect.top);

	sf::Vector2i identpos[4]
	{
		sf::Vector2i(),
		sf::Vector2i(textureRect.width, 0),
		sf::Vector2i(textureRect.width, textureRect.height),
		sf::Vector2i(0, textureRect.height)
	};

	sf::Vector2i pos[4];

	switch (orientation) {
	case Orientation::FlipHorizontally:
		pos[0] = identpos[1];
		pos[1] = identpos[0];
		pos[2] = identpos[3];
		pos[3] = identpos[2];
		break;
	case Orientation::RotateClockwise:
		pos[0] = identpos[1];
		pos[1] = identpos[2];
		pos[2] = identpos[3];
		pos[3] = identpos[0];
		break;
	case Orientation::InverseTranspose:
		pos[0] = identpos[2];
		pos[1] = identpos[1];
		pos[2] = identpos[0];
		pos[3] = identpos[3];
		break;
	case Orientation::Flip:
		pos[0] = identpos[2];
		pos[1] = identpos[3];
		pos[2] = identpos[0];
		pos[3] = identpos[1];
		break;
	case Orientation::FlipVertically:
		pos[0] = identpos[3];
		pos[1] = identpos[2];
		pos[2] = identpos[1];
		pos[3] = identpos[0];
		break;
	case Orientation::RotateCounterClockwise:
		pos[0] = identpos[3];
		pos[1] = identpos[0];
		pos[2] = identpos[1];
		pos[3] = identpos[2];
		break;
	case Orientation::MainTranspose:
		pos[0] = identpos[0];
		pos[1] = identpos[3];
		pos[2] = identpos[2];
		pos[3] = identpos[1];
		break;
	default:
		pos[0] = identpos[0];
		pos[1] = identpos[1];
		pos[2] = identpos[2];
		pos[3] = identpos[3];
		break;
	}

	std::for_each(pos, pos + 4,
				  [&ltposition](sf::Vector2i& vec) {
					  // pos
					  vec += ltposition;
				  });

	vertices[0].position = (sf::Vector2f)pos[0];
	vertices[1].position = (sf::Vector2f)pos[1];
	vertices[2].position = (sf::Vector2f)pos[2];
	vertices[3].position = (sf::Vector2f)pos[2];
	vertices[4].position = (sf::Vector2f)pos[3];
	vertices[5].position = (sf::Vector2f)pos[0];

	if (m_used_size+6 > m_vertices.size()) {
		m_vertices.resize(m_vertices.size() + 6);
	}

	std::memcpy(m_vertices.data() + m_used_size, vertices, 6 * sizeof(sf::Vertex));
	m_used_size += 6;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteArray::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.texture = m_texture;
	target.draw(m_vertices.data(), m_used_size, PrimitiveType, states);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteArray::clear() noexcept {
	m_used_size = 0;
}

}
