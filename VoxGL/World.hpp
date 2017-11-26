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
	const static int noiseArg = precision;
};

constexpr WorldgenPrecision<3> heightPrecision;
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
	std::mutex worldgenMapMutex;

	std::unordered_map<long long, float> heightmap;
	std::unordered_map<long long, float> temperaturemap;
	std::unordered_map<long long, float> humiditymap;

	friend float getWorldgenVal(BlockCoord, BlockCoord, World &, PerlinInstance);

	std::unordered_map<ChunkIndex, std::shared_ptr<Chunk>> chunks;
	//std::unordered_map<ChunkIndex, std::shared_ptr<std::set<std::function<void>>>> eventCallbacks;
	std::thread worldgenThread;
};

float getWorldgenVal(BlockCoord x, BlockCoord y, World &world, PerlinInstance instance);
