#include "Textures.hpp"

#include "Blocks.hpp"
#include "Game.hpp"

std::unique_ptr <TextureAtlas> blockTextures, itemTextures;

std::map <std::string, int> textureIDLookup;

void loadTextures(std::string texturePath) {
	blockTextures = std::make_unique<TextureAtlas>(texturePath + "Blocks.png");
	itemTextures = std::make_unique<TextureAtlas>(texturePath + "Items.png");

	int cnt = 0;
	for (auto &name : textureNames)
		textureIDLookup[name] = cnt++;
}

void unloadTextures() {
	blockTextures.release();
	itemTextures.release();
}
