#include "Block.hpp"

#include "Blocks.hpp"

#include "Game.hpp"
#include "Textures.hpp"
#include "Transform.hpp"
#include "BlockFaceMesh.hpp"
#include "World.hpp"
#include "Chunk.hpp"
#include "Item.hpp"

std::unordered_map<std::string, BlockHandle> StringToHandle;
std::vector<BlockFactory> BlockFactoryHandles;
std::vector<std::string> HandleToString;

BlockHandle InvalidHandle = RegisterBlockFactory("invalid", [](BlockCoord x, BlockCoord y, BlockCoord z, World *w) { return nullptr; });
BlockHandle DirtHandle    = RegisterBlockFactory("dirt", [](BlockCoord x, BlockCoord y, BlockCoord z, World *w) {
  return std::make_unique<BasicBlock<BlockType::Dirt>>();
});
BlockHandle GrassHandle = RegisterBlockFactory("grass", [](BlockCoord x, BlockCoord y, BlockCoord z, World *w) {
  return std::make_unique<BasicBlock<BlockType::Grass>>();
});
BlockHandle StoneHandle = RegisterBlockFactory("stone", [](BlockCoord x, BlockCoord y, BlockCoord z, World *w) {
  return std::make_unique<BasicBlock<BlockType::Stone>>();
});
BlockHandle SandHandle = RegisterBlockFactory("sand", [](BlockCoord x, BlockCoord y, BlockCoord z, World *w) {
  return std::make_unique<BasicBlock<BlockType::Sand>>();
});

namespace Textures {
  Texture NoTexture = Texture{ 0 };
  Texture GrassSide = Texture{ 1 };
  Texture GrassTop  = Texture{ 2 };
  Texture Dirt      = Texture{ 3 };
  Texture Stone     = Texture{ 4 };
  Texture Sand      = Texture{ 5 };
}

template<BlockType Type>
BasicBlock<Type>::BasicBlock() = default;

template<BlockType Type>
BasicBlock<Type>::~BasicBlock() = default;

template<BlockType Type>
bool BasicBlock<Type>::isSolid() { return true; }

template<BlockType Type>
MeshData BasicBlock<Type>::getMesh(BlockCoord x, BlockCoord y, BlockCoord z, BlockSide blockSides) {
  const auto texture = [&]() {
    if constexpr(Type == BlockType::Dirt)
      return BlockTexture{Textures::Dirt};
    else if constexpr(Type == BlockType::Grass)
      return BlockTexture{Textures::GrassTop, Textures::Dirt, Textures::GrassSide};
    else if constexpr(Type == BlockType::Stone)
      return BlockTexture{Textures::Stone};
    else if constexpr(Type == BlockType::Sand)
      return BlockTexture{Textures::Sand};
  }();

  auto textId = 0;

  switch(blockSides) {
  case BlockSide::Top:
    textId = texture.top.id;
    break;
  case BlockSide::Bottom:
    textId = texture.bottom.id;
    break;
  case BlockSide::Front:
    textId = texture.front.id;
    break;
  case BlockSide::Back:
    textId = texture.back.id;
    break;
  case BlockSide::Left:
    textId = texture.left.id;
    break;
  case BlockSide::Right:
    textId = texture.right.id;
    break;
  default:
    break;
  }

  return BasicBlockFaceMesh({x, y, z}, textId, blockSides);
}

template<BlockType Type>
void BasicBlock<Type>::onBreak(World &game, BlockCoord x, BlockCoord y, BlockCoord z) {
  game.addItem(std::make_unique<BlockItem<BasicBlock<Type>>>(), x, y, z);
}

void Block::remove(BlockCoord x, BlockCoord y, BlockCoord z, World &w) {
  std::lock_guard<std::mutex> l(w.chunkMutex);
  auto [bx, cx] = Chunk::decomposeBlockPos(x);
  auto [by, cy] = Chunk::decomposeBlockPos(y);
  auto [bz, cz] = Chunk::decomposeBlockPos(z);
  auto c        = w.getChunk<true>(cx, cy, cz);
  c->blocks[Chunk::blockPos(bx, by, bz)] = std::unique_ptr<Block>{};
}

void Block::destroy(BlockCoord x, BlockCoord y, BlockCoord z, World &w) {
  // Remove block and spawn block item
  remove(x, y, z, w);
}
