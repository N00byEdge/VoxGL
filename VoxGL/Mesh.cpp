#include "Mesh.hpp"

#include <iostream>
#include <typeinfo>

#include "GL/glew.h"

#include "Game.hpp"

#include "SFML/Graphics/Texture.hpp"

Mesh::Mesh(const std::vector <MeshPoint> &vertices, const std::vector<unsigned> &indices): count((unsigned int) indices.size()) {
	std::vector <MeshPoint::WorldPos> locVerts(vertices.size());
	for (size_t i = 0; i < vertices.size(); ++i) locVerts[i] = vertices[i].loc;
	std::vector <MeshPoint::TextPos> textVerts(vertices.size());
	for (size_t i = 0; i < vertices.size(); ++i) textVerts[i] = vertices[i].textPoint;

	// TODO: Make callable from any thread!

	auto err = glGetError();
	assert(glGetError() == GL_NO_ERROR);

	glGenVertexArrays(VB_NUM, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
	glGenBuffers(VB_NUM, vertexArrayBuffers);

	assert(glGetError() == GL_NO_ERROR);

	glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffers[VB_POSITION]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(locVerts[0]), locVerts.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(VB_POSITION);
	glVertexAttribPointer(VB_POSITION, sizeof(locVerts[0]) / sizeof(locVerts[0].x), GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffers[VB_TEXTCOORD]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(textVerts[0]), textVerts.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(VB_TEXTCOORD);
	glVertexAttribPointer(VB_TEXTCOORD, sizeof(textVerts[0]) / sizeof(textVerts[0].x), GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexArrayBuffers[VB_INDICES]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);

	assert(glGetError() == GL_NO_ERROR);

	glBindVertexArray(0);
}

Mesh::~Mesh() {
	glDeleteVertexArrays(VB_NUM, &vertexArrayObject);
}

void Mesh::draw() const {
	glBindVertexArray(vertexArrayObject);

	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}
