#pragma once

#include "GL/glew.h"

#include <string>

constexpr int TextureLength = 16; // 16 * 16 texture atlases

struct TextureAtlas {
  using TextureId = size_t;

  explicit TextureAtlas(const std::string &filename);
  ~TextureAtlas();

  void bind() const;
private:
  GLuint textureHandle;
};
