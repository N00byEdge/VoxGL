#include "Game.hpp"

#include "MenuState.hpp"

#include <thread>

#include "GL/glew.h"
#include "Textures.hpp"

Game *Game::gameState = nullptr;

#define LOADT(text) text.loadFromFile(texturePath() + #text + ".png");

Game::Game() :

conf([&]() {
	Config conf("Config.txt");

	conf.ADDOPT(windowSize);
	conf.ADDOPT(fullscreen);
	conf.ADDOPT(antialiasingLevel);
	conf.ADDOPT(fov);
	conf.ADDOPT(maxFPS);
	conf.ADDOPT(shaderPath);
	conf.ADDOPT(texturePath);
	conf.ADDOPT(renderDistance);
	conf.ADDOPT(vsync);

	conf.read();
	conf.write();
	return conf;
}()),

gameWindow(fullscreen() ? sf::VideoMode::getDesktopMode() : sf::VideoMode(windowSize().first, windowSize().second), "Fem", fullscreen() ? sf::Style::Fullscreen : sf::Style::Resize | sf::Style::Close, [&]() {
	sf::ContextSettings settings;

	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.sRgbCapable = false;
	settings.antialiasingLevel = antialiasingLevel();

	return settings;
}()),

postWindowInit([&]() {
	//glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);

	assert(glGetError() == GL_NO_ERROR);

	loadTextures(texturePath());

	return nullptr;
}()),

basicShader(shaderPath() + "basicShader") {

	basicShader.bind();

	gameState = this;
	setFrameTime(maxFPS());
	gameWindow.setVerticalSyncEnabled(vsync());
	pushGameState(std::make_unique<MenuState>(this));

	static const auto frame = [&]() {
		auto status = states.back()->frame(this, gameWindow, std::max(lastFrameTime.count(), minFrameTime.count()) / 1000.0f);

		if (status.exitState)
			states.pop_back();
		if (status.exitGame)
			states.clear();
		if (status.nextState)
			pushGameState(std::move(status.nextState));
	};

	while (states.size()) {
		auto frameStart = std::chrono::high_resolution_clock::now();
		sf::Event event;

		while (gameWindow.pollEvent(event)) {
			if(states.size())
				states.back()->handleEvent(this, gameWindow, event);
		}
		auto lastState = states.back().get();
		if constexpr(isDebugging) frame();
		else {
			try {
				frame();
			}
			catch (std::exception &e) {
				std::cout << "State " << lastState << " threw the exception " << e.what() << ".\n";
				std::cin.ignore();
				states.clear();
				pushGameState(std::make_unique<MenuState>(this));
			}
		}

		auto frameEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float, std::milli> frameTime = frameEnd - frameStart;
		lastFrameTime = frameTime;
		auto sleepFor = minFrameTime - frameTime;
		if(sleepFor.count() > 0.0f)
			std::this_thread::sleep_for(sleepFor);
	}

	unloadTextures();
}

void Game::pushGameState(std::unique_ptr<GameState> state) {
	states.push_back(std::move(state));
}

void Game::setFrameTime(float maxFPS) {
	if (maxFPS > 0)
		minFrameTime = std::chrono::duration<float, std::milli>(1000.0f / (maxFPS + 10));
	else
		minFrameTime = std::chrono::duration<float, std::milli>(0);
}
