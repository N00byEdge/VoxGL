#pragma once

#include <vector>
#include <array>

#include <GL/glew.h>
#include "glm/glm.hpp"

struct Camera;

struct Shader {
  Shader(std::string_view vertexShader, std::string_view fragmentShader);
  ~Shader();
  void bind() const;
  void update(const glm::mat4 &transform, const glm::mat4 &camera, const glm::ivec3 &renderTranslation = {0, 0, 0}) const;
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
