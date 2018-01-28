#include "Chunk.hpp"

#include "Blocks.hpp"
#include "BlockFaceMesh.hpp"
#include "World.hpp"
#include "Game.hpp"

#include "Maths.hpp"

#include <future>

float Chunk::getBlerpWorldgenVal(BlockCoord x, BlockCoord y, World *world, PerlinInstance instance) {
	auto num = getPrecision(instance).first;
	auto f00 = getWorldgenVal(x, y, *world, instance);
	auto f10 = getWorldgenVal(x + num, y, *world, instance);
	auto f01 = getWorldgenVal(x, y + num, *world, instance);
	auto f11 = getWorldgenVal(x + num, y + num, *world, instance);
	
	x = posmod(x, getPrecision(instance).first);
	y = posmod(y, getPrecision(instance).first);

	return blerp(f00, f01, f10, f11, (float)x / (num - 1), (float)y / (num - 1));
}

Chunk::Chunk(BlockCoord _x, BlockCoord _y, BlockCoord _z, World *world): w(*world), x(_x * chunkSize), y(_y * chunkSize), z(_z * chunkSize), cx(_x), cy(_y), cz(_z) {
	for (BlockCoord bx = 0; bx < chunkSize; ++bx) {
		for (BlockCoord by = 0; by < chunkSize; ++by) {
			auto bh = blockHeight(getBlerpWorldgenVal(x + bx, y + by, world, PerlinInstance::Height));
			for (BlockCoord bz = 0; bz < chunkSize; ++bz)
				if (bz + z < bh)
					blocks[blockPos(bx, by, bz)] = createBlock(bz + z == bh - 1, x + bx, y + by, z + bz, &w);
		}
	}
}

Block *Chunk::blockAt(BlockCoord x, BlockCoord y, BlockCoord z) {
	return blocks[blockPos(x, y, z)].get();
}

Block *Chunk::blockAtSafe(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (0 <= x && x < chunkSize && 0 <= y && y < chunkSize && 0 <= z && z < chunkSize)
		return blocks.at(blockPos(x, y, z)).get();
	return nullptr;
}

Block *Chunk::blockAtAdjacent(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (0 <= x && x < chunkSize && 0 <= y && y < chunkSize && 0 <= z && z < chunkSize)
		return blockAtSafe(x, y, z);
	else {
		if (chunkSize <= x) {
			if (auto c = std::get<0>(adjacentChunks).lock())
				return c->blockAtSafe(x - chunkSize, y, z);
		}
		else if (x < 0) {
			if (auto c = std::get<1>(adjacentChunks).lock())
				return c->blockAtSafe(x + chunkSize, y, z);
		}
		else if (chunkSize <= y) {
			if (auto c = std::get<2>(adjacentChunks).lock())
				return c->blockAtSafe(x, y - chunkSize, z);
		}
		else if (y < 0) {
			if (auto c = std::get<3>(adjacentChunks).lock())
				return c->blockAtSafe(x, y + chunkSize, z);
		}
		else if (chunkSize <= z) {
			if (auto c = std::get<4>(adjacentChunks).lock())
				return c->blockAtSafe(x, y, z - chunkSize);
		}
		else if (z < 0 && this->cz) {
			if (auto c = std::get<5>(adjacentChunks).lock())
				return c->blockAtSafe(x, y, z + chunkSize);
		}	
	}

	return nullptr;
}

const static auto addFace = [](BlockCoord bx, BlockCoord by, BlockCoord bz, BlockSide face, Block *b, MeshData &meshData) {
	auto md = b->getMesh(bx, by, bz, face);

	for (auto &ind : md.indices)
		meshData.indices.push_back((GLuint)(ind + meshData.vertices.size()));
	for (auto &vert : md.vertices)
		meshData.vertices.push_back(vert);
};

void Chunk::regenerateChunkMesh() {
	auto meshData = std::make_unique<MeshData>();

	forEachBlock([&](BlockCoord bx, BlockCoord by, BlockCoord bz) {
		auto b = blockAtSafe(bx, by, bz);

		if (b) {
			auto block = blockAtAdjacent(bx + 1, by, bz);
			if (!block || !block->isSolid())
				addFace(bx + x, by + y, bz + z, BlockSide::Right, b, *meshData);

			block = blockAtAdjacent(bx - 1, by, bz);
			if (!block || !block->isSolid())
				addFace(bx + x, by + y, bz + z, BlockSide::Left, b, *meshData);

			block = blockAtAdjacent(bx, by + 1, bz);
			if (!block || !block->isSolid())
				addFace(bx + x, by + y, bz + z, BlockSide::Back, b, *meshData);

			block = blockAtAdjacent(bx, by - 1, bz);
			if (!block || !block->isSolid())
				addFace(bx + x, by + y, bz + z, BlockSide::Front, b, *meshData);

			block = blockAtAdjacent(bx, by, bz + 1);
			if (!block || !block->isSolid())
				addFace(bx + x, by + y, bz + z, BlockSide::Top, b, *meshData);

			block = blockAtAdjacent(bx, by, bz - 1);
			if (!block || !block->isSolid())
				addFace(bx + x, by + y, bz + z, BlockSide::Bottom, b, *meshData);
		}
	});

	{
		std::lock_guard<std::mutex> meshLock(chunkMeshMutex);
		chunkMeshData = std::move(meshData);
	}
}

void Chunk::draw(float deltaT, const glm::vec3 &worldPos) {
	if (std::lock_guard<std::mutex> lck(chunkMeshMutex); chunkMeshData) {
		chunkMesh = std::make_unique<Mesh>(chunkMeshData->vertices, chunkMeshData->indices);
		chunkMeshData.release();
	}

	if(chunkMesh)
		chunkMesh->draw();
}

void Chunk::onAdjacentChunkLoad(BlockCoord relX, BlockCoord relY, BlockCoord relZ, std::weak_ptr<Chunk> wp) {
	if (relX == 1)
		std::get<0>(adjacentChunks) = wp;
	else if (relX == -1)
		std::get<1>(adjacentChunks) = wp;
	else if (relY == 1)
		std::get<2>(adjacentChunks) = wp;
	else if (relY == -1)
		std::get<3>(adjacentChunks) = wp;
	else if (relZ == 1)
		std::get<4>(adjacentChunks) = wp;
	else if (relZ == -1)
		std::get<5>(adjacentChunks) = wp;

	if (auto nAdjacent = getAdjacentChunks().size(); nAdjacent == 6ull || (nAdjacent >= 5ull && !z))
		regenerateChunkMesh();
}

std::vector<std::shared_ptr<Chunk>> Chunk::getAdjacentChunks() {
	std::vector<std::shared_ptr<Chunk>> result;

	for (auto &chunk : adjacentChunks)
		if(auto sharedChunk = chunk.lock())
			result.push_back(std::move(sharedChunk));

	return result;
}

int Chunk::blockPos(BlockCoord x, BlockCoord y, BlockCoord z) {
	return x + (y << chunkCoordBits) + (z << chunkCoordBits * 2);
}

void Chunk::removeBlockAt(BlockCoord _x, BlockCoord _y, BlockCoord _z) {
	auto bp = blockPos(_x, _y, _z);
	blocks[bp]->remove(x + _x, y + _y, z + _z, w);
	blocks[bp].release();
	regenerateChunkMesh();

	if (_x == chunkSize - 1) {
		if (auto c = std::get<0>(adjacentChunks).lock())
			c->regenerateChunkMesh();
	}
	if (_x == 0) {
		if (auto c = std::get<1>(adjacentChunks).lock())
			c->regenerateChunkMesh();
	}
	if (_y == chunkSize - 1) {
		if (auto c = std::get<2>(adjacentChunks).lock())
			c->regenerateChunkMesh();
	}
	if (_y == 0) {
		if (auto c = std::get<3>(adjacentChunks).lock())
			c->regenerateChunkMesh();
	}
	if (_z == chunkSize - 1) {
		if (auto c = std::get<4>(adjacentChunks).lock())
			c->regenerateChunkMesh();
	}
	if (_z == 0) {
		if (auto c = std::get<5>(adjacentChunks).lock())
			c->regenerateChunkMesh();
	}
}
