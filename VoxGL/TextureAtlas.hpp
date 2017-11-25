#pragma once

#include "GL/glew.h"

#include <string>

constexpr int textureLength = 16; // 16 * 16 texture atlases

struct TextureAtlas {
	using TextureID = size_t;

	TextureAtlas(const std::string &filename);
	~TextureAtlas();

	void bind() const;
private:
	GLuint textureHandle;
};
