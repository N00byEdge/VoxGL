#include "Chunk.hpp"

#include "BlockFaceMesh.hpp"
#include "Blocks.hpp"
#include "Game.hpp"
#include "World.hpp"

#include "Maths.hpp"
#include "Util.hpp"

#include <future>

float Chunk::getBlerpWorldgenVal(BlockCoord x, BlockCoord y, World *world, PerlinInstance instance) {
  auto const num = getPrecision(instance).first;
  auto const f00 = getWorldgenVal(x, y, *world, instance);
  auto const f10 = getWorldgenVal(x + num, y, *world, instance);
  auto const f01 = getWorldgenVal(x, y + num, *world, instance);
  auto const f11 = getWorldgenVal(x + num, y + num, *world, instance);

  x = Posmod(x, getPrecision(instance).first);
  y = Posmod(y, getPrecision(instance).first);

  return Blerp(f00, f01, f10, f11, static_cast<float>(x) / (num - 1), static_cast<float>(y) / (num - 1));
}

auto const BlockgenAt = [](BlockCoord x, BlockCoord y, BlockCoord z, World *world, BlockCoord height, float temperature) {
  if(z <= height) {
    if(z < 16) {
      return StoneHandle;
    }
    if(temperature > .5f)
      return SandHandle;
    else {
      if(z == height)
        return GrassHandle;
      else
        return DirtHandle;
    }
  }
  return InvalidHandle;
};

Chunk::Chunk(BlockCoord const _x, BlockCoord const _y, BlockCoord const _z, World *world) : x(_x * ChunkSize), y(_y * ChunkSize), z(_z * ChunkSize), cx(_x),
                                                                          cy(_y), cz(_z), w(*world) {
  for(BlockCoord bx          = 0; bx < ChunkSize; ++bx) {
    for(BlockCoord by        = 0; by < ChunkSize; ++by) {
      auto const height      = blockHeight(getBlerpWorldgenVal(x + bx, y + by, world, PerlinInstance::Height));
      auto const temperature = getBlerpWorldgenVal(x + bx, y + by, world, PerlinInstance::Temperature);

      for(BlockCoord bz = 0; bz < ChunkSize; ++bz) {
        BlockHandle h   = BlockgenAt(x + bx, y + by, z + bz, &w, height, temperature);
        if(h)
          blocks[blockPos(bx, by, bz)] =
              CreateBlock(h, x + bx, y + by, z + bz, &w);
      }
    }
  }
}

Block *Chunk::blockAt(BlockCoord const x, BlockCoord const y, BlockCoord const z) {
  auto &blockVar = blocks[blockPos(x, y, z)];
  return std::visit(Overloaded{
      [](std::unique_ptr<Block> &uptr) -> Block * { return uptr.get(); }
    , [](auto &inlineBlock)            -> Block * { return &inlineBlock; }
  }, blockVar);
}

Block *Chunk::blockAtSafe(BlockCoord const x, BlockCoord const y, BlockCoord const z) {
  if(0 <= x && x < ChunkSize && 0 <= y && y < ChunkSize && 0 <= z && z < ChunkSize)
    return blockAt(x, y, z);
  return nullptr;
}

Block *Chunk::blockAtAdjacent(BlockCoord const x, BlockCoord const y, BlockCoord const z) {
  if(0 <= x && x < ChunkSize && 0 <= y && y < ChunkSize && 0 <= z &&
     z < ChunkSize)
    return blockAtSafe(x, y, z);
  if(ChunkSize <= x) {
    if(auto c = std::get<0>(adjacentChunks).lock())
      return c->blockAtSafe(x - ChunkSize, y, z);
  }
  else if(x < 0) {
    if(auto c = std::get<1>(adjacentChunks).lock())
      return c->blockAtSafe(x + ChunkSize, y, z);
  }
  else if(ChunkSize <= y) {
    if(auto c = std::get<2>(adjacentChunks).lock())
      return c->blockAtSafe(x, y - ChunkSize, z);
  }
  else if(y < 0) {
    if(auto c = std::get<3>(adjacentChunks).lock())
      return c->blockAtSafe(x, y + ChunkSize, z);
  }
  else if(ChunkSize <= z) {
    if(auto c = std::get<4>(adjacentChunks).lock())
      return c->blockAtSafe(x, y, z - ChunkSize);
  }
  else if(z < 0 && this->cz) {
    if(auto c = std::get<5>(adjacentChunks).lock())
      return c->blockAtSafe(x, y, z + ChunkSize);
  }

  return nullptr;
}

const static auto AddFace = [](BlockCoord bx, BlockCoord by, BlockCoord bz, BlockSide face, Block *b, MeshData &meshData) {
  auto md                 = b->getMesh(bx, by, bz, face);

  for(auto &ind: md.indices)
    meshData.indices.push_back(static_cast<GLuint>(ind + meshData.vertices.size()));
  for(auto &vert: md.vertices)
    meshData.vertices.push_back(vert);
};

void Chunk::regenerateChunkMesh() {
  auto meshData = std::make_unique<MeshData>();

  ForEachBlock([&](BlockCoord bx, BlockCoord by, BlockCoord bz) {
    auto b = blockAtSafe(bx, by, bz);

    if(b) {
      auto block = blockAtAdjacent(bx + 1, by, bz);
      if(!block || !block->isSolid())
        AddFace(bx + x, by + y, bz + z, BlockSide::Right, b, *meshData);

      block = blockAtAdjacent(bx - 1, by, bz);
      if(!block || !block->isSolid())
        AddFace(bx + x, by + y, bz + z, BlockSide::Left, b, *meshData);

      block = blockAtAdjacent(bx, by + 1, bz);
      if(!block || !block->isSolid())
        AddFace(bx + x, by + y, bz + z, BlockSide::Back, b, *meshData);

      block = blockAtAdjacent(bx, by - 1, bz);
      if(!block || !block->isSolid())
        AddFace(bx + x, by + y, bz + z, BlockSide::Front, b, *meshData);

      block = blockAtAdjacent(bx, by, bz + 1);
      if(!block || !block->isSolid())
        AddFace(bx + x, by + y, bz + z, BlockSide::Top, b, *meshData);

      block = blockAtAdjacent(bx, by, bz - 1);
      if(!block || !block->isSolid())
        AddFace(bx + x, by + y, bz + z, BlockSide::Bottom, b, *meshData);
    }
  });

  std::lock_guard<std::mutex> meshLock(chunkMeshMutex);
  chunkMeshData = std::move(meshData);
}

void Chunk::draw(float deltaT, const glm::vec3 &worldPos) {
  if(chunkMeshData) {
    std::lock_guard<std::mutex> lck(chunkMeshMutex);
    chunkMesh = std::make_unique<Mesh>(chunkMeshData->vertices, chunkMeshData->indices);
    chunkMeshData.reset();
  }

  if(chunkMesh)
    chunkMesh->draw();
}

void Chunk::onAdjacentChunkLoad(BlockCoord const relX, BlockCoord const relY, BlockCoord const relZ, std::weak_ptr<Chunk> const &wp) {
  if(relX == 1)
    std::get<0>(adjacentChunks) = wp;
  else if(relX == -1)
    std::get<1>(adjacentChunks) = wp;
  else if(relY == 1)
    std::get<2>(adjacentChunks) = wp;
  else if(relY == -1)
    std::get<3>(adjacentChunks) = wp;
  else if(relZ == 1)
    std::get<4>(adjacentChunks) = wp;
  else if(relZ == -1)
    std::get<5>(adjacentChunks) = wp;

  if (auto const nAdjacent =
      std::count_if(std::begin(adjacentChunks),
                    std::end  (adjacentChunks),
                    [](auto ptr) {
                      return !ptr.expired();
                    });
      nAdjacent == 6ull - !z)
    regenerateChunkMesh();
}

std::vector<std::shared_ptr<Chunk>> Chunk::getAdjacentChunks() {
  std::vector<std::shared_ptr<Chunk>> result;

  for(auto &chunk: adjacentChunks)
    if(auto sharedChunk = chunk.lock())
      result.push_back(std::move(sharedChunk));

  return result;
}

int Chunk::blockPos(BlockCoord const x, BlockCoord const y, BlockCoord const z) {
  return x + (y << ChunkCoordBits) + (z << ChunkCoordBits * 2);
}

void Chunk::reloadAdjacent(BlockCoord x, BlockCoord y, BlockCoord z) {
  if (x == ChunkSize - 1) {
    if (auto c = std::get<0>(adjacentChunks).lock())
      c->regenerateChunkMesh();
  }
  if (x == 0) {
    if (auto c = std::get<1>(adjacentChunks).lock())
      c->regenerateChunkMesh();
  }
  if (y == ChunkSize - 1) {
    if (auto c = std::get<2>(adjacentChunks).lock())
      c->regenerateChunkMesh();
  }
  if (y == 0) {
    if (auto c = std::get<3>(adjacentChunks).lock())
      c->regenerateChunkMesh();
  }
  if (z == ChunkSize - 1) {
    if (auto c = std::get<4>(adjacentChunks).lock())
      c->regenerateChunkMesh();
  }
  if (z == 0) {
    if (auto c = std::get<5>(adjacentChunks).lock())
      c->regenerateChunkMesh();
  }
}

void Chunk::removeBlockAt(BlockCoord const _x, BlockCoord const _y, BlockCoord const _z) {
  blockAt(_x, _y, _z)->remove(x + _x, y + _y, z + _z, w);
  blocks[blockPos(_x, _y, _z)] = nullptr;
  regenerateChunkMesh();
  reloadAdjacent(x, y, z);
}

void Chunk::addBlockAt(BlockCoord x, BlockCoord y, BlockCoord z, BlockStorage block) {
  auto const bp = blockPos(x, y, z);
  blocks[bp] = std::move(block);
  regenerateChunkMesh();
  reloadAdjacent(x, y, z);
}
