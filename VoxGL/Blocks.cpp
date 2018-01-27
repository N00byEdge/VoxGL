#include "Blocks.hpp"

const static std::vector <BlockCoord> vx{ { 1 },{ -1 } };
const static std::vector <std::tuple<BlockCoord, BlockCoord>> vdxy{ { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
const static std::vector <std::tuple<BlockCoord, BlockCoord, BlockCoord>> vdxyz{ { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };

BlockHandle registerBlockFactory(const std::string &name, BlockFactory factory) {
	stringToHandle[name] = (BlockHandle)blockFactoryHandles.size();
	blockFactoryHandles.push_back(factory);
	handleToString.push_back(name);

	return (BlockHandle)blockFactoryHandles.size() - 1;
}

std::unique_ptr<Block> createBlock(int factoryHandle, int x, int y, int z, World *w) {
	try {
		return blockFactoryHandles[factoryHandle](x, y, z, w);
	}
	catch (std::exception &e) {
		return nullptr;
	}
}

const static std::string invalid = "Invalid";

const std::string &getBlockName(int blockHandle) {
	try {
		return handleToString[blockHandle];
	}
	catch (std::exception &e) {
		return invalid;
	}
}

BlockHandle getBlockHandle(std::string blockName) {
	auto it = stringToHandle.find(blockName);
	if (it != stringToHandle.end())
		return it->second;

	return -1;
}
