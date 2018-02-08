#pragma once

#include "Textures.hpp"
#include "Mesh.hpp"

enum struct BlockSide {
	None,
	Top,
	Bottom,
	Front,
	Back,
	Left,
	Right
};

MeshData BasicBlockFaceMesh(glm::vec3 blockPosition, int textureID, BlockSide side);
