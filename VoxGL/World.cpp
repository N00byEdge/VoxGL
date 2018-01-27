#include "World.hpp"

#include "Chunk.hpp"
#include "Game.hpp"

#include "Util.hpp"

#include <future>

constexpr float worldgenDist = isDebugging ? 10.f : 15.f;

World::World(glm::vec3 *const position, long long seed) : position(position), worldgenThread(&World::worldgen, this), seed((decltype(this->seed))seed) {
	
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

// Plz no dir == .0f
float intDist(float from, float dir) {
	if (dir > .0f) return (1.f - from) / dir;
	else return -from / dir;
}

std::tuple<Block *, BlockSide, BlockCoord, BlockCoord, BlockCoord, float> World::raycast(glm::vec3 from, glm::vec3 dir, float maxDist) {
	auto start = from;
	auto curr = sf::Vector3i((BlockCoord)floor(from.x), (BlockCoord)floor(from.y), (BlockCoord)floor(from.z));
	auto progress = glm::vec3(from.x - floor(from.x), from.y - floor(from.y), from.z - floor(from.z));

	int lim = static_cast<int>(maxDist * 3);
	auto travelled = .0f;
	std::lock_guard<std::mutex> lck(chunkMutex);

	for (int i = 0; i < lim; ++ i) {
		float remaining = 5.f;

		enum struct Changed {
			none, x, y, z
		};

		Changed changed = Changed::none;

		if (dir.x) {
			if (auto dist = intDist(progress.x, dir.x); dist < remaining) {
				remaining = dist; changed = Changed::x;
			}
		}

		if (dir.y) {
			if (auto dist = intDist(progress.y, dir.y); dist < remaining) {
				remaining = dist; changed = Changed::y;
			}
		}

		if (dir.z) {
			if (auto dist = intDist(progress.z, dir.z); dist < remaining) {
				remaining = dist; changed = Changed::z;
			}
		}

		travelled += (float)sqrt(pow<2>(remaining * dir.x) + pow<2>(remaining * dir.y) + pow<2>(remaining * dir.z));
		if (changed == Changed::none || travelled > maxDist)
			break;

		progress += remaining * dir;

		BlockSide hit;

		switch (changed) {
		case Changed::x:
			if (dir.x > 0.f) ++curr.x, hit = BlockSide::Front, progress.x = .0f;
			else --curr.x, hit = BlockSide::Back, progress.x = 1.f;
			break;
		case Changed::y:
			if (dir.y > 0.f) ++curr.y, hit = BlockSide::Left, progress.y = .0f;
			else --curr.y, hit = BlockSide::Right, progress.y = 1.f;
			break;
		case Changed::z:
			if (dir.z > 0.f) ++curr.z, hit = BlockSide::Bottom, progress.z = .0f;
			else --curr.z, hit = BlockSide::Top, progress.z = 1.f;
			break;
		default:
			// Rip.
			break;
		}

		// #TODO: not make this assume that the block is a unit cube, this will not work for non-full blocks.
		if (Block *b = blockAt<true>(curr.x, curr.y, curr.z); b)
			return { b, hit, curr.x, curr.y, curr.z, travelled };
	}

	return { nullptr, BlockSide::Top, 0, 0, 0, .0f };
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

constexpr ChunkIndex World::getChunkIndexBlock(BlockCoord x, BlockCoord y, BlockCoord z) {
	return getChunkIndexChunk(Chunk::decomposeChunkFromBlock(x), Chunk::decomposeChunkFromBlock(y), Chunk::decomposeChunkFromBlock(z));
}

void World::worldgen() {
	const static auto makeChunk = [&](BlockCoord cx, BlockCoord cy, BlockCoord cz, ChunkIndex ci) {
		auto c = std::make_shared<Chunk>(cx, cy, cz, this);
		{
			std::lock_guard<std::mutex> lck(chunkMutex);
			chunks[ci] = c;

			// Hunt for adjacent chunks!
			for (auto[dx, dy, dz] : vdxyz) {
				if (auto adjC = getChunk<true>(cx + dx, cy + dy, cz + dz)) {
					adjC->onAdjacentChunkLoad(-dx, -dy, -dz, std::weak_ptr<Chunk>{ c });
					c->onAdjacentChunkLoad(dx, dy, dz, adjC);
				}
			}
		}
	};

	while (generating) {
		std::vector <std::future<void>> futs;

		for (BlockCoord x = (BlockCoord)(position->x / chunkSize - worldgenDist - 1); x <= (BlockCoord)(position->x / chunkSize + worldgenDist) && generating; ++x) {
			for (BlockCoord y = (BlockCoord)(position->y / chunkSize - worldgenDist - 1); y <= (BlockCoord)(position->y / chunkSize + worldgenDist) && generating; ++y) {
				for (BlockCoord z = std::max(0, (BlockCoord)(position->z / chunkSize - worldgenDist - 1)); z <= (BlockCoord)(position->z / chunkSize + worldgenDist) && generating; ++z) {
					auto xd = .5f + x - position->x/chunkSize, yd = .5f + y - position->y / chunkSize, zd = .5f + z - position->z / chunkSize;
					if (xd * xd + yd * yd + zd * zd > worldgenDist * worldgenDist) continue;
					auto height = Chunk::blockHeight(Chunk::getBlerpWorldgenVal(x * chunkSize, y * chunkSize, this, PerlinInstance::Height));
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
		return (decltype(&world.heightmap)) nullptr;
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
	static_assert(std::get<0>(World::getChunkPos(World::getChunkIndexChunk(-4, -1, -4))) == -4);
	static_assert(std::get<1>(World::getChunkPos(World::getChunkIndexChunk(-4, -1, -4))) == -1);
	static_assert(std::get<2>(World::getChunkPos(World::getChunkIndexChunk(-4, -1, -4))) == -4);
}
