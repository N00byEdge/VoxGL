#pragma once

struct Block;
struct BlockTexture;
struct World;

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

#include "Block.hpp"

using BlockFactory = std::function<std::unique_ptr<Block>(BlockCoord, BlockCoord, BlockCoord, World *)>;
const extern std::vector <BlockCoord> vx;
const extern std::vector <std::tuple<BlockCoord, BlockCoord>> vdxy;
const extern std::vector <std::tuple<BlockCoord, BlockCoord, BlockCoord>> vdxyz;

extern std::unordered_map<std::string, BlockHandle> stringToHandle;
extern std::vector <BlockFactory> blockFactoryHandles;
extern std::vector <std::string> handleToString;

BlockHandle registerBlockFactory(const std::string &name, BlockFactory factory);

std::unique_ptr<Block> createBlock(int factoryHandle, int x, int y, int z, World *);
const std::string &getBlockName(int blockHandle);
BlockHandle getBlockHandle(std::string blockName);
