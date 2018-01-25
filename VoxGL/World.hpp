#pragma once

#include "glm/glm.hpp"

#include "PerlinNoise.hpp"
#include "Blocks.hpp"
#include "Maths.hpp"

#include <thread>
#include <map>
#include <set>
#include <mutex>
#include <atomic>

struct Shader;
struct Chunk;
struct Block;

enum struct PerlinInstance : BlockCoord {
	Height,
	Temperature,
	Humidity,
};

template <int precision>
struct WorldgenPrecision {
	const static int num = 1 << precision;
	const static int noiseArg = precision - 1;
};

constexpr WorldgenPrecision<5> heightPrecision;
constexpr WorldgenPrecision<3> humidityPrecision;
constexpr WorldgenPrecision<3> temperaturePrecision;

constexpr std::pair<int, int> getPrecision(PerlinInstance pi) {
	switch (pi) {
	case PerlinInstance::Height:
		return { heightPrecision.num, heightPrecision.noiseArg };
	case PerlinInstance::Humidity:
		return { humidityPrecision.num, humidityPrecision.noiseArg };
	case PerlinInstance::Temperature:
		return { temperaturePrecision.num, temperaturePrecision.noiseArg };
	}
	assert(false);
	return {0, 0};
}

using ChunkIndex = long long;
constexpr ChunkIndex chunkIndexBits = 21;
static_assert(sizeof(ChunkIndex) * 8 > chunkIndexBits * 3, "Too many chunkPosBits for ChunkIndex type!");
constexpr BlockCoord chunkIndexMask = (1 << chunkIndexBits) - 1;

struct World {
	World(glm::vec3 *const position, long long seed);
	~World();

	void draw(float deltaT, const glm::mat4 &perspective, Shader &);
	template <bool alreadyHasMutex> void tryRegen(BlockCoord x, BlockCoord y, BlockCoord z);
	template <bool alreadyHasMutex> void chunkUpdated(BlockCoord x, BlockCoord y, BlockCoord z);
	template <bool alreadyHasMutex> Block *blockAt(BlockCoord x, BlockCoord y, BlockCoord z);
	template <bool alreadyHasMutex> Chunk *getChunkAtBlock(BlockCoord x, BlockCoord y, BlockCoord z);
	template <bool alreadyHasMutex> Chunk *getChunk(BlockCoord x, BlockCoord y, BlockCoord z);
	std::tuple<Block *, BlockSide, BlockCoord, BlockCoord, BlockCoord, float> raycast(glm::vec3 from, glm::vec3 dir, float maxDist);
	static constexpr std::tuple <BlockCoord, BlockCoord, BlockCoord> getChunkPos(ChunkIndex ci);
	static constexpr ChunkIndex getChunkIndexChunk(BlockCoord x, BlockCoord y, BlockCoord z);
	static constexpr ChunkIndex getChunkIndexBlock(BlockCoord x, BlockCoord y, BlockCoord z);

	std::mutex chunkMutex;
private:
	bool generating = true;
	void worldgen();
	glm::vec3 *const position;
	std::mutex worldgenMapMutex;
	int seed;

	std::unordered_map<long long, float> heightmap;
	std::unordered_map<long long, float> temperaturemap;
	std::unordered_map<long long, float> humiditymap;

	friend float getWorldgenVal(BlockCoord, BlockCoord, World &, PerlinInstance);

	std::unordered_map<ChunkIndex, std::shared_ptr<Chunk>> chunks;
	//std::unordered_map<ChunkIndex, std::shared_ptr<std::set<std::function<void>>>> eventCallbacks;
	std::thread worldgenThread;
};

float getWorldgenVal(BlockCoord x, BlockCoord y, World &world, PerlinInstance instance);

constexpr ChunkIndex World::getChunkIndexChunk(BlockCoord x, BlockCoord y, BlockCoord z) {
	return ((ChunkIndex)((unsigned)x & chunkIndexMask) << (chunkIndexBits * 2)) | ((ChunkIndex)((unsigned)y & chunkIndexMask) << chunkIndexBits) | (ChunkIndex)((unsigned)z & chunkIndexMask);
}

template <bool alreadyHasMutex>
void World::tryRegen(BlockCoord x, BlockCoord y, BlockCoord z) {
	auto chunk = getChunk<alreadyHasMutex>(x, y, z);
	if (chunk)
		chunk->regenerateChunkMesh<alreadyHasMutex>(x, y, z, this);
}

template <bool alreadyHasMutex>
void World::chunkUpdated(BlockCoord x, BlockCoord y, BlockCoord z) {
	// Update chunk above and below
	for (auto cz = z + 2; cz--;)
		tryRegen<alreadyHasMutex>(x, y, cz);

	// Update adjacent chunks
	for (std::pair <BlockCoord, BlockCoord> dxdy : std::vector <std::pair<BlockCoord, BlockCoord>>{ { 0, 1 },{ 0, -1 },{ 1, 0 },{ -1, 0 } })
		tryRegen<alreadyHasMutex>(x + dxdy.first, y + dxdy.second, z);
}

template <bool alreadyHasMutex>
Block *World::blockAt(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (z < 0) return nullptr;

	auto xx = Chunk::decomposeBlockPos(x);
	auto yy = Chunk::decomposeBlockPos(y);
	auto zz = Chunk::decomposeBlockPos(z);

	auto c = getChunk<alreadyHasMutex>(xx.second, yy.second, zz.second);
	if (c)
		return c->blockAt(xx.first, yy.first, zz.first);
	else
		return nullptr;
}

template <bool alreadyHasMutex>
Chunk *World::getChunkAtBlock(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (z < 0) return nullptr;
	return getChunk<alreadyHasMutex>(Chunk::decomposeChunkFromBlock(x), Chunk::decomposeChunkFromBlock(y), Chunk::decomposeChunkFromBlock(z));
}

template <bool alreadyHasMutex>
Chunk *World::getChunk(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (z < 0) return nullptr;
	auto ci = getChunkIndexChunk(x, y, z);
	if constexpr(alreadyHasMutex) {
		auto chunk = chunks.find(ci);
		if (chunk == chunks.end()) return nullptr;
		else return chunk->second.get();
	}
	else constexpr {
		std::lock_guard<std::mutex> lck(chunkMutex);
		auto chunk = chunks.find(ci);
		if (chunk == chunks.end()) return nullptr;
		else return chunk->second.get();
	}
}
