#include "World.hpp"

#include "Chunk.hpp"
#include "Game.hpp"

#include <future>

constexpr int worldgenDist = isDebugging ? 1 : 10;

namespace {
	constexpr bool modulusWorksAsExpected = ((-(chunkSize - 1) % chunkSize) + chunkSize) % chunkSize == (-(chunkSize - 1) + chunkSize) % chunkSize;
	static_assert(modulusWorksAsExpected, "Uuuuuhhh... Modulus is not working as expected... Please try a different platform.");
}

World::World(glm::vec3 *const position) : position(position), worldgenThread(&World::worldgen, this) {
	
}


World::~World() {
	generating = false;
	if(worldgenThread.joinable())
		worldgenThread.join();
}

void World::draw(float deltaT, const glm::mat4 &perspective, Shader &shader) {
	blockTextures->bind();
	std::lock_guard<std::mutex> lck(chunkMutex);
	for (auto &c : chunks) {
		auto cp = getChunkPos(c.first);
		c.second->draw(deltaT, { std::get<0>(cp), std::get<1>(cp), std::get<2>(cp) });
	}
}

void World::tryRegen(BlockCoord x, BlockCoord y, BlockCoord z) {
	auto chunk = getChunk(x, y, z);
	if (chunk)
		chunk->regenerateChunkMesh(x, y, z, this);
}

void World::chunkUpdated(BlockCoord x, BlockCoord y, BlockCoord z) {
	// Update chunk above and below
	for (auto cz = z + 2; cz--;)
		tryRegen(x, y, cz);

	// Update adjacent chunks
	for (std::pair <BlockCoord, BlockCoord> dxdy : std::vector <std::pair<BlockCoord, BlockCoord>>{ {0, 1}, {0, -1}, {1, 0}, {-1, 0} })
		tryRegen(x + dxdy.first, y + dxdy.second, z);

	//tryRegen(x, y, z);
}

Block *World::blockAt(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (z < 0) return nullptr;

	auto xx = Chunk::decomposeBlockPos(x);
	auto yy = Chunk::decomposeBlockPos(y);
	auto zz = Chunk::decomposeBlockPos(z);

	auto c = getChunk(xx.second, yy.second, zz.second);
	if (c)
		return c->blockAtSafe(xx.first, yy.first, zz.first);
	else
		return nullptr;
}

Chunk *World::getChunkAtBlock(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (z < 0) return nullptr;
	return getChunk(Chunk::decomposeChunkFromBlock(x), Chunk::decomposeChunkFromBlock(y), Chunk::decomposeChunkFromBlock(z));
}

Chunk *World::getChunk(BlockCoord x, BlockCoord y, BlockCoord z) {
	if (z < 0) return nullptr;
	auto ci = getChunkIndexChunk(x, y, z);
	std::lock_guard<std::mutex> lck(chunkMutex);
	auto chunk = chunks.find(ci);
	if (chunk == chunks.end()) return nullptr;
	else return chunk->second.get();
}

constexpr std::tuple<BlockCoord, BlockCoord, BlockCoord> World::getChunkPos(ChunkIndex ci) {
	return std::make_tuple((BlockCoord)(ci >> chunkIndexBits * 2) & chunkIndexMask, (BlockCoord)(ci >> chunkIndexBits) & chunkIndexMask, (BlockCoord)ci & chunkIndexMask);
}

constexpr ChunkIndex World::getChunkIndexChunk(BlockCoord x, BlockCoord y, BlockCoord z) {
	return ((ChunkIndex)(x & chunkIndexMask) << (chunkIndexBits * 2)) | ((ChunkIndex)(y & chunkIndexMask) << chunkIndexBits) | (ChunkIndex)(z & chunkIndexMask);
}

constexpr ChunkIndex World::getChunkIndexBlock(BlockCoord x, BlockCoord y, BlockCoord z) {
	return getChunkIndexChunk(Chunk::decomposeChunkFromBlock(x), Chunk::decomposeChunkFromBlock(y), Chunk::decomposeChunkFromBlock(z));
}

void World::worldgen() {
	//std::vector<std::tuple<BlockCoord, BlockCoord, BlockCoord>> generatedChunks;

	const static auto makeChunk = [&](BlockCoord cx, BlockCoord cy, BlockCoord cz, ChunkIndex ci) {
		auto c = std::make_unique<Chunk>(cx, cy, cz, this);
		{
			std::lock_guard<std::mutex> lck(chunkMutex);
			chunks[ci] = std::move(c);
		}
		//generatedChunks.push_back({ cx, cy, cz });
		chunkUpdated(cx, cy, cz);
	};

	while (generating) {
		std::vector <std::future<void>> futs;

		for (BlockCoord x = (BlockCoord)(position->x / chunkSize) - worldgenDist - 1; x <= (BlockCoord)(position->x / chunkSize + worldgenDist) && generating; ++x) {
			for (BlockCoord y = (BlockCoord)(position->y / chunkSize) - worldgenDist - 1; y <= (BlockCoord)(position->y / chunkSize + worldgenDist) && generating; ++y) {
				for (BlockCoord z = (BlockCoord)0; z < (BlockCoord)6; ++z) {
					auto ci = getChunkIndexChunk(x, y, z);
					bool create;
					{
						std::lock_guard<std::mutex> lck(chunkMutex);
						create = (chunks.find(ci) == chunks.end());
					}
					if (create) futs.push_back(std::async(std::launch::async, makeChunk, x, y, z, ci));
					//if (create) makeChunk(x, y, z, ci);
				}
			}
		}

		//for (auto c = 0ull; c < generatedChunks.size() && generating; ++c) {
			//auto &cc{ generatedChunks[c] };

			//futs.push_back(std::async(std::launch::async, &World::chunkUpdated, this, std::get<0>(cc), std::get<1>(cc), std::get<2>(cc)));
		//}

		for (auto &f : futs)
			f.get();

		//generatedChunks.clear();

		std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(300));
	}
}
