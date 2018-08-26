#include "IngameState.hpp"

#include "GL/glew.h"

#include "SFML/OpenGL.hpp"
#include "SFML/Window.hpp"

#include "Game.hpp"
#include "Transform.hpp"
#include "PerlinNoise.hpp"
#include "Maths.hpp"

#include <cmath>
#include <iostream>

IngameState::IngameState(Game *g, sf::Window &window): GameState(g), w(std::make_unique<World>(&position, 0)) {
  if(!releaseCursor)
    sf::Mouse::setPosition({static_cast<int>(window.getSize().x) / 2, static_cast<int>(window.getSize().y) / 2}, window);
}

IngameState::~IngameState() {
  std::thread unloadThread = std::thread(unload, std::move(w));
  unloadThread.detach();
}

FrameRet IngameState::frame(Game *g, sf::Window &window, float const timeDelta) {
  auto const cam = Camera(position, lookX, lookZ, -g->fov() * 2, static_cast<float>(window.getSize().x) / window.getSize().y,
                          g->renderDistance());
  auto lookDir = glm::vec3{0, 0, 0};

  auto const dt = std::min(timeDelta, 0.1f);

  if(length(velocity) < 0.5f)
    velocity *= 0.0f;
  else
    velocity -= (drag(velocity, 0.00000001f) * dt) + (0.3f * (velocity / length(velocity)));

  glm::vec3 acceleration{0, 0, 0};

  if(!releaseCursor) {
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
      acceleration += Forward(lookX, 0);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
      acceleration -= Forward(lookX, 0);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
      acceleration += Forward(lookX - acos(-1.0f) / 2, 0);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
      acceleration += Forward(lookX + acos(-1.0f) / 2, 0);

    if(acceleration != glm::vec3{0, 0, 0}) { acceleration /= sqrt(acceleration.x * acceleration.x + acceleration.y * acceleration.y); }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
      acceleration.z += 1.f;
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
      acceleration.z -= 1.f;
  }

  velocity += acceleration * timeDelta * 250.0f;

  position += velocity * timeDelta;

  if(isVerbose)
    std::cerr << position.x << " " << position.y << " " << position.z << " " << lookX << " " << lookZ << " " << length(velocity) << "\n";

  glClearColor(static_cast<float>(62) / 255, static_cast<float>(215) / 255, static_cast<float>(249) / 255, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  assert(glGetError() == GL_NO_ERROR);

  g->shaderBasic.update(Transform(), cam);
  g->shaderBasic.bind();
  w->draw(timeDelta, cam, g->shaderBasic);

  if([[maybe_unused]] auto [block, side, x, y, z, dist] = w->raycast(position, Forward(lookX, lookZ), 8.0f); block) {
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
    auto const xFactor = static_cast<float>(window.getSize().y) / window.getSize().x;
    glBegin(GL_QUADS);
    glVertex2f(-0.003f * xFactor, -0.003f);
    glVertex2f(0.003f * xFactor, -0.003f);
    glVertex2f(0.003f * xFactor, 0.003f);
    glVertex2f(-0.003f * xFactor, 0.003f);
    glEnd();
  }

  window.setMouseCursorVisible(releaseCursor);
  window.display();

  auto ret = std::move(fr);
  return ret;
}

void IngameState::handleEvent(Game *g, sf::Window &window, sf::Event &event) {
  switch(event.type) {
  case sf::Event::Closed:
    fr.exitGame = true;
    break;
  case sf::Event::Resized:
    glViewport(0, 0, event.size.width, event.size.height);
    if(!releaseCursor)
      sf::Mouse::setPosition({static_cast<int>(window.getSize().x) / 2, static_cast<int>(window.getSize().y) / 2}, window);
    break;
  case sf::Event::KeyPressed:
    switch(event.key.code) {
    case sf::Keyboard::Key::Escape:
      fr.exitState = true;
      break;
    case sf::Keyboard::Key::LAlt:
      if(window.hasFocus()) {
        releaseCursor = !releaseCursor;
        if(!releaseCursor)
          sf::Mouse::setPosition({static_cast<int>(window.getSize().x) / 2, static_cast<int>(window.getSize().y) / 2}, window);
      }
      break;
    case sf::Keyboard::Key::F3:
      isVerbose = !isVerbose;
    }
    break;
  case sf::Event::MouseMoved:
    if(!releaseCursor) {
      auto const mousePos = sf::Mouse::getPosition(window);
      auto const delta    = mousePos - sf::Vector2i{static_cast<int>(window.getSize().x) / 2, static_cast<int>(window.getSize().y) / 2};

      lookX -= static_cast<float>(delta.x) / 1000.0f;

      lookX = Posfmod(lookX, 3.1415f * 2.0f);

      lookZ -= static_cast<float>(delta.y) / 1000.0f;

      if(lookZ > 3.1415f / 2)
        lookZ = 3.1415f / 2;
      if(lookZ < -3.1415f / 2)
        lookZ = -3.1415f / 2;

      sf::Mouse::setPosition({static_cast<int>(window.getSize().x) / 2, static_cast<int>(window.getSize().y) / 2}, window);
    }
    break;
  case sf::Event::MouseButtonPressed:
    if(!releaseCursor) {
      if([[maybe_unused]] auto [block, side, x, y, z, dist] = w->raycast(position, Forward(lookX, lookZ), 8.0f); block) {
        auto const xx                                       = Chunk::decomposeBlockPos(x);
        auto const yy                                       = Chunk::decomposeBlockPos(y);
        auto const zz                                       = Chunk::decomposeBlockPos(z);
        auto c                                              = w->getChunk<false>(xx.second, yy.second, zz.second);
        c->removeBlockAt(xx.first, yy.first, zz.first);
      }
    }
    break;
  }
}

float IngameState::pressure(float const h) const {
  auto p = (2.574681483305359e6f - (1.173851852033113f * h) + (1.784060919377516e-7f * h * h) - (9.038819697211279e-15f * h * h * h));
  p      = std::max(0.0f, p);
  return p;
}

glm::vec3 IngameState::drag(const glm::vec3 &velocity, float const coefficient) const {
  return (velocity * length(velocity) * (coefficient * pressure(position.z / 10000))) + (coefficient * pressure(position.z / 10000) * 100.0f * velocity);
}
