#include "World.hpp"

#include "Chunk.hpp"
#include "Game.hpp"

#include <future>

constexpr float worldgenDist = isDebugging ? 3.0f : 10.0f;

namespace {
	constexpr bool modulusWorksAsExpected = ((-(chunkSize - 1) % chunkSize) + chunkSize) % chunkSize == (-(chunkSize - 1) + chunkSize) % chunkSize;
	static_assert(modulusWorksAsExpected, "Uuuuuhhh... Modulus is not working as expected... Please try a different platform.");
}

World::World(glm::vec3 *const position) : position(position), worldgenThread(&World::worldgen, this) {
	
}


World::~World() {
	generating = false;
	worldgenThread.join();
}

void World::draw(float deltaT, const glm::mat4 &perspective, Shader &shader) {
	blockTextures->bind();
	std::lock_guard<std::mutex> lck(chunkMutex);
	for (auto &c : chunks) {
		auto [x, y, z] = getChunkPos(c.first);
		c.second->draw(deltaT, { x, y, z });
	}
}

float intDist(float from, float dir) {
	from = posfmod(from, 1.f);
	if (dir > .0f) return (1.f - from) / dir;
	else return from / dir;
}

std::tuple<Block *, BlockSide, BlockCoord, BlockCoord, BlockCoord, float> World::raycast(glm::vec3 from, glm::vec3 dir, float maxDist) {
	auto start = from;
	std::lock_guard<std::mutex> lck(chunkMutex);
	while (true) {
		float remaining = maxDist + 5.0f;

		enum struct Changed {
			none, x, y, z
		};

		Changed changed = Changed::none;

		if (dir.x) {
			auto dist = intDist(from.x, dir.x);
			if (dist < remaining)
				remaining = dist;
			changed = Changed::x;
		}

		if (dir.y) {
			auto dist = intDist(from.y, dir.y);
			if (dist < remaining)
				remaining = dist;
			changed = Changed::y;
		}

		if (dir.z) {
			auto dist = intDist(from.z, dir.z);
			if (dist < remaining)
				remaining = dist;
			changed = Changed::z;
		}

		auto xd = from.x - start.x, yd = from.y - start.y, zd = start.x - start.y;
		auto travelled = xd * xd + yd * yd + zd * zd;
		if (changed == Changed::none || travelled + remaining > maxDist)
			return { nullptr, BlockSide::Top, 0, 0, 0, .0f };

		from += remaining * dir;

		Block *b = nullptr;
		int x = 0, y = 0, z = 0;
		BlockSide hit;

		switch (changed) {
		case Changed::x:
			if (dir.x > 0.f) x = round(from.x),     y = floor(from.y),     z = floor(from.z),     hit = BlockSide::Front;
			else             x = round(from.x) - 1, y = floor(from.y),     z = floor(from.z),     hit = BlockSide::Back;
			break;
		case Changed::y:
			if (dir.y > 0.f) x = floor(from.x),     y = round(from.y),     z = floor(from.z),     hit = BlockSide::Left;
			else             x = floor(from.x),     y = round(from.y) - 1, z = floor(from.z),     hit = BlockSide::Right;
			break;
		case Changed::z:
			if (dir.z > 0.f) x = floor(from.x),     y = floor(from.y),     z = round(from.z),     hit = BlockSide::Bottom;
			else             x = floor(from.x),     y = floor(from.y),     z = round(from.z) - 1, hit = BlockSide::Top;
			break;
		}

		b = blockAt<true>(x, y, z);

		// #TODO: not make this assume that the block is a unit cube, this will not work for non-full blocks.
		if (b)
			return { b, hit, x, y, z, travelled };
	}
}

constexpr std::tuple<BlockCoord, BlockCoord, BlockCoord> World::getChunkPos(ChunkIndex ci) {
	auto cc = (unsigned long long)ci;

	auto xx = (BlockCoord)(cc >> chunkIndexBits * 2) & chunkIndexMask;
	auto yy = (BlockCoord)(cc >> chunkIndexBits) & chunkIndexMask;
	auto zz = (BlockCoord)cc & chunkIndexMask;

	if (xx & (1 << (chunkIndexBits - 1)))
		xx |= ~chunkIndexMask;

	if (yy & (1 << (chunkIndexBits - 1)))
		yy |= ~chunkIndexMask;

	if (zz & (1 << (chunkIndexBits - 1)))
		zz |= ~chunkIndexMask;

	return std::make_tuple(xx, yy, zz);
}

constexpr auto temp = World::getChunkIndexChunk(-1, 0, 0);
constexpr auto test = World::getChunkPos(temp);

constexpr ChunkIndex World::getChunkIndexBlock(BlockCoord x, BlockCoord y, BlockCoord z) {
	return getChunkIndexChunk(Chunk::decomposeChunkFromBlock(x), Chunk::decomposeChunkFromBlock(y), Chunk::decomposeChunkFromBlock(z));
}

void World::worldgen() {
	const static auto makeChunk = [&](BlockCoord cx, BlockCoord cy, BlockCoord cz, ChunkIndex ci) {
		auto c = std::make_unique<Chunk>(cx, cy, cz, this);
		{
			std::lock_guard<std::mutex> lck(chunkMutex);
			chunks[ci] = std::move(c);
		}
		chunkUpdated<false>(cx, cy, cz);
	};

	while (generating) {
		std::vector <std::future<void>> futs;

		for (BlockCoord x = (BlockCoord)(position->x / chunkSize - worldgenDist - 1); x <= (BlockCoord)(position->x / chunkSize + worldgenDist) && generating; ++x) {
			for (BlockCoord y = (BlockCoord)(position->y / chunkSize - worldgenDist - 1); y <= (BlockCoord)(position->y / chunkSize + worldgenDist) && generating; ++y) {
				auto xd = .5f + x - position->x/chunkSize, yd = .5f + y - position->y / chunkSize;
				if (xd * xd + yd * yd > worldgenDist * worldgenDist) continue;
				auto height = Chunk::blockHeight(Chunk::getBlerpWorldgenVal(x * chunkSize, y * chunkSize, this, PerlinInstance::Height));
				for (BlockCoord z = (BlockCoord)0; z <= ceil((float)height/chunkSize); ++z) {
					auto ci = getChunkIndexChunk(x, y, z);
					bool create;
					{
						std::lock_guard<std::mutex> lck(chunkMutex);
						create = (chunks.find(ci) == chunks.end());
					}
					
					if constexpr(isDebugging) {
						if (create) makeChunk(x, y, z, ci);
					}
					else {
						if (create) futs.push_back(std::async(makeChunk, x, y, z, ci));
					}
				}
			}
		}

		if constexpr(!isDebugging) {
			for (auto &f : futs)
				f.get();
			futs.clear();
		}

		std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(300));
	}
}

float getWorldgenVal(BlockCoord x, BlockCoord y, World &world, PerlinInstance instance) {
	int num;
	auto *cache = [&]() {
		if (instance == PerlinInstance::Height) {
			num = heightPrecision.num;
			return &world.heightmap;
		}

		else if (instance == PerlinInstance::Temperature) {
			num = temperaturePrecision.num;
			return &world.temperaturemap;
		}

		else if (instance == PerlinInstance::Humidity) {
			num = humidityPrecision.num;
			return &world.humiditymap;
		}

		assert(false);
		return &world.heightmap;
	}();

	x -= posmod(x, num);
	y -= posmod(y, num);

	auto comp = (((long long)x << 32)) | ((long long)y & 0x00000000ffffffff);

	// If cached, return it
	{
		std::lock_guard<std::mutex> lck(world.worldgenMapMutex);
		auto it = cache->find(comp);
		if (it != cache->end()) {
			return it->second;
		}
	}

	// If not cached, compute and store it
	float val = 0;
	switch (instance) {
	case PerlinInstance::Height:
		val = perlinNoise<heightPrecision.noiseArg>(x, y, world.seed, (int)instance);
		break;
	case PerlinInstance::Humidity:
		val = perlinNoise<humidityPrecision.noiseArg>(x, y, world.seed, (int)instance);
		break;
	case PerlinInstance::Temperature:
		val = perlinNoise<temperaturePrecision.noiseArg>(x, y, world.seed, (int)instance);
		break;
	}

	std::lock_guard<std::mutex> lck(world.worldgenMapMutex);
	(*cache)[comp] = val;

	return val;
}

namespace {
	// Make sure encoding is working properly
	static_assert(std::get<1>(World::getChunkPos(World::getChunkIndexChunk(-4, -1, -4))) == -1);
}
