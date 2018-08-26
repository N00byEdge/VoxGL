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

  void emplaceGameState(std::unique_ptr<GameState> &&state);

  static Game *gameState;

  Config::Option<std::pair<unsigned, unsigned>> windowSize = MakeOption<std::pair<unsigned, unsigned>>(1920, 1080);
  Config::Option<bool> fullscreen                          = MakeOption<bool>(0);
  Config::Option<bool> vsync                               = MakeOption<bool>(0);
  Config::Option<int> antialiasingLevel                    = MakeOption<int>(0);
  Config::Option<float> fov                                = MakeOption<float>(100.0f);
  Config::Option<float> maxFps                             = MakeOption<float>(-1.0f);
  Config::Option<float> renderDistance                     = MakeOption<float>(1000.0f);
  Config::Option<std::string> shaderPath                   = MakeOption<std::string>("./assets/res/");
  Config::Option<std::string> texturePath                  = MakeOption<std::string>("./assets/textures/");
private:

  Config conf;

  // Use values from conf only after it has been loaded

  std::vector<std::unique_ptr<GameState>> states;
  std::chrono::duration<float, std::milli> minFrameTime{}, lastFrameTime{0.1f};

  void setFrameTime(float maxFps);

public:
  sf::Window gameWindow;
  void const *postWindowInit;

  // Shaders
  Shader shaderBasic;
};
