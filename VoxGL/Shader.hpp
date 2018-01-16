#pragma once

#include <vector>
#include <array>

#include <GL/glew.h>
#include "glm/glm.hpp"

#define LDSHADER(shadername) Shader shadername(#shadername)

struct Camera;

struct Shader {
	Shader(const std::string &vertexShader, const std::string &fragmentShader);
	~Shader();
	void bind();
	void update(const glm::mat4 &transform, const glm::mat4 &camera, const glm::vec3 &renderTranslation = {.0f, .0f, .0f});
private:
	enum {
		U_TRANSFORM,
		U_BLOCKTRASLATION,

		U_NUM
	};

	const GLuint program;
	const std::vector <GLuint> shaders;
	const std::array <GLuint, U_NUM> uniforms;
};
