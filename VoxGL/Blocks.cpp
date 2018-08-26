#include "Blocks.hpp"

std::vector<BlockCoord> const Vx{{1}, {-1}};
std::vector<std::tuple<BlockCoord, BlockCoord>> const Vdxy{{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
std::vector<std::tuple<BlockCoord, BlockCoord, BlockCoord>> const Vdxyz{
  {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}
};

BlockHandle RegisterBlockFactory(std::string const &&name, BlockFactory factory) {
  StringToHandle[name] = static_cast<BlockHandle>(BlockFactoryHandles.size());
  BlockFactoryHandles.emplace_back(factory);
  HandleToString.emplace_back(name);

  return static_cast<BlockHandle>(BlockFactoryHandles.size() - 1);
}

std::unique_ptr<Block> CreateBlock(int const factoryHandle, int const x, int const y, int const z, World *w) {
  try { return BlockFactoryHandles[factoryHandle](x, y, z, w); }
  catch(std::exception &) { return nullptr; }
}

static std::string const Invalid = "Invalid";

std::string const &GetBlockName(int const blockHandle) {
  try { return HandleToString[blockHandle]; }
  catch(std::exception &) { return Invalid; }
}

BlockHandle GetBlockHandle(std::string const &blockName) {
  auto const it = StringToHandle.find(blockName);
  if(it != StringToHandle.end())
    return it->second;

  return -1;
}
