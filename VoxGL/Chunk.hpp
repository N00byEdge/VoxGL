#pragma once

#include "Block.hpp"

#include <array>
#include <memory>
#include <mutex>

constexpr BlockCoord chunkCoordBits = 4;
constexpr BlockCoord chunkSize = 1 << chunkCoordBits;
constexpr BlockCoord chunkBlockMask = chunkSize - 1;
constexpr BlockCoord chunkLocMask = ~chunkBlockMask;

struct World;
enum struct PerlinInstance;

using Lookup = std::unordered_map<long long, float>;

const static auto forEachBlock = [&](auto callable) {
	for (BlockCoord x = 0; x < chunkSize; ++x)
		for (BlockCoord y = 0; y < chunkSize; ++y)
			for (BlockCoord z = 0; z < chunkSize; ++z)
				callable(x, y, z);
};

struct Chunk {
	// Construct a chunk with the chunk coordinates x, y, z
	Chunk(BlockCoord x, BlockCoord y, BlockCoord z, World *);

	// x, y, z, relative to chunk, Returns block inside the chunk. No chunk bounds check, use for fast access and that only.
	Block *blockAt(BlockCoord x, BlockCoord y, BlockCoord z);

	// x, y, z relative to chunk. Returns block inside the chunk. If outside of the chunk, returns nullptr
	Block *blockAtSafe(BlockCoord x, BlockCoord y, BlockCoord z);

	// x, y, z, relative to the chunk. If not inside the chunk, it will use World * and ask it for the block. cx, cy, cz starting block coordinates for the chunk
	template <bool alreadyHasMutex> Block *blockAtExternal(BlockCoord x, BlockCoord y, BlockCoord z, BlockCoord cx, BlockCoord cy, BlockCoord cz, World *);

	// Regenerates the mesh for the chunk. x, y, z are chunk coordinates (not block coordinates)
	template <bool alreadyHasMutex> void regenerateChunkMesh(BlockCoord x, BlockCoord y, BlockCoord z, World *);

	// Draws the chunk mesh
	void draw(float deltaT, const glm::vec3 &pos);

	static constexpr BlockCoord decomposeLocalBlockFromBlock(BlockCoord bc);
	static constexpr BlockCoord decomposeChunkFromBlock(BlockCoord bc);
	static constexpr std::pair <BlockCoord, BlockCoord> decomposeBlockPos(BlockCoord bc);

	static float getBlerpWorldgenVal(BlockCoord x, BlockCoord y, World *world, PerlinInstance instance);
	static constexpr BlockCoord blockHeight(float height);

	std::array <std::unique_ptr<Block>, chunkSize * chunkSize * chunkSize> blocks;

	std::mutex chunkMeshMutex;
private:
	std::unique_ptr<Mesh> chunkMesh;
	std::unique_ptr<MeshData> chunkMeshData;
	static int blockPos(BlockCoord x, BlockCoord y, BlockCoord z);
};

template <bool alreadyHasMutex>
Block *Chunk::blockAtExternal(BlockCoord x, BlockCoord y, BlockCoord z, BlockCoord cx, BlockCoord cy, BlockCoord cz, World *world) {
	if (0 <= x && x < chunkSize && 0 <= y && y < chunkSize && 0 <= z && z < chunkSize) return blockAt(x, y, z);
	return world->blockAt<alreadyHasMutex>(x + cx, y + cy, z + cz);
}

template <bool alreadyHasMutex>
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
			auto block = blockAtExternal<alreadyHasMutex>(x + 1, y, z, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Right, b, *meshData);

			block = blockAtExternal<alreadyHasMutex>(x - 1, y, z, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Left, b, *meshData);

			block = blockAtExternal<alreadyHasMutex>(x, y + 1, z, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Back, b, *meshData);

			block = blockAtExternal<alreadyHasMutex>(x, y - 1, z, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Front, b, *meshData);

			block = blockAtExternal<alreadyHasMutex>(x, y, z + 1, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Top, b, *meshData);

			block = blockAtExternal<alreadyHasMutex>(x, y, z - 1, wx, wy, wz, world);
			if (!block || !block->isSolid())
				addFace(wx + x, wy + y, wz + z, BlockSide::Bottom, b, *meshData);
		}
	});

	chunkMeshData = std::move(meshData);
	//chunkMesh = std::make_unique<Mesh>(meshData->vertices, meshData->indices);
}

constexpr BlockCoord Chunk::decomposeLocalBlockFromBlock(BlockCoord bc) {
	return bc & chunkBlockMask;
}

constexpr BlockCoord Chunk::decomposeChunkFromBlock(BlockCoord bc) {
	return (bc & chunkLocMask) >> chunkCoordBits;
}

constexpr std::pair<BlockCoord, BlockCoord> Chunk::decomposeBlockPos(BlockCoord bc) {
	return { decomposeLocalBlockFromBlock(bc), decomposeChunkFromBlock(bc) };
}

constexpr BlockCoord Chunk::blockHeight(float height) {
	return (BlockCoord)(60.0f + height * 10.0f);
}
