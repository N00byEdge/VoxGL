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

void Chunk::draw(float deltaT, const glm::vec3 &worldPos) {
	if (std::lock_guard<std::mutex> lck(chunkMeshMutex); chunkMeshData) {
		chunkMesh = std::make_unique<Mesh>(chunkMeshData->vertices, chunkMeshData->indices);
		chunkMeshData.release();
	}

	if(chunkMesh)
		chunkMesh->draw();

	if constexpr(isDebugging) {
		glLineWidth(2.5f);
		glColor3f(1.f, 1.f, 1.f);

		glBegin(GL_LINES);
		glVertex3f(worldPos.x * chunkSize, worldPos.y * chunkSize, worldPos.z * chunkSize);
		glVertex3f((worldPos.x + 1.f) * chunkSize, (worldPos.y + 1.f) * chunkSize, (worldPos.z + 1.f) * chunkSize);
		glEnd();
	}
}

int Chunk::blockPos(BlockCoord x, BlockCoord y, BlockCoord z) {
	return x + (y << chunkCoordBits) + (z << chunkCoordBits * 2);
}
