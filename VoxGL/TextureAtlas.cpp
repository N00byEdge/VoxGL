#include "TextureAtlas.hpp"
#include "stb_image.h"

#include <iostream>
#include <cassert>

#include "Game.hpp"

int TextureSize = 0;

TextureAtlas::TextureAtlas(const std::string &filename) {
  int width, height, channels;
  auto const data = stbi_load(filename.c_str(), &width, &height, &channels, 4);

  if(!data)
    std::cerr << "Unable to load texture " << filename << std::endl;
  else {
    glGenTextures(1, &textureHandle);
    if constexpr(isDebugging)
      assert(glGetError() == GL_NO_ERROR);

    glBindTexture(GL_TEXTURE_2D, textureHandle);
    if constexpr(isDebugging)
      assert(glGetError() == GL_NO_ERROR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    if constexpr(isDebugging)
      assert(glGetError() == GL_NO_ERROR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if constexpr(isDebugging)
      assert(glGetError() == GL_NO_ERROR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    if constexpr(isDebugging)
      assert(glGetError() == GL_NO_ERROR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    if constexpr(isDebugging)
      assert(glGetError() == GL_NO_ERROR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    if constexpr(isDebugging)
      assert(glGetError() == GL_NO_ERROR);

    stbi_image_free(data);
  }
}

TextureAtlas::~TextureAtlas() {
  glDeleteTextures(1, &textureHandle);

  if constexpr(isDebugging)
    assert(glGetError() == GL_NO_ERROR);
}

void TextureAtlas::bind() const {
  glBindTexture(GL_TEXTURE_2D, textureHandle);

  if constexpr(isDebugging)
    assert(glGetError() == GL_NO_ERROR);
}
