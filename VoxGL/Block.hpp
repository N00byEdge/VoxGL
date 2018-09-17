#pragma once

struct Game;
struct World;
struct BlockTexture;

using BlockHandle = int;
using BlockCoord = int;

extern BlockHandle InvalidHandle;
extern BlockHandle DirtHandle;
extern BlockHandle GrassHandle;
extern BlockHandle StoneHandle;
extern BlockHandle SandHandle;

#include "BlockFaceMesh.hpp"
#include "Mesh.hpp"

enum struct BlockType {
  Dirt,
  Grass,
  Stone,
  Sand,
};

struct Block {
  virtual bool isSolid() { return false; }
  virtual void remove(BlockCoord x, BlockCoord y, BlockCoord z, World &w);
  virtual void destroy(BlockCoord x, BlockCoord y, BlockCoord z, World &w);
  virtual MeshData getMesh(int x, int y, int z, BlockSide blockSides) = 0;
  virtual void onBreak(World &, BlockCoord x, BlockCoord y, BlockCoord z) = 0;
  virtual ~Block() = default;
};

template<BlockType Type>
struct BasicBlock: public Block {
  BasicBlock();
  ~BasicBlock() override;
  bool isSolid() override;
  MeshData getMesh(BlockCoord x, BlockCoord y, BlockCoord z, BlockSide blockSides) override;
  void onBreak(World &, BlockCoord x, BlockCoord y, BlockCoord z) override;
};
