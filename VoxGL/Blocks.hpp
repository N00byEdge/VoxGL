#pragma once

struct Block;
struct BlockTexture;
struct World;

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>

#include "Block.hpp"

using BlockStorage = std::variant<
    std::unique_ptr<Block>
  , DirtBlock
  , GrassBlock
  , StoneBlock
  , SandBlock
>;

using BlockFactory = std::function<BlockStorage(BlockCoord, BlockCoord, BlockCoord, World *)>;
extern std::vector<BlockCoord> const  Vx;
extern std::vector<std::tuple<BlockCoord, BlockCoord>> const  Vdxy;
extern std::vector<std::tuple<BlockCoord, BlockCoord, BlockCoord>> const Vdxyz;

extern std::unordered_map<std::string, BlockHandle> StringToHandle;
extern std::vector<BlockFactory> BlockFactoryHandles;
extern std::vector<std::string> HandleToString;

BlockHandle RegisterBlockFactory(std::string const &&name, BlockFactory factory);

BlockStorage CreateBlock(int factoryHandle, int x, int y, int z, World *);
std::string const &GetBlockName(int blockHandle);
BlockHandle GetBlockHandle(std::string const &blockName);
