#pragma once

#include "TextureAtlas.hpp"

#include <vector>
#include <memory>
#include <map>

extern std::unique_ptr<TextureAtlas> BlockTextures, ItemTextures;

static std::vector<std::string> TextureNames{
  "None",
  "GrassSide",
  "GrassTop",
  "Dirt",
};

extern std::map<std::string, int> TextureIdLookup;

struct Texture {
  explicit Texture(int const id) : id(id) { }
  explicit operator int() const { return id; }
  int id;
};

struct BlockTexture {
  BlockTexture(Texture const top, Texture const bottom, Texture const front, Texture const back, Texture const left, Texture const right) :
    top(top), bottom(bottom), front(front), back(back), left(left), right(right) {}

  explicit BlockTexture(Texture const allSides): top(allSides), bottom(allSides), front(allSides), back(allSides), left(allSides),
                                                 right(allSides) {}

  BlockTexture(Texture const top, Texture const bottom, Texture const sides) : top(top), bottom(bottom), front(sides), back(sides),
                                                                               left(sides), right(sides) {}

  BlockTexture(Texture const topAndBottom, Texture const sides) : top(topAndBottom), bottom(topAndBottom), front(sides), back(sides),
                                                                  left(sides), right(sides) {}

  Texture top, bottom, front, back, left, right;
};

struct ItemTexture {
  explicit ItemTexture(Texture const &itemText) : itemText{itemText} {}

  Texture itemText;
};

void LoadTextures(std::string texturePath);
void UnloadTextures();
