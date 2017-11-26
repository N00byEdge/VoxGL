#include "Chunk.hpp"

#include "Blocks.hpp"
#include "BlockFaceMesh.hpp"
#include "World.hpp"

#include "Maths.hpp"

#include <future>

const static auto forEachBlock = [&](auto callable) {
	for (BlockCoord x = 0; x < chunkSize; ++x)
		for (BlockCoord y = 0; y < chunkSize; ++y)
			for (BlockCoord z = 0; z < chunkSize; ++z)
				callable(x, y, z);
};

float Chunk::getBlerpWorldgenVal(BlockCoord x, BlockCoord y, World *world, PerlinInstance instance) {
	auto num = getPrecision(instance).first;
	auto f00 = getWorldgenVal(x, y, *world, instance);
	auto f10 = getWorldgenVal(x + num, y, *world, instance);
	auto f01 = getWorldgenVal(x, y + num, *world, instance);
	auto f11 = getWorldgenVal(x + num, y + num, *world, instance);
	
	x = posmod(x, getPrecision(instance).first);
	y = posmod(y, getPrecision(instance).first);

	auto h = blerp(f00, f01, f10, f11, (float)x / (num - 1), (float)y / (num - 1));
	h = blerp(f00, f01, f10, f11, (float)x / (num - 1), (float)y / (num - 1));
	return h;
}

Chunk::Chunk(BlockCoord x, BlockCoord y, BlockCoord z, World *world) {
	x *= chunkSize, y *= chunkSize, z *= chunkSize;
	for (BlockCoord bx = 0; bx < chunkSize; ++bx) {
		for (BlockCoord by = 0; by < chunkSize; ++by) {
			auto bh = blockHeight(getBlerpWorldgenVal(x + bx, y + by, world, PerlinInstance::Height));
			//auto bh00 = getWorldgenVal(x + bx, y + by, *world, PerlinInstance::Height);
			//auto bh10 = getWorldgenVal(x + bx + heightPrecision.num, y + by, *world, PerlinInstance::Height);
			//auto bh01 = getWorldgenVal(x + bx, y + by + 8, *world, PerlinInstance::Height);
			//auto bh11 = getWorldgenVal(x + bx + 8, y + by + 8, *world, PerlinInstance::Height);
			//auto bh = blockHeight(blerp(bh00, bh01, bh10, bh11, (float)(((bx % 8) + 8) % 8) / 7, (float)(((by % 8) + 8) % 8) / 7));
			for (BlockCoord bz = 0; bz < chunkSize; ++bz) {
				if (bz + z < bh)
					blocks[blockPos(bx, by, bz)] = std::make_unique<BasicBlock>(bz + z == bh - 1 ? static_cast<BlockID>(Blocks::Grass) : static_cast<BlockID>(Blocks::Dirt), x + bx, y + by, z + bz);
			}
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

Block *Chunk::blockAtExternal(BlockCoord x, BlockCoord y, BlockCoord z, BlockCoord cx, BlockCoord cy, BlockCoord cz, World *world) {
	if (0 <= x && x < chunkSize && 0 <= y && y < chunkSize && 0 <= z && z < chunkSize) return blockAt(x, y, z);
	return world->blockAt(x + cx, y + cy, z + cz);
}

void Chunk::draw(float deltaT, const glm::vec3 &worldPos) {
	if (std::lock_guard<std::mutex> lck(chunkMeshMutex); chunkMeshData) {
		chunkMesh = std::make_unique<Mesh>(chunkMeshData->vertices, chunkMeshData->indices);
		chunkMeshData.release();
	}

	if(chunkMesh)
		chunkMesh->draw();
}

void Chunk::regenerateChunkMesh(BlockCoord wx, BlockCoord wy, BlockCoord wz, World *world) {
	wx *= chunkSize, wy *= chunkSize, wz *= chunkSize;
	auto meshData = std::make_unique<MeshData>();

	forEachBlock([&](BlockCoord x, BlockCoord y, BlockCoord z) {
		auto b = blockAtSafe(x, y, z);

		const static auto addFace = [](BlockCoord bx, BlockCoord by, BlockCoord bz, BlockSide face, Block *b, MeshData &meshData) {
			auto md = b->getMesh(bx, by, bz, face);

			for (auto &ind : md.indices)
				meshData.indices.push_back((GLuint)(ind + meshData.vertices.size()));
			for (auto &vert : md.vertices)
				meshData.vertices.push_back(vert);
		};

		if (b) {
			auto block = blockAtExternal(x + 1, y, z, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Right, b, *meshData);

			block = blockAtExternal(x - 1, y, z, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Left, b, *meshData);

			block = blockAtExternal(x, y + 1, z, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Back, b, *meshData);

			block = blockAtExternal(x, y - 1, z, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Front, b, *meshData);

			block = blockAtExternal(x, y, z + 1, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Top, b, *meshData);

			block = blockAtExternal(x, y, z - 1, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Bottom, b, *meshData);
		}
	});

	chunkMeshData = std::move(meshData);
	//chunkMesh = std::make_unique<Mesh>(meshData->vertices, meshData->indices);
}

int Chunk::blockPos(BlockCoord x, BlockCoord y, BlockCoord z) {
	return x + (y << chunkCoordBits) + (z << chunkCoordBits * 2);
}
