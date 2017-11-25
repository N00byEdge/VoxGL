#pragma once

#include "glm/glm.hpp"

#include "PerlinNoise.hpp"
#include "Blocks.hpp"

#include <thread>
#include <map>
#include <set>
#include <mutex>
#include <atomic>

struct Shader;
struct Chunk;
struct Block;

using ChunkIndex = long long;
constexpr ChunkIndex chunkIndexBits = 21;
static_assert(sizeof(ChunkIndex) * 8 > chunkIndexBits * 3, "Too many chunkPosBits for ChunkIndex type!");
constexpr BlockCoord chunkIndexMask = (1 << chunkIndexBits) - 1;

enum struct PerlinInstance : BlockCoord {
	Height,
	Temperature,
	Humidity,
};

struct World {
	World(glm::vec3 *const position);
	~World();

	void draw(float deltaT, const glm::mat4 &perspective, Shader &);
	void tryRegen(BlockCoord x, BlockCoord y, BlockCoord z);
	void chunkUpdated(BlockCoord x, BlockCoord y, BlockCoord z);
	Block *blockAt(BlockCoord x, BlockCoord y, BlockCoord z);
	Chunk *getChunkAtBlock(BlockCoord x, BlockCoord y, BlockCoord z);
	Chunk *getChunk(BlockCoord x, BlockCoord y, BlockCoord z);
	static constexpr std::tuple <BlockCoord, BlockCoord, BlockCoord> getChunkPos(ChunkIndex ci);
	static constexpr ChunkIndex getChunkIndexChunk(BlockCoord x, BlockCoord y, BlockCoord z);
	static constexpr ChunkIndex getChunkIndexBlock(BlockCoord x, BlockCoord y, BlockCoord z);
private:
	bool generating = true;
	void worldgen();
	glm::vec3 *const position;
	std::mutex chunkMutex;
	int seed = 0;

	std::unordered_map<long long, float> heightmap;
	std::unordered_map<long long, float> temperaturemap;
	std::unordered_map<long long, float> humiditymap;

	friend inline float getWorldgenVal(BlockCoord, BlockCoord, World &, PerlinInstance);

	std::unordered_map<ChunkIndex, std::shared_ptr<Chunk>> chunks;
	//std::unordered_map<ChunkIndex, std::shared_ptr<std::set<std::function<void>>>> eventCallbacks;
	std::thread worldgenThread;
};

inline float getWorldgenVal(BlockCoord x, BlockCoord y, World &world, PerlinInstance instance) {
	auto cache = [&]() {
		if (instance == PerlinInstance::Height)
			return world.heightmap;

		else if (instance == PerlinInstance::Temperature)
			return world.temperaturemap;

		else if (instance == PerlinInstance::Humidity)
			return world.humiditymap;

		return world.heightmap;
	}();

	auto comp = (long long)x << 32 | y;
	auto it = cache.find(comp);

	if (it == cache.end())
		return cache[comp] = perlinNoise<2>(x, y, world.seed, (int) instance);
	else return it->second;
}
