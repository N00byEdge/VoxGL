#pragma once

using BlockID = int;
using BlockCoord = int;

#include <vector>
#include <string>
#include <memory>

#include "Textures.hpp"

const static std::vector <std::string> blockNames {
	"Stone",
	"Dirt",
	"Grass",
};

enum struct Blocks : BlockID {
	Stone,
	Dirt,
	Grass,

	numBlocks
};

template <Blocks b>
constexpr BlockID blockID() {
	return static_cast<int>(b);
}

constexpr BlockID blockID(Blocks b) {
	return static_cast<BlockID>(b);
}

std::shared_ptr<BlockTexture> getBlockTexture(BlockID blockID);
