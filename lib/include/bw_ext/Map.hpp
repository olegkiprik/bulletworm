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

#ifndef MAP_HPP
#define MAP_HPP
#include <SFML/System/Vector2.hpp>
#include <vector>

namespace Bulletworm {

// 2 dimensional vector
template<class T>
class Map {
public:

	Map() noexcept = default;
	Map(const Map<T>&) = default;
	Map(Map<T>&&) noexcept;

	Map<T>& operator=(const Map<T>&) = default;
	Map<T>& operator=(Map<T>&&) noexcept;

	// I assume the map contains elementary data not threads ;)
	void create(unsigned int width, unsigned int height, T element);
	void create(const sf::Vector2u& size, T element);

	// with the elements prepared
	void create(unsigned int width, unsigned int height, const T* elements);
	void create(const sf::Vector2u& size, const T* elements);

	T* data() noexcept {
		return m_elements.data();
	}
	const T* data() const noexcept {
		return m_elements.data();
	}

	T& at(int x, int y) noexcept;
	T at(int x, int y) const noexcept;

	T at(const sf::Vector2i& position) const noexcept {
		return at(position.x, position.y);
	}
	T& at(const sf::Vector2i& position) noexcept {
		return at(position.x, position.y);
	}

	T operator[](const sf::Vector2i& position) const noexcept {
		return at(position);
	}
	T& operator[](const sf::Vector2i& position) noexcept {
		return at(position);
	}

	const sf::Vector2u& getSize() const noexcept {
		return m_size;
	}

private:

	std::vector<T> m_elements;   // Map elements
	sf::Vector2u m_size;         // Map size
};


////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
Map<T>::Map(Map<T>&& src) noexcept :
	m_elements(std::move(src.m_elements)),
	m_size(src.m_size) {
	src.m_size.x = 0;
	src.m_size.y = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
Map<T>& Map<T>::operator=(Map<T>&& src) noexcept {
	if (this == &src)
		return *this;

	m_elements = std::move(src.m_elements);
	m_size = src.m_size;

	src.m_size.x = 0;
	src.m_size.y = 0;

	return *this;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
void Map<T>::create(unsigned int width, unsigned int height, T element) {
	if (width && height) {
		unsigned int area = width * height;

		std::vector<T>(area, element).swap(m_elements);

		m_size.x = width;
		m_size.y = height;
	} else {
		std::vector<T>().swap(m_elements);

		m_size.x = 0;
		m_size.y = 0;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
void Map<T>::create(unsigned int width, unsigned int height, const T* elements) {
	if (width && height && elements) {
		unsigned int area = width * height;

		m_elements.assign(elements, elements + area);

		m_size.x = width;
		m_size.y = height;
	} else {
		std::vector<T>().swap(m_elements);

		m_size.x = 0;
		m_size.y = 0;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
void Map<T>::create(const sf::Vector2u& size, T element) {
	create(size.x, size.y, element);
}

template<class T>
void Map<T>::create(const sf::Vector2u& size, const T* elements) {
	create(size.x, size.y, elements);
}

template<class T>
T& Map<T>::at(int x, int y) noexcept {
	return m_elements[x + (std::size_t)y * m_size.x];
}

template<class T>
T Map<T>::at(int x, int y) const noexcept {
	return m_elements[x + (std::size_t)y * m_size.x];
}

} // namespace Bulletworm

#endif // !MAP_HPP
