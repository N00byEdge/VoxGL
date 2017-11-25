#pragma once

struct Game;

#include "glm/glm.hpp"

#include "Shader.hpp"
#include "Blocks.hpp"

struct BlockTexture;

#include "BlockFaceMesh.hpp"
#include "Mesh.hpp"

struct Block {
	virtual bool isSolid() { return false; }
	virtual MeshData getMesh(int x, int y, int z, BlockSide blockSides) = 0;
	virtual void onBreak(Game &, BlockCoord x, BlockCoord y, BlockCoord z) = 0;
	virtual ~Block() = 0;
};

struct BasicBlock : public Block {
	BasicBlock(BlockID id, BlockCoord x, BlockCoord y, BlockCoord z);
	virtual ~BasicBlock() override;
	virtual bool isSolid() override;
	virtual MeshData getMesh(BlockCoord x, BlockCoord y, BlockCoord z, BlockSide blockSides) override;
	virtual void onBreak(Game &, BlockCoord x, BlockCoord y, BlockCoord z) override;
protected:
	int id;
};
