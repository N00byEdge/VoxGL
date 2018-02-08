#include "Shader.hpp"

#include <iostream>
#include <string>
#include <memory>

#include "Game.hpp"
#include "Transform.hpp"
#include "Mesh.hpp"

#include <GL/gl.h>

template<bool IsProgram>
void CheckShaderError(GLuint const shader, GLuint const flag, const std::string &errorPrefix = "") {
  auto success = 0;
  GLchar error[1024]{};

  (IsProgram ? glGetProgramiv : glGetShaderiv)(shader, flag, &success);
  if(!success)
    (IsProgram ? glGetProgramInfoLog : glGetShaderInfoLog)(shader, sizeof(error), nullptr, error), std::cerr << errorPrefix << error << std
        ::endl;
}

GLuint MakeShader(const std::string &shaderSource, GLenum const shaderType) {
  auto const shader = glCreateShader(shaderType);

  auto fptr     = std::make_unique<const char *>(shaderSource.c_str());
  auto filesize = std::make_unique<GLint>(static_cast<GLint>(shaderSource.size()));

  glShaderSource(shader, 1, fptr.get(), filesize.get());
  glCompileShader(shader);

  CheckShaderError<false>(shader, GL_COMPILE_STATUS, "Shader linking error: ");

  assert(glGetError() == GL_NO_ERROR);

  return shader;
}

Shader::Shader(const std::string &vertexShader, const std::string &fragmentShader) :
  program(glCreateProgram()),

  shaders([&]() {
    std::vector<GLuint> shaders;
    shaders.push_back(MakeShader(vertexShader, GL_VERTEX_SHADER));
    shaders.push_back(MakeShader(fragmentShader, GL_FRAGMENT_SHADER));

    for(const auto &s: shaders)
      glAttachShader(program, s);

    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "textCoord");

    glLinkProgram(program);
    CheckShaderError<true>(program, GL_LINK_STATUS, "Shader linking error: ");

    glValidateProgram(program);
    CheckShaderError<true>(program, GL_VALIDATE_STATUS, "Shader validation error: ");

    return shaders;
  }()),

  uniforms([&]() {
    std::array<GLuint, UNum> arr{};
    arr[UTransform]       = glGetUniformLocation(program, "transform");
    arr[UBlocktraslation] = glGetUniformLocation(program, "blockTraslation");
    return arr;
  }()) {
  assert(glGetError() == GL_NO_ERROR);
}

Shader::~Shader() {
  for(const auto &s: shaders) {
    glDetachShader(program, s);
    glDeleteShader(s);
  }
  glDeleteProgram(program);
}

void Shader::bind() const {
  glUseProgram(program);
}

void Shader::update(const glm::mat4 &transform, const glm::mat4 &camera, const glm::vec3 &renderTranslation) const {
  glUseProgram(program);
  auto result = camera * transform;

  glUniformMatrix4fv(uniforms[UTransform], 1, GL_FALSE, &result[0][0]);
  glUniform3fv(uniforms[UBlocktraslation], 1, &renderTranslation.x);
}
