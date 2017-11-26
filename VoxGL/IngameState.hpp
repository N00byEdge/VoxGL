#pragma once

#include "GameState.hpp"
#include "Game.hpp"

#include "World.hpp"

#include "glm/glm.hpp"

struct IngameState : public GameState {
	IngameState(Game *, sf::Window &);
	virtual ~IngameState() override { };
	virtual FrameRet frame(Game *, sf::Window &, float timeDelta) override;
	virtual void handleEvent(Game *, sf::Window &, sf::Event &) override;
	glm::vec3 position{ 0, 0, 82 };
	glm::vec3 momentum{ 0, 0, 0 };
	float lookX = .0f, lookZ = .0f;
	bool releaseCursor = isDebugging;
	bool isVerbose = false;

	// _Please_ initialize the world last.
	World w;
};
