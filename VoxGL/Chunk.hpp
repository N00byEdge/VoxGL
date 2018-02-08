#pragma once

#include "Block.hpp"

#include <array>
#include <memory>
#include <mutex>

constexpr BlockCoord ChunkCoordBits = 4;
constexpr BlockCoord ChunkSize      = 1 << ChunkCoordBits;
constexpr BlockCoord ChunkBlockMask = ChunkSize - 1;
constexpr BlockCoord ChunkLocMask   = ~ChunkBlockMask;

struct World;
enum struct PerlinInstance;

using Lookup = std::unordered_map<long long, float>;

auto const static ForEachBlock = [&](auto callable) {
  for(BlockCoord x             = 0; x < ChunkSize; ++x)
    for(BlockCoord y           = 0; y < ChunkSize; ++y)
      for(BlockCoord z         = 0; z < ChunkSize; ++z)
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

  // x, y, z, relative to the chunk. If not inside the chunk, it will use World * and ask it for the block.
  template<bool AlreadyHasMutex>
  Block *blockAtExternal(BlockCoord x, BlockCoord y, BlockCoord z);
  Block *blockAtAdjacent(BlockCoord x, BlockCoord y, BlockCoord z);

  // Regenerates the mesh for the chunk. x, y, z are chunk coordinates (not block coordinates)
  void regenerateChunkMesh();

  // Draws the chunk mesh
  void draw(float deltaT, glm::vec3 const &pos);

  void onAdjacentChunkLoad(BlockCoord relX, BlockCoord relY, BlockCoord relZ, std::weak_ptr<Chunk> const &chunk);
  std::vector<std::shared_ptr<Chunk>> getAdjacentChunks();

  static constexpr BlockCoord decomposeLocalBlockFromBlock(BlockCoord bc);
  static constexpr BlockCoord decomposeChunkFromBlock(BlockCoord bc);
  static constexpr std::pair<BlockCoord, BlockCoord> decomposeBlockPos(BlockCoord bc);
  static int blockPos(BlockCoord x, BlockCoord y, BlockCoord z);

  static float getBlerpWorldgenVal(BlockCoord x, BlockCoord y, World *world, PerlinInstance instance);
  static constexpr BlockCoord blockHeight(float height);

  void removeBlockAt(BlockCoord x, BlockCoord y, BlockCoord z);

  std::ostream &operator<<(std::ostream &os);

  std::array<std::unique_ptr<Block>, ChunkSize * ChunkSize * ChunkSize> blocks;
  std::array<std::weak_ptr<Chunk>, 6> adjacentChunks;

  std::mutex chunkMeshMutex;

  BlockCoord x, y, z, cx, cy, cz;
  World &w;
private:
  std::unique_ptr<Mesh> chunkMesh;
  std::unique_ptr<MeshData> chunkMeshData;
};

#include "World.hpp"

template<bool AlreadyHasMutex>
Block *Chunk::blockAtExternal(BlockCoord _x, BlockCoord _y, BlockCoord _z) {
  if(0 <= _x && _x < ChunkSize && 0 <= _y && _y < ChunkSize && 0 <= _z && _z < ChunkSize)
    return blockAt(_x, _y, _z);
  return w.blockAt<AlreadyHasMutex>(_x + x, _y + y, _z + z);
}

constexpr BlockCoord Chunk::decomposeLocalBlockFromBlock(BlockCoord const bc) { return bc & ChunkBlockMask; }

constexpr BlockCoord Chunk::decomposeChunkFromBlock(BlockCoord const bc) { return (bc & ChunkLocMask) >> ChunkCoordBits; }

constexpr std::pair<BlockCoord, BlockCoord> Chunk::decomposeBlockPos(BlockCoord const bc) {
  return {decomposeLocalBlockFromBlock(bc), decomposeChunkFromBlock(bc)};
}

constexpr BlockCoord Chunk::blockHeight(float const height) { return static_cast<BlockCoord>(64.f + height * 5.0f); }
