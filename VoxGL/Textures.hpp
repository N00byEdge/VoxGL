#pragma once

#include "TextureAtlas.hpp"

#include <vector>
#include <memory>
#include <map>

extern std::unique_ptr <TextureAtlas> blockTextures, itemTextures;

static std::vector <std::string> textureNames {
	"None",
	"GrassSide",
	"GrassTop",
	"Dirt",
};

extern std::map <std::string, int> textureIDLookup;

struct Texture {
	Texture(int id) : id(id) { }
	operator int() { return id; }
	int id;
};

struct BlockTexture {
	BlockTexture(Texture top, Texture bottom, Texture front, Texture back, Texture left, Texture right) : top(top), bottom(bottom), front(front), back(back), left(left), right(right) {}
	Texture top, bottom, front, back, left, right;
};

struct ItemTexture {
	Texture itemText;
};

void loadTextures(std::string texturePath);
void unloadTextures();
