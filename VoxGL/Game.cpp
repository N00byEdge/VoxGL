#include "Game.hpp"

#include "MenuState.hpp"
#include "Shaders.hpp"
#include "Textures.hpp"

#include "GL/glew.h"

#include <thread>
#include <iostream>

Game *Game::gameState = nullptr;

Game::Game() :

  conf([&]() {
    Config conf("Config.txt");

    conf.ADDOPT(windowSize);
    conf.ADDOPT(fullscreen);
    conf.ADDOPT(antialiasingLevel);
    conf.ADDOPT(fov);
    conf.ADDOPT(maxFps);
    conf.ADDOPT(texturePath);
    conf.ADDOPT(renderDistance);
    conf.ADDOPT(vsync);

    conf.read();
    conf.write();
    return conf;
  }()),

  gameWindow(fullscreen() ? sf::VideoMode::getDesktopMode() : sf::VideoMode(windowSize().first, windowSize().second),
             "Fem",
             fullscreen() ? sf::Style::Fullscreen : sf::Style::Resize | sf::Style::Close,
             [&]() {
               sf::ContextSettings settings;

               settings.depthBits         = 24;
               settings.stencilBits       = 8;
               //settings.sRgbCapable       = false;
               settings.antialiasingLevel = antialiasingLevel();

               return settings;
             }()),

  postWindowInit([&]() {
    //glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    assert(glGetError() == GL_NO_ERROR);

    LoadTextures(texturePath());

    return nullptr;
  }()),

  shaderBasic(vsBasic, fsBasic) {

  shaderBasic.bind();

  gameState = this;
  setFrameTime(maxFps());
  gameWindow.setVerticalSyncEnabled(vsync());
  emplaceGameState(std::make_unique<MenuState>(this));

  static const auto frame = [&]() {
    auto status           = states.back()->frame(this, gameWindow, std::max(lastFrameTime.count(), minFrameTime.count()) / 1000.0f);

    if(status.exitState)
      states.pop_back();
    if(status.exitGame)
      states.clear();
    if(status.nextState)
      emplaceGameState(std::move(status.nextState));
  };

  while(!states.empty()) {
    auto const frameStart = std::chrono::high_resolution_clock::now();
    sf::Event event{};


    for(int i = 0; gameWindow.pollEvent(event) && i < 20; ++ i)
      if(!states.empty())
        states.back()->handleEvent(this, gameWindow, event);

    auto const lastState = states.back().get();
    if constexpr(isDebugging)
      frame();
    else {
      try { frame(); }
      catch(std::exception &e) {
        std::cout << "State " << lastState << " threw the exception " << e.what() << ".\n";
        std::cin.ignore();
        states.clear();
        emplaceGameState(std::make_unique<MenuState>(this));
      }
    }

    auto const frameEnd                                      = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::milli> const frameTime = frameEnd - frameStart;
    lastFrameTime                                            = frameTime;
    auto sleepFor                                            = minFrameTime - frameTime;
    //std::cout << "Sleeping for " << sleepFor.count() << " after a frame time of " << frameTime.count() << ".\n";
    if(sleepFor.count() > 0.0f)
      std::this_thread::sleep_for(sleepFor);
  }

  UnloadTextures();
}

void Game::emplaceGameState(std::unique_ptr<GameState> &&state) { states.emplace_back(std::forward<std::unique_ptr<GameState>>(state)); }

void Game::setFrameTime(float const maxFps) {
  if(maxFps > 0)
    minFrameTime = std::chrono::duration<float, std::milli>(1000.0f / (maxFps + 10));
  else
    minFrameTime = std::chrono::duration<float, std::milli>(0);
}
