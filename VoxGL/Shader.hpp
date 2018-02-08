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
  void bind() const;
  void update(const glm::mat4 &transform, const glm::mat4 &camera, const glm::vec3 &renderTranslation = {.0f, .0f, .0f}) const;
private:
  enum {
    UTransform,
    UBlocktraslation,

    UNum
  };

  const GLuint program;
  const std::vector<GLuint> shaders;
  const std::array<GLuint, UNum> uniforms;
};
