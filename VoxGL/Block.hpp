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
  ~BasicBlock() final;
  bool isSolid() final;
  MeshData getMesh(BlockCoord x, BlockCoord y, BlockCoord z, BlockSide blockSides) final;
  void onBreak(World &, BlockCoord x, BlockCoord y, BlockCoord z) final;
};

using DirtBlock = BasicBlock<BlockType::Dirt>;
using GrassBlock = BasicBlock<BlockType::Grass>;
using StoneBlock = BasicBlock<BlockType::Stone>;
using SandBlock = BasicBlock<BlockType::Sand>;
