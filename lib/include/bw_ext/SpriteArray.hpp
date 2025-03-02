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

#ifndef SPRITE_ARRAY_HPP
#define SPRITE_ARRAY_HPP
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <vector>

namespace Bulletworm {

enum class Orientation;

// just drop sprites here
class SpriteArray : public sf::Drawable {
public:

	// move semantics ok

	SpriteArray() noexcept;
	explicit SpriteArray(const sf::Texture& texture) noexcept;

	void clear() noexcept;

	void push(const sf::IntRect& textureRect, const sf::Vector2i& ltposition, Orientation orientation);

	void setTexture(const sf::Texture& texture) noexcept {
		m_texture = &texture;
	}
	const sf::Texture* getTexture() const noexcept {
		return m_texture;
	}

	const sf::Vertex* getVertices() const noexcept {
		return m_vertices.data();
	}
	std::size_t getVertexCount() const noexcept {
		return m_used_size;
	}

	static constexpr sf::PrimitiveType PrimitiveType = sf::Triangles;

private:

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	std::vector<sf::Vertex> m_vertices;
	std::size_t m_used_size = 0;
	const sf::Texture* m_texture;
};

}

#endif // !SPRITE_ARRAY_HPP