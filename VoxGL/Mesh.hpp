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
	std::vector <MeshPoint> vertices;
	std::vector <unsigned> indices;
};

struct Mesh {
	Mesh(const std::vector <MeshPoint> &vertices, const std::vector <unsigned> &indices);
	~Mesh();
	void draw() const;
private:
	enum {
		VB_POSITION,
		VB_TEXTCOORD,
		VB_INDICES,

		VB_NUM
	};
	
	const unsigned int count;
	GLuint vertexArrayObject;
	GLuint vertexArrayBuffers[VB_NUM];
};
