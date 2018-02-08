#pragma once

#include "GameState.hpp"

struct MenuState: public GameState {
  explicit MenuState(Game *g);
  ~MenuState() override = default;
  FrameRet frame(Game *, sf::Window &, float timeDelta) override;
  void handleEvent(Game *, sf::Window &, sf::Event &) override;
};
