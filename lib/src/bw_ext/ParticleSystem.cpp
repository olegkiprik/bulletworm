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

#include <bw_ext/ParticleSystem.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace Bulletworm {

ParticleSystem::ParticleSystem() noexcept {}
ParticleSystem::ParticleSystem(std::size_t count) :
	m_particles(count),
	m_vertices(count * 3) {}
void ParticleSystem::init(std::size_t count) {
	m_particles.resize(count);
	m_vertices.resize(count * 3);
}
void ParticleSystem::update(sf::Time elapsed) noexcept {
	for (std::size_t i = 0; i < m_particles.size(); ++i) {
		Particle& p = m_particles[i];
		p.lifetime -= elapsed;

		if (p.lifetime <= sf::Time::Zero) {
			m_vertices[0 + i * 3].color.a = 0;
			m_vertices[1 + i * 3].color.a = 0;
			m_vertices[2 + i * 3].color.a = 0;
			continue;
		}

		m_vertices[0 + i * 3].position += p.velocity * elapsed.asSeconds();
		m_vertices[1 + i * 3].position += p.velocity * elapsed.asSeconds();
		m_vertices[2 + i * 3].position += p.velocity * elapsed.asSeconds();

		p.velocity += p.acceleration * elapsed.asSeconds();

		float ratio = p.lifetime.asSeconds() / p.constLifetime.asSeconds();

		m_vertices[0 + i * 3].color.a = static_cast<std::uint8_t>(ratio * 255);
		m_vertices[1 + i * 3].color.a = static_cast<std::uint8_t>(ratio * 255);
		m_vertices[2 + i * 3].color.a = static_cast<std::uint8_t>(ratio * 255);
	}
}
void ParticleSystem::awake(float particleRadius,
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
						   float maxVelocity) {
	constexpr float pi = 3.141592654f;

	for (std::size_t i = 0; i < std::min(count, m_particles.size()); ++i) {
		Particle& p = m_particles[i];
		float angle = (float)std::rand() / RAND_MAX * pi * 2;
		int speedSelection = std::rand();
		float speed = minVelocity + (maxVelocity - minVelocity) * speedSelection / RAND_MAX;
		p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

		p.acceleration = sf::Vector2f(std::cos(angle) * acceleration, std::sin(angle) * acceleration);

		int lifetimeSelection = std::rand();
		sf::Time lifetime = minLifetime + (maxLifetime - minLifetime) * ((float)lifetimeSelection / RAND_MAX);
		p.lifetime = p.constLifetime = lifetime;

		int distanceSelection = std::rand();
		float distance = minDistance + (maxDistance - minDistance) * distanceSelection / RAND_MAX;
		sf::Vector2f rcpos = centralPosition + sf::Vector2f(std::cos(angle) * distance, std::sin(angle) * distance);

		constexpr float second3angle = pi * 2 / 3;
		constexpr float third3angle = pi * 4 / 3;
		float rotation = (float)std::rand() / RAND_MAX * second3angle;

		m_vertices[0 + i * 3].position = rcpos + particleRadius * sf::Vector2f(std::cos(rotation), std::sin(rotation));
		m_vertices[1 + i * 3].position = rcpos + particleRadius * sf::Vector2f(std::cos(rotation + second3angle), std::sin(rotation + second3angle));
		m_vertices[2 + i * 3].position = rcpos + particleRadius * sf::Vector2f(std::cos(rotation + third3angle), std::sin(rotation + third3angle));

		float colorSelection = (float)std::rand() / RAND_MAX;
		if (colorSelection < secondColorRatio)
			m_vertices[0 + i * 3].color = m_vertices[1 + i * 3].color = m_vertices[2 + i * 3].color = (sf::Color)secondColor;
		else
			m_vertices[0 + i * 3].color = m_vertices[1 + i * 3].color = m_vertices[2 + i * 3].color = (sf::Color)firstColor;
		m_vertices[0 + i * 3].color.a = m_vertices[1 + i * 3].color.a = m_vertices[2 + i * 3].color.a = 255;


	}
}
void ParticleSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();
	states.texture = nullptr;
	target.draw(m_vertices.data(), m_vertices.size(), sf::Triangles, states);
}

}
