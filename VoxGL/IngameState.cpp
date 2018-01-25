#include "IngameState.hpp"

#include "GL/glew.h"

#include "SFML/OpenGL.hpp"
#include "SFML/Window.hpp"

#include "Game.hpp"
#include "Mesh.hpp"
#include "Transform.hpp"
#include <cmath>

#include "Blocks.hpp"
#include "Textures.hpp"
#include "PerlinNoise.hpp"

#include <cmath>

IngameState::IngameState(Game *g, sf::Window &window): GameState(g), w(&position, 0) {
	if(!releaseCursor) sf::Mouse::setPosition({(int)window.getSize().x / 2, (int)window.getSize().y / 2}, window);
}

FrameRet IngameState::frame(Game *g, sf::Window &window, float timeDelta) {
	auto cam = camera(position, lookX, lookZ, -g->fov() * 2, (float)window.getSize().x/window.getSize().y, g->renderDistance());
	auto lookDir = glm::vec3{0, 0, 0};

	auto dt = std::min(timeDelta, 0.1f);

	if (length(velocity) < 0.5f) velocity *= 0.0f;
	else velocity -= (drag(velocity, 0.00000001f) * dt) + (0.3f * (velocity / length(velocity)));

	glm::vec3 acceleration{0, 0, 0};

	if (!releaseCursor) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			acceleration += forward(lookX, 0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			acceleration -= forward(lookX, 0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			acceleration += forward(lookX - acos(-1.0f) / 2, 0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			acceleration += forward(lookX + acos(-1.0f) / 2, 0);

		if (acceleration != glm::vec3{ 0, 0, 0 }) {
			acceleration /= sqrt(acceleration.x * acceleration.x + acceleration.y * acceleration.y);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			acceleration.z += .5f;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
			acceleration.z -= .5f;
	}
	
	velocity += acceleration * timeDelta * 50.0f;

	position += velocity * timeDelta;

	if(isVerbose) std::cerr << position.x << " " << position.y << " " << position.z << " " << lookX << " " << lookZ << " " << length(velocity) << "\n";

	glClearColor((float)62/255, (float)215/255, (float)249/255, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	assert(glGetError() == GL_NO_ERROR);

	g->shaderBasic.update(transform(), cam);
	g->shaderBasic.bind();
	w.draw(timeDelta, cam, g->shaderBasic);

	if (auto[block, side, x, y, z, dist] = w.raycast(position, forward(lookX, lookZ), 8.0f); block) {
		glColor3f(0, 0, 0);
		glBegin(GL_LINES);
		glVertex3f(x - 0.001f, y - 0.001f, z - 0.001f);
		glVertex3f(x - 0.001f, y - 0.001f, z + 1.001f);
		glVertex3f(x - 0.001f, y - 0.001f, z - 0.001f);
		glVertex3f(x - 0.001f, y + 1.001f, z - 0.001f);
		glVertex3f(x - 0.001f, y - 0.001f, z - 0.001f);
		glVertex3f(x + 1.001f, y - 0.001f, z - 0.001f);
		glVertex3f(x - 0.001f, y - 0.001f, z + 1.001f);
		glVertex3f(x - 0.001f, y + 1.001f, z + 1.001f);
		glVertex3f(x - 0.001f, y - 0.001f, z + 1.001f);
		glVertex3f(x + 1.001f, y - 0.001f, z + 1.001f);
		glVertex3f(x - 0.001f, y + 1.001f, z - 0.001f);
		glVertex3f(x - 0.001f, y + 1.001f, z + 1.001f);
		glVertex3f(x - 0.001f, y + 1.001f, z - 0.001f);
		glVertex3f(x + 1.001f, y + 1.001f, z - 0.001f);
		glVertex3f(x + 1.001f, y - 0.001f, z - 0.001f);
		glVertex3f(x + 1.001f, y + 1.001f, z - 0.001f);
		glVertex3f(x + 1.001f, y - 0.001f, z - 0.001f);
		glVertex3f(x + 1.001f, y - 0.001f, z + 1.001f);
		glVertex3f(x + 1.001f, y + 1.001f, z - 0.001f);
		glVertex3f(x + 1.001f, y + 1.001f, z + 1.001f);
		glVertex3f(x + 1.001f, y - 0.001f, z + 1.001f);
		glVertex3f(x + 1.001f, y + 1.001f, z + 1.001f);
		glVertex3f(x - 0.001f, y + 1.001f, z + 1.001f);
		glVertex3f(x + 1.001f, y + 1.001f, z + 1.001f);
		glEnd();
	}

	{
		glUseProgram(0);
		glDisable(GL_DEPTH_TEST);
		glColor3f(1.f, 1.f, 1.f);
		auto xFactor = (float)window.getSize().y / window.getSize().x;
		glBegin(GL_QUADS);
		glVertex2f(-0.003 * xFactor, -0.003f);
		glVertex2f(0.003f * xFactor, -0.003f);
		glVertex2f(0.003f * xFactor, 0.003f);
		glVertex2f(-0.003f * xFactor, 0.003f);
		glEnd();
	}

	window.setMouseCursorVisible(releaseCursor);
	window.display();

	auto ret = fr;
	return ret;
}

void IngameState::handleEvent(Game *g, sf::Window &window, sf::Event &event) {
	switch (event.type) {
	case sf::Event::Closed:
		fr.exitGame = true;
		break;
	case sf::Event::Resized:
		glViewport(0, 0, event.size.width, event.size.height);
		if(!releaseCursor) sf::Mouse::setPosition({ (int)window.getSize().x / 2, (int)window.getSize().y / 2 }, window);
		break;
	case sf::Event::KeyPressed:
		switch (event.key.code) {
		case sf::Keyboard::Key::Escape:
			fr.exitState = true;
			break;
		case sf::Keyboard::Key::LAlt:
			if (window.hasFocus()) {
				releaseCursor = !releaseCursor;
				if (!releaseCursor) sf::Mouse::setPosition({ (int)window.getSize().x / 2, (int)window.getSize().y / 2 }, window);
			}
			break;
		case sf::Keyboard::Key::F3:
			isVerbose = !isVerbose;
		}
		break;
	case sf::Event::MouseMoved:
		if (!releaseCursor) {
			auto mousePos = sf::Mouse::getPosition(window);
			auto delta = mousePos - sf::Vector2i{ (int)window.getSize().x / 2, (int)window.getSize().y / 2 };

			lookX -= (float)delta.x / 1000.0f;

			lookX = posfmod(lookX, 3.1415f * 2.0f);

			lookZ -= (float)delta.y / 1000.0f;

			if (lookZ > 3.1415f / 2)
				lookZ = 3.1415f / 2;
			if (lookZ < -3.1415f / 2)
				lookZ = -3.1415f / 2;

			sf::Mouse::setPosition({ (int)window.getSize().x / 2, (int)window.getSize().y / 2 }, window);
		}
		break;
	case sf::Event::MouseButtonPressed:
		if (!releaseCursor) {
			if (auto[block, side, x, y, z, dist] = w.raycast(position, forward(lookX, lookZ), 8.0f); block) {
				block->remove(x, y, z, w);
				w.getChunkAtBlock<false>(x, y, z)->regenerateChunkMesh<false>(Chunk::decomposeChunkFromBlock(x), Chunk::decomposeChunkFromBlock(y), Chunk::decomposeChunkFromBlock(z), &w);
			}
		}
		break;
	}
}

float IngameState::pressure(float h) {
	// float p = (2.571610703826904e6f - (1.172433375403285e2f * h) + (0.001781877002114f * h * h) - (9.027612713185521e-9f * h * h * h));
	float p = (2.574681483305359e6f - (1.173851852033113f * h) + (1.784060919377516e-7f * h * h) - (9.038819697211279e-15f * h * h * h));
	p = std::max(0.0f, p);
	return p;
}

glm::vec3 IngameState::drag(const glm::vec3 &velocity, float coefficient) {
	return (velocity * length(velocity) * (coefficient * pressure(position.z))) + (coefficient * pressure(position.z) * 100.0f * velocity);
}
