#pragma once

#include "Textures.hpp"
#include "Mesh.hpp"

enum struct BlockSide {
	Top,
	Bottom,
	Front,
	Back,
	Left,
	Right
};

MeshData basicBlockFaceMesh(glm::vec3 blockPosition, int textureID, BlockSide side);
