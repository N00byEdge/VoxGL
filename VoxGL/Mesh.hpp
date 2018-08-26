#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <vector>

struct MeshPoint {
  using WorldPos = glm::vec3;
  using TextPos = glm::vec2;
  WorldPos loc;
  TextPos textPoint;
};

struct MeshData {
  std::vector<MeshPoint> vertices;
  std::vector<unsigned> indices;
};

struct Mesh {
  Mesh(std::vector<MeshPoint> const &vertices, std::vector<unsigned> const &indices);
  Mesh(Mesh &) = delete;
  Mesh(Mesh &&other) noexcept;
  Mesh &operator=(Mesh &) = delete;
  Mesh &operator=(Mesh &&other) noexcept;
  ~Mesh();
  void draw() const;
private:
  enum {
    VbPosition,
    VbTextcoord,
    VbIndices,

    VbNum
  };

  unsigned int count;
  GLuint vertexArrayObject;
  GLuint vertexArrayBuffers[3];
};
