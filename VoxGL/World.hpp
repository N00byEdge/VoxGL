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

union ChunkIndex {
	constexpr ChunkIndex(BlockCoord x, BlockCoord y, BlockCoord z): x(x), y(y), z(z) { }
	constexpr ChunkIndex(const ChunkIndex &other): repr(other.repr) { }

	template <size_t ind>
	constexpr decltype(auto) get() const {
		     if constexpr(ind == 0) return (x);
		else if constexpr(ind == 1) return (y);
		else if constexpr(ind == 2) return (z);
	}

	constexpr bool operator==(const ChunkIndex &other) const { return repr == other.repr; }
	constexpr bool operator< (const ChunkIndex &other) const { return repr <  other.repr; }

	struct {
		long long x : 22;
		long long y : 22;
		unsigned long long z : 20;
	};

	long long repr;
};

namespace std {
	template <> struct tuple_size<ChunkIndex> : public integral_constant<size_t, 3> { };

	template<size_t ind> struct tuple_element<ind, ChunkIndex> {
		using type = decltype(std::declval<ChunkIndex>().get<ind>());
	};

	template <> struct hash<ChunkIndex> {
		size_t operator()(const ChunkIndex &ci) const { return std::hash<long long>{}(ci.repr); }
	};
}

static_assert(sizeof(ChunkIndex) <= 8, "ChunkIndex too large");

struct World {
	World(glm::vec3 *const position, long long seed);
	~World();

	void draw(float deltaT, const glm::mat4 &perspective, Shader &);
	template <bool alreadyHasMutex> void tryRegen(BlockCoord x, BlockCoord y, BlockCoord z);
	template <bool alreadyHasMutex> Block *blockAt(BlockCoord x, BlockCoord y, BlockCoord z);
	template <bool alreadyHasMutex> std::shared_ptr<Chunk> getChunkAtBlock(BlockCoord x, BlockCoord y, BlockCoord z);
	template <bool alreadyHasMutex> std::shared_ptr<Chunk> getChunk(BlockCoord x, BlockCoord y, BlockCoord z);
	std::tuple<Block *, BlockSide, BlockCoord, BlockCoord, BlockCoord, float> raycast(glm::vec3 from, glm::vec3 dir, float maxDist);
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

template <bool alreadyHasMutex>
void World::tryRegen(BlockCoord x, BlockCoord y, BlockCoord z) {
	auto chunk = getChunk<alreadyHasMutex>(x, y, z);
	if (chunk)
		chunk->regenerateChunkMesh<alreadyHasMutex>(x, y, z, this);
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
std::shared_ptr<Chunk> World::getChunkAtBlock(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (z < 0) return nullptr;
	return getChunk<alreadyHasMutex>(Chunk::decomposeChunkFromBlock(x), Chunk::decomposeChunkFromBlock(y), Chunk::decomposeChunkFromBlock(z));
}

template <bool alreadyHasMutex>
std::shared_ptr<Chunk> World::getChunk(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (z < 0) return nullptr;
	auto ci = ChunkIndex(x, y, z);
	std::unique_lock<std::mutex> lck(chunkMutex, std::defer_lock);
	if constexpr(!alreadyHasMutex)
		lck.lock();

	auto chunk = chunks.find(ci);
	if (chunk == chunks.end()) return nullptr;
	else return chunk->second;
}
