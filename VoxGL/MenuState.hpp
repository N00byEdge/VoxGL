#pragma once

#include "GameState.hpp"

struct MenuState : public GameState {
	MenuState(Game *g);
	virtual ~MenuState() override = default;
	virtual FrameRet frame(Game *, sf::Window &, float timeDelta) override;
	virtual void handleEvent(Game *, sf::Window &, sf::Event &) override;
};
