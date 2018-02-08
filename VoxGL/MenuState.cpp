#include "MenuState.hpp"

#include "IngameState.hpp"

#include <GL/glew.h>

#include "SFML/Window.hpp"
#include "SFML/OpenGL.hpp"

#include <gl/GL.h>

#include "Game.hpp"

MenuState::MenuState(Game *g): GameState(g) {}

FrameRet MenuState::frame(Game *g, sf::Window &window, float timeDelta) {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  window.setMouseCursorVisible(true);

  window.display();
  auto ret = std::move(fr);
  return ret;
}

void MenuState::handleEvent(Game *g, sf::Window &window, sf::Event &event) {
  switch(event.type) {
  case sf::Event::Closed:
    fr.exitGame = true;
    break;
  case sf::Event::Resized:
    glViewport(0, 0, event.size.width, event.size.height);
    break;
  case sf::Event::KeyPressed:
    switch(event.key.code) {
    case sf::Keyboard::Key::Return:
      fr.nextState = std::make_unique<IngameState>(g, window);
      break;
    case sf::Keyboard::Escape:
      fr.exitState = true;
      break;
    }
    break;
  }
}
