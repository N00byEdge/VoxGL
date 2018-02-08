#pragma once

#include "glm/glm.hpp"

#include "Blocks.hpp"

#include <thread>
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

template <int Precision>
struct WorldgenPrecision {
	static int const Num = 1 << Precision;
	static int const NoiseArg = Precision - 1;
};

constexpr WorldgenPrecision<5> HeightPrecision;
constexpr WorldgenPrecision<10> HumidityPrecision;
constexpr WorldgenPrecision<10> TemperaturePrecision;

constexpr std::pair<int, int> getPrecision(PerlinInstance pi) {
	switch (pi) {
	case PerlinInstance::Height:
		return { HeightPrecision.Num, HeightPrecision.NoiseArg };
	case PerlinInstance::Humidity:
		return { HumidityPrecision.Num, HumidityPrecision.NoiseArg };
	case PerlinInstance::Temperature:
		return { TemperaturePrecision.Num, TemperaturePrecision.NoiseArg };
	}
	throw "Waddafik";
}

union ChunkIndex {
	constexpr ChunkIndex(BlockCoord x, BlockCoord y, BlockCoord z): x(x), y(y), z(z) { }
	constexpr ChunkIndex(ChunkIndex const &other): repr(other.repr) { }

	template <size_t Ind>
	constexpr decltype(auto) get() const {
		     if constexpr(Ind == 0) return (x);
		else if constexpr(Ind == 1) return (y);
		else if constexpr(Ind == 2) return (z);
	}

	constexpr bool operator==(ChunkIndex const &other) const { return repr == other.repr; }
	constexpr bool operator< (ChunkIndex const &other) const { return repr <  other.repr; }

	struct {
		long long x : 22;
		long long y : 22;
		unsigned long long z : 20;
	};

	long long repr;
};

static_assert(sizeof(ChunkIndex) <= 8, "ChunkIndex too large");

template <> struct std::tuple_size<ChunkIndex> : public integral_constant<size_t, 3> { };

template<size_t Ind> struct std::tuple_element<Ind, ChunkIndex> {
	using type = decltype(std::declval<ChunkIndex>().get<Ind>());
};

template <> struct std::hash<ChunkIndex> {
	size_t operator()(ChunkIndex const &ci) const { return std::hash<long long>{}(ci.repr); }
};

struct World {
	World(glm::vec3 *const position, long long seed);
	~World();

	void draw(float deltaT, glm::mat4 const &perspective, Shader &);
	template <bool AlreadyHasMutex> void tryRegen(BlockCoord x, BlockCoord y, BlockCoord z);
	template <bool AlreadyHasMutex> Block *blockAt(BlockCoord x, BlockCoord y, BlockCoord z);
	template <bool AlreadyHasMutex> std::shared_ptr<Chunk> getChunkAtBlock(BlockCoord x, BlockCoord y, BlockCoord z);
	template <bool AlreadyHasMutex> std::shared_ptr<Chunk> getChunk(BlockCoord x, BlockCoord y, BlockCoord z);
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

#include "Chunk.hpp"

float getWorldgenVal(BlockCoord x, BlockCoord y, World &world, PerlinInstance instance);

template <bool AlreadyHasMutex>
void World::tryRegen(BlockCoord x, BlockCoord y, BlockCoord z) {
	auto chunk = getChunk<AlreadyHasMutex>(x, y, z);
	if (chunk)
		chunk->regenerateChunkMesh<AlreadyHasMutex>(x, y, z, this);
}

template <bool AlreadyHasMutex>
Block *World::blockAt(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (z < 0) return nullptr;

	auto xx = Chunk::decomposeBlockPos(x);
	auto yy = Chunk::decomposeBlockPos(y);
	auto zz = Chunk::decomposeBlockPos(z);

	auto c = getChunk<AlreadyHasMutex>(xx.second, yy.second, zz.second);
	if (c)
		return c->blockAt(xx.first, yy.first, zz.first);
	else
		return nullptr;
}

template <bool AlreadyHasMutex>
std::shared_ptr<Chunk> World::getChunkAtBlock(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (z < 0) return nullptr;
	return getChunk<AlreadyHasMutex>(Chunk::decomposeChunkFromBlock(x), Chunk::decomposeChunkFromBlock(y), Chunk::decomposeChunkFromBlock(z));
}

template <bool AlreadyHasMutex>
std::shared_ptr<Chunk> World::getChunk(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (z < 0) return nullptr;
	auto const ci = ChunkIndex(x, y, z);
	std::unique_lock<std::mutex> lck(chunkMutex, std::defer_lock);
	if constexpr(!AlreadyHasMutex)
		lck.lock();

	auto const chunk = chunks.find(ci);
	if (chunk == chunks.end()) return nullptr;
	else return chunk->second;
}
