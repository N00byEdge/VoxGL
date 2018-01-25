#pragma once

struct Game;
struct World;
struct BlockTexture;

using BlockHandle = int;
using BlockCoord = int;

#include "glm/glm.hpp"

#include "BlockFaceMesh.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"

enum struct BlockType {
	Dirt,
	Grass,
	Stone,

	Num_blocks
};

struct Block {
	virtual bool isSolid() { return false; }
	virtual void remove(BlockCoord x, BlockCoord y, BlockCoord z, World &w);
	virtual void destroy(BlockCoord x, BlockCoord y, BlockCoord z, World &w);
	virtual MeshData getMesh(int x, int y, int z, BlockSide blockSides) = 0;
	virtual void onBreak(Game &, BlockCoord x, BlockCoord y, BlockCoord z) = 0;
	virtual ~Block() { };
};

template <BlockType type>
struct BasicBlock : public Block {
	BasicBlock(BlockCoord x, BlockCoord y, BlockCoord z, World *w);
	virtual ~BasicBlock() override;
	virtual bool isSolid() override;
	virtual MeshData getMesh(BlockCoord x, BlockCoord y, BlockCoord z, BlockSide blockSides) override;
	virtual void onBreak(Game &, BlockCoord x, BlockCoord y, BlockCoord z) override;
};
