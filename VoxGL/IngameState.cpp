#include "IngameState.hpp"

#include "GL/glew.h"

#include "SFML/OpenGL.hpp"
#include "SFML/Window.hpp"

#include "Game.hpp"
#include "Mesh.hpp"
#include "Transform.hpp"
#include <cmath>

//#include "BasicBlockMesh.hpp"

#include "Blocks.hpp"
#include "Textures.hpp"
#include "PerlinNoise.hpp"

#include <cmath>

IngameState::IngameState(Game *g, sf::Window &window): GameState(g), w(&position) {
	if(!releaseCursor) sf::Mouse::setPosition({(int)window.getSize().x / 2, (int)window.getSize().y / 2}, window);
}

FrameRet IngameState::frame(Game *g, sf::Window &window, float timeDelta) {
	auto cam = camera(position, lookX, lookZ, -g->fov() * 2, (float)window.getSize().x/window.getSize().y, g->renderDistance());
	auto lookDir = glm::vec3{0, 0, 0};

	momentum -= momentum * 0.3f * timeDelta;

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
			acceleration.z += 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
			acceleration.z -= 1;
	}
	
	momentum += acceleration * timeDelta * 5.0f;

	position += momentum * timeDelta;

	if(isVerbose) std::cerr << position.x << " " << position.y << " " << position.z << " " << lookX << " " << lookZ << "\n";

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	assert(glGetError() == GL_NO_ERROR);

	g->basicShader.update(transform(), cam);
	g->basicShader.bind();
	w.draw(timeDelta, cam, g->basicShader);

	//glLineWidth(2.5f);
	//glBegin(GL_LINES);
	//glVertex2f(0.0, 0.0);
	//glVertex2f(15, 15);
	//glEnd();

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

			lookX = fmod(lookX, 3.1415f * 2.0f);

			lookZ -= (float)delta.y / 1000.0f;

			if (lookZ > 3.1415f / 2)
				lookZ = 3.1415f / 2;
			if (lookZ < -3.1415f / 2)
				lookZ = -3.1415f / 2;

			sf::Mouse::setPosition({ (int)window.getSize().x / 2, (int)window.getSize().y / 2 }, window);
			break;
		}
	}
}
