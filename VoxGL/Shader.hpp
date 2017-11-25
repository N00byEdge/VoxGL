#pragma once

#include <vector>
#include <array>

#include <GL/glew.h>
#include "glm/glm.hpp"

#define LDSHADER(shadername) Shader shadername(#shadername)

struct Camera;

struct Shader {
	Shader(std::string path);
	~Shader();
	void bind();
	void update(const glm::mat4 &transform, const glm::mat4 &camera);
private:
	enum {
		U_TRANSFORM,

		U_NUM
	};

	const GLuint program;
	const std::vector <GLuint> shaders;
	const std::array <GLuint, U_NUM> uniforms;
};
