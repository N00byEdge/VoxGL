#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <vector>
#include <memory>

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
  Mesh(Mesh &&other) noexcept;
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
  std::unique_ptr<GLuint[]> vertexArrayBuffers = std::make_unique<GLuint[]>(VbNum);
};
