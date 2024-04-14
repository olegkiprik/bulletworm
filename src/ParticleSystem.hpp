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

#ifndef PARTICLE_SYSTEM_HPP
#define PARTICLE_SYSTEM_HPP
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <vector>
#include <cstdint>
#include <cmath>

namespace Bulletworm {

class ParticleSystem : public sf::Drawable, public sf::Transformable {
public:

	ParticleSystem() noexcept;

	explicit ParticleSystem(std::size_t count);

	void init(std::size_t count);

	void update(sf::Time elapsed) noexcept;

	void awake(float particleRadius,
			   std::size_t count,
			   const sf::Vector2f& centralPosition,
			   std::uint32_t firstColor,
			   std::uint32_t secondColor,
			   float minDistance,
			   float maxDistance,
			   sf::Time minLifetime,
			   sf::Time maxLifetime,
			   float secondColorRatio,
			   float acceleration,
			   float minVelocity,
			   float maxVelocity);

private:

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:

	struct Particle {
		sf::Vector2f acceleration;
		sf::Vector2f velocity;
		sf::Time lifetime;
		sf::Time constLifetime;
	};

	std::vector<Particle> m_particles;
	std::vector<sf::Vertex> m_vertices;
};

}

#endif // !PARTICLE_SYSTEM_HPP
