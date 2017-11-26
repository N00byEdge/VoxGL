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

struct Chunk {
	// Construct a chunk with the chunk coordinates x, y, z
	Chunk(BlockCoord x, BlockCoord y, BlockCoord z, World *);

	// x, y, z, relative to chunk, Returns block inside the chunk. No chunk bounds check, use for fast access and that only.
	Block *blockAt(BlockCoord x, BlockCoord y, BlockCoord z);

	// x, y, z relative to chunk. Returns block inside the chunk. If outside of the chunk, returns nullptr
	Block *blockAtSafe(BlockCoord x, BlockCoord y, BlockCoord z);

	// x, y, z, relative to the chunk. If not inside the chunk, it will use World * and ask it for the block. cx, cy, cz starting block coordinates for the chunk
	Block *blockAtExternal(BlockCoord x, BlockCoord y, BlockCoord z, BlockCoord cx, BlockCoord cy, BlockCoord cz, World *);

	// Regenerates the mesh for the chunk. x, y, z are chunk coordinates (not block coordinates)
	void regenerateChunkMesh(BlockCoord x, BlockCoord y, BlockCoord z, World *);

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
	return (BlockCoord)(60.0f + height * 15.0f);
}
