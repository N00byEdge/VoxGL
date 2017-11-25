#pragma once

#include <memory>

#include "SFML/Window/Event.hpp"

struct Game;

struct GameState;

struct FrameRet {
	FrameRet() = default;
	FrameRet(FrameRet &other) : exitState(other.exitState), exitGame(other.exitGame), nextState(std::move(other.nextState)) { }
	bool exitState = false, exitGame = false;
	std::unique_ptr<GameState> nextState{nullptr};
};

struct GameState {
public:
	virtual ~GameState() { };
	virtual FrameRet frame(Game *, sf::Window &, float timeDelta) = 0;
	virtual void handleEvent(Game *, sf::Window &, sf::Event &) = 0;
protected:
	Game *g;
	FrameRet fr;
	GameState(Game *g) : g(g) { };
private:
};
