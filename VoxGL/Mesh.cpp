#include "Mesh.hpp"

#include "GL/glew.h"
#include "cstring"
#include "Game.hpp"

Mesh::Mesh(std::vector<MeshPoint> const &vertices, std::vector<unsigned> const &indices) :
  count(static_cast<unsigned int>(indices.size())), vertexArrayObject{}, vertexArrayBuffers{} {
  if(!count)
    return;

  std::vector<MeshPoint::WorldPos> locVerts(vertices.size());
  for(size_t i  = 0; i < vertices.size(); ++i)
    locVerts[i] = vertices[i].loc;
  std::vector<MeshPoint::TextPos> textVerts(vertices.size());
  for(size_t i   = 0; i < vertices.size(); ++i)
    textVerts[i] = vertices[i].textPoint;

  // @TODO: Make callable from any thread!
  auto err = glGetError();
  assert(glGetError() == GL_NO_ERROR);

  glGenVertexArrays(VbNum, &vertexArrayObject);
  glBindVertexArray(vertexArrayObject);
  glGenBuffers(VbNum, vertexArrayBuffers);

  assert(glGetError() == GL_NO_ERROR);

  glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffers[VbPosition]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(locVerts[0]), locVerts.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(VbPosition);
  glVertexAttribPointer(VbPosition, sizeof(locVerts[0]) / sizeof(locVerts[0].x), GL_FLOAT, GL_FALSE, 0, nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffers[VbTextcoord]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(textVerts[0]), textVerts.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(VbTextcoord);
  glVertexAttribPointer(VbTextcoord, sizeof(textVerts[0]) / sizeof(textVerts[0].x), GL_FLOAT, GL_FALSE, 0, nullptr);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexArrayBuffers[VbIndices]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);

  assert(glGetError() == GL_NO_ERROR);

  glBindVertexArray(0);
}

Mesh::Mesh(Mesh &&other) noexcept: count{other.count}, vertexArrayObject{other.vertexArrayObject},
                                   vertexArrayBuffers{} {
  std::memcpy(vertexArrayBuffers, other.vertexArrayBuffers, VbNum * sizeof(GLuint));
}

Mesh &Mesh::operator=(Mesh &&other) noexcept {
  count             = other.count;
  vertexArrayObject = other.vertexArrayObject;
  std::memcpy(vertexArrayBuffers, other.vertexArrayBuffers, VbNum * sizeof(GLuint));

  return *this;
}

Mesh::~Mesh() {
  if(count)
    glDeleteVertexArrays(VbNum, &vertexArrayObject);
}

void Mesh::draw() const {
  if(!count)
    return;

  glBindVertexArray(vertexArrayObject);

  glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
}
