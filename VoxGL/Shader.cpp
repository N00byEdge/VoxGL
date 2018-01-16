#include "Shader.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include "Game.hpp"
#include "Transform.hpp"
#include "Mesh.hpp"

#include <GL/gl.h>

template <bool isProgram>
void checkShaderError(GLuint shader, GLuint flag, const std::string errorPrefix = "") {
	GLint success = 0;
	GLchar error[1024]{};

	(isProgram ? glGetProgramiv : glGetShaderiv)(shader, flag, &success);
	if(!success)
		(isProgram ? glGetProgramInfoLog : glGetShaderInfoLog)(shader, sizeof(error), NULL, error), std::cerr << errorPrefix << error << std::endl;
}

GLuint makeShader(const std::string &shaderSource, GLenum shaderType) {
	GLuint shader = glCreateShader(shaderType);

	auto fptr = std::make_unique<const char *>(shaderSource.c_str());
	auto filesize = std::make_unique<GLint>((GLint)shaderSource.size());

	glShaderSource(shader, 1, fptr.get(), filesize.get());
	glCompileShader(shader);

	checkShaderError<false>(shader, GL_COMPILE_STATUS, "Shader linking error: ");

	assert(glGetError() == GL_NO_ERROR);

	return shader;
}

Shader::Shader(const std::string &vertexShader, const std::string &fragmentShader) : program(glCreateProgram()),

shaders([&]() {
	std::vector <GLuint> shaders;
	shaders.push_back(makeShader(vertexShader, GL_VERTEX_SHADER));
	shaders.push_back(makeShader(fragmentShader, GL_FRAGMENT_SHADER));

	for (const auto &s : shaders)
		glAttachShader(program, s);

	glBindAttribLocation(program, 0, "position");
	glBindAttribLocation(program, 1, "textCoord");

	glLinkProgram(program);
	checkShaderError<true>(program, GL_LINK_STATUS, "Shader linking error: ");

	glValidateProgram(program);
	checkShaderError<true>(program, GL_VALIDATE_STATUS, "Shader validation error: ");

	return shaders;
}()),

uniforms([&]() {
	std::array<GLuint, U_NUM> arr{};
	arr[U_TRANSFORM] = glGetUniformLocation(program, "transform");
	arr[U_BLOCKTRASLATION] = glGetUniformLocation(program, "blockTraslation");
	return arr;
}()) {
	assert(glGetError() == GL_NO_ERROR);
}

Shader::~Shader() {
	for (const auto &s : shaders) {
		glDetachShader(program, s);
		glDeleteShader(s);
	}
	glDeleteProgram(program);
}

void Shader::bind() {
	glUseProgram(program);
}

void Shader::update(const glm::mat4 &transform, const glm::mat4 &camera, const glm::vec3 &renderTranslation) {
	glm::mat4 result = camera * transform;
	
	glUniformMatrix4fv(uniforms[U_TRANSFORM], 1, GL_FALSE, &result[0][0]);
	glUniform3fv(uniforms[U_BLOCKTRASLATION], 1, &renderTranslation.x);
}
