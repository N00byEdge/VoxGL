#pragma once

#include "GameState.hpp"
#include "Game.hpp"

#include "World.hpp"
#include "Chunk.hpp"

#include "glm/glm.hpp"

struct IngameState: public GameState {
  IngameState(Game *, sf::Window &);
  ~IngameState() override;;
  FrameRet frame(Game *, sf::Window &, float timeDelta) override;
  void handleEvent(Game *, sf::Window &, sf::Event &) override;
  glm::vec3 position{0, 0, Chunk::blockHeight(1.f)};
  glm::vec3 velocity{0, 0, 0};
  float lookX        = .0f, lookZ = .0f;
  bool releaseCursor = isDebugging;
  bool isVerbose     = false;
  glm::vec3 drag(glm::vec3 const &velocity, float coefficient) const;
  float pressure(float h) const;

  //Initialize the world last.
  std::unique_ptr<World> w;
};
