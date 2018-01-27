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
	Chunk(BlockCoord x, BlockCoord y, BlockCoord z, World *, std::istream &is);

	// x, y, z, relative to chunk, Returns block inside the chunk. No chunk bounds check, use for fast access and that only.
	Block *blockAt(BlockCoord x, BlockCoord y, BlockCoord z);

	// x, y, z relative to chunk. Returns block inside the chunk. If outside of the chunk, returns nullptr
	Block *blockAtSafe(BlockCoord x, BlockCoord y, BlockCoord z);

	// x, y, z, relative to the chunk. If not inside the chunk, it will use World * and ask it for the block. cx, cy, cz starting block coordinates for the chunk
	template <bool alreadyHasMutex> Block *blockAtExternal(BlockCoord x, BlockCoord y, BlockCoord z);
	Block *blockAtAdjacent(BlockCoord x, BlockCoord y, BlockCoord z);

	// Regenerates the mesh for the chunk. x, y, z are chunk coordinates (not block coordinates)
	void regenerateChunkMesh();

	// Draws the chunk mesh
	void draw(float deltaT, const glm::vec3 &pos);

	void onAdjacentChunkLoad(BlockCoord relX, BlockCoord relY, BlockCoord relZ, std::weak_ptr<Chunk> chunk);
	std::vector<std::shared_ptr<Chunk>> getAdjacentChunks();

	static constexpr BlockCoord decomposeLocalBlockFromBlock(BlockCoord bc);
	static constexpr BlockCoord decomposeChunkFromBlock(BlockCoord bc);
	static constexpr std::pair <BlockCoord, BlockCoord> decomposeBlockPos(BlockCoord bc);
	static int blockPos(BlockCoord x, BlockCoord y, BlockCoord z);

	static float getBlerpWorldgenVal(BlockCoord x, BlockCoord y, World *world, PerlinInstance instance);
	static constexpr BlockCoord blockHeight(float height);

	void removeBlockAt(BlockCoord x, BlockCoord y, BlockCoord z);

	std::ostream &operator<<(std::ostream &os);

	std::array <std::unique_ptr<Block>, chunkSize * chunkSize * chunkSize> blocks;
	std::array <std::weak_ptr<Chunk>, 6> adjacentChunks;

	std::mutex chunkMeshMutex;

	BlockCoord x, y, z, cx, cy, cz;
	World &w;
private:
	std::unique_ptr<Mesh> chunkMesh;
	std::unique_ptr<MeshData> chunkMeshData;
};

#include "World.hpp"

template <bool alreadyHasMutex>
Block *Chunk::blockAtExternal(BlockCoord _x, BlockCoord _y, BlockCoord _z) {
	if (0 <= _x && _x < chunkSize && 0 <= _y && _y < chunkSize && 0 <= _z && _z < chunkSize) return blockAt(_x, _y, _z);
	return w.blockAt<alreadyHasMutex>(_x + x, _y + y, _z + z);
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
	return (BlockCoord)(64.f + height * 5.0f);
}
