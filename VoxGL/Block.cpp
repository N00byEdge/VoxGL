#include "Block.hpp"

#include "Blocks.hpp"

#include "Game.hpp"
#include "Textures.hpp"
#include "Blocks.hpp"
#include "Transform.hpp"
#include "BlockFaceMesh.hpp"

Block::~Block() {

}

BasicBlock::BasicBlock(BlockID id, BlockCoord x, BlockCoord y, BlockCoord z): id(id) {

}

BasicBlock::~BasicBlock() {
}

bool BasicBlock::isSolid() {
	return true;
}

MeshData BasicBlock::getMesh(BlockCoord x, BlockCoord y, BlockCoord z, BlockSide blockSides) {
	std::shared_ptr<BlockTexture> texture{getBlockTexture(id)};
	int textID = 0;

	if (texture) {
		switch (blockSides) {
		case BlockSide::Top:
			textID = texture->top.id;
			break;
		case BlockSide::Bottom:
			textID = texture->bottom.id;
			break;
		case BlockSide::Front:
			textID = texture->front.id;
			break;
		case BlockSide::Back:
			textID = texture->back.id;
			break;
		case BlockSide::Left:
			textID = texture->left.id;
			break;
		case BlockSide::Right:
			textID = texture->right.id;
			break;
		}
	}

	return basicBlockFaceMesh({x, y, z}, textID, blockSides);
}

void BasicBlock::onBreak(Game &game, BlockCoord x, BlockCoord y, BlockCoord z) {

}
