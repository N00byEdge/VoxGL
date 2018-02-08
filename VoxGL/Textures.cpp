#include "Textures.hpp"

#include "Blocks.hpp"
#include "Game.hpp"

std::unique_ptr<TextureAtlas> BlockTextures, ItemTextures;

std::map<std::string, int> TextureIdLookup;

void LoadTextures(std::string const texturePath) {
  BlockTextures = std::make_unique<TextureAtlas>(texturePath + "Blocks.png");
  ItemTextures  = std::make_unique<TextureAtlas>(texturePath + "Items.png");

  auto cnt = 0;
  for(auto &name: TextureNames)
    TextureIdLookup[name] = cnt++;
}

void UnloadTextures() {
  BlockTextures.release();
  ItemTextures.release();
}
