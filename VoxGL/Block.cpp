#include "Block.hpp"

#include "Blocks.hpp"

#include "Game.hpp"
#include "Textures.hpp"
#include "Blocks.hpp"
#include "Transform.hpp"
#include "BlockFaceMesh.hpp"
#include "World.hpp"
#include "Chunk.hpp"

std::unordered_map<std::string, BlockHandle> stringToHandle;
std::vector <BlockFactory> blockFactoryHandles;
std::vector <std::string> handleToString;

auto dirtHandle  = registerBlockFactory("dirt",  [](BlockCoord x, BlockCoord y, BlockCoord z, World *w) { return std::make_unique<BasicBlock<BlockType::Dirt >>(x, y, z, w); });
auto grassHandle = registerBlockFactory("grass", [](BlockCoord x, BlockCoord y, BlockCoord z, World *w) { return std::make_unique<BasicBlock<BlockType::Grass>>(x, y, z, w); });
auto stoneHandle = registerBlockFactory("stone", [](BlockCoord x, BlockCoord y, BlockCoord z, World *w) { return std::make_unique<BasicBlock<BlockType::Stone>>(x, y, z, w); });

template <BlockType type>
BasicBlock<type>::BasicBlock(BlockCoord x, BlockCoord y, BlockCoord z, World *w) {

}

template <BlockType type>
BasicBlock<type>::~BasicBlock() {

}

template <BlockType type>
bool BasicBlock<type>::isSolid() {
	return true;
}

template <BlockType type>
MeshData BasicBlock<type>::getMesh(BlockCoord x, BlockCoord y, BlockCoord z, BlockSide blockSides) {
	const auto texture = [&]() {
		if constexpr(type == BlockType::Dirt)
			return BlockTexture{ 3, 3, 3, 3, 3, 3 };
		else if constexpr(type == BlockType::Grass)
			return BlockTexture{ 2, 3, 1, 1, 1, 1 };
		else if constexpr(type == BlockType::Stone)
			return BlockTexture{ 4, 4, 4, 4, 4, 4 };
	}();

	int textID = 0;

	switch (blockSides) {
	case BlockSide::Top:
		textID = texture.top.id;
		break;
	case BlockSide::Bottom:
		textID = texture.bottom.id;
		break;
	case BlockSide::Front:
		textID = texture.front.id;
		break;
	case BlockSide::Back:
		textID = texture.back.id;
		break;
	case BlockSide::Left:
		textID = texture.left.id;
		break;
	case BlockSide::Right:
		textID = texture.right.id;
		break;
	}

	return basicBlockFaceMesh({x, y, z}, textID, blockSides);
}

template <BlockType type>
void BasicBlock<type>::onBreak(Game &game, BlockCoord x, BlockCoord y, BlockCoord z) {

}

void Block::remove(BlockCoord x, BlockCoord y, BlockCoord z, World &w) {
	std::lock_guard<std::mutex> l(w.chunkMutex);
	auto[bx, cx] = Chunk::decomposeBlockPos(x);
	auto[by, cy] = Chunk::decomposeBlockPos(y);
	auto[bz, cz] = Chunk::decomposeBlockPos(z);
	auto c = w.getChunk<true>(cx, cy, cz);
	c->blocks[Chunk::blockPos(bx, by, bz)].release();
}

void Block::destroy(BlockCoord x, BlockCoord y, BlockCoord z, World &w) {
	// Remove block and spawn block item
	remove(x, y, z, w);
}
