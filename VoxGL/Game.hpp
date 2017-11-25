#pragma once

#include <chrono>

#include "Config.hpp"
#include "GameState.hpp"

#include "SFML/Graphics.hpp"
#include "Shader.hpp"

#ifdef _DEBUG
constexpr bool isDebugging = true;
#else
constexpr bool isDebugging = false;
#endif // _DEBUG



struct Game {
	Game();

	void pushGameState(std::unique_ptr<GameState> state);
	
	static Game *gameState;

	Config::Option<std::pair<unsigned, unsigned>> windowSize = makeOption<std::pair<unsigned, unsigned>>(1920, 1080);
	Config::Option<bool> fullscreen = makeOption<bool>(0);
	Config::Option<bool> vsync = makeOption<bool>(0);
	Config::Option<int> antialiasingLevel = makeOption<int>(0);
	Config::Option<float> fov = makeOption<float>(100.0f);
	Config::Option<float> maxFPS = makeOption<float>(-1.0f);
	Config::Option<float> renderDistance = makeOption<float>(1000.0f);
	Config::Option<std::string> shaderPath = makeOption<std::string>("./assets/res/");
	Config::Option<std::string> texturePath = makeOption<std::string>("./assets/textures/");
private:

	Config conf;

	// Use values from conf only after it has been loaded
	;

	std::vector <std::unique_ptr<GameState>> states;
	std::chrono::duration<float, std::milli> minFrameTime, lastFrameTime{0.1f};

	void setFrameTime(float maxFPS);

public:
	sf::Window gameWindow;
	const void *postWindowInit;

	// Shaders
	Shader basicShader;
};
