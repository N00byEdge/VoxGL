#include "World.hpp"
#include "Maths.hpp"

#include "Chunk.hpp"
#include "Game.hpp"

#include "Util.hpp"

#include <future>
#include "PerlinNoise.hpp"

constexpr float WorldgenDist = isDebugging ? 10.f : 15.f;

World::World(glm::vec3 *const position, long long const seed) : position(position), seed(static_cast<decltype(this->seed)>(seed)),
                                                                worldgenThread(&World::worldgen, this) { }

World::World(World &&other) noexcept : position{ other.position }, seed{0}, worldgenThread{std::move(other.worldgenThread)} {
  chunks = std::move(other.chunks);
}

World::~World() {
  generating = false;
  worldgenThread.join();
}

void World::draw(float const deltaT, glm::mat4 const &perspective, Shader &shader) {
  BlockTextures->bind();
  std::lock_guard<std::mutex> lck(chunkMutex);
  for(auto &c: chunks) {
    auto [x, y, z] = c.first;
    c.second->draw(deltaT, {x, y, z});
  }
}

// Plz no dir == .0f
float IntDist(float const from, float const dir) {
  if(dir > .0f)
    return (1.f - from) / dir;
  return -from / dir;
}

std::tuple<Block *, BlockSide, BlockCoord, BlockCoord, BlockCoord, float> World::raycast(
  glm::vec3 const from, glm::vec3 const dir, float const maxDist) {
  auto start = from;
  auto curr  = sf::Vector3i(static_cast<BlockCoord>(floor(from.x)), static_cast<BlockCoord>(floor(from.y)),
                            static_cast<BlockCoord>(floor(from.z)));
  auto progress = glm::vec3(from.x - floor(from.x), from.y - floor(from.y), from.z - floor(from.z));

  auto const lim = static_cast<int>(maxDist * 3);
  auto travelled = .0f;
  std::lock_guard<std::mutex> lck(chunkMutex);

  for(auto i       = 0; i < lim; ++ i) {
    auto remaining = 5.f;

    enum struct Changed {
      None, X, Y, Z
    };

    auto changed = Changed::None;

    if(dir.x) {
      if(auto const dist = IntDist(progress.x, dir.x); dist < remaining) {
        remaining        = dist;
        changed          = Changed::X;
      }
    }

    if(dir.y) {
      if(auto const dist = IntDist(progress.y, dir.y); dist < remaining) {
        remaining        = dist;
        changed          = Changed::Y;
      }
    }

    if(dir.z) {
      if(auto const dist = IntDist(progress.z, dir.z); dist < remaining) {
        remaining        = dist;
        changed          = Changed::Z;
      }
    }

    travelled += static_cast<float>(sqrt(Pow<2>(remaining * dir.x) + Pow<2>(remaining * dir.y) + Pow<2>(remaining * dir.z)));
    if(changed == Changed::None || travelled > maxDist)
      break;

    progress += remaining * dir;

    BlockSide hit = BlockSide::Front;

    switch(changed) {
    case Changed::X:
      if(dir.x > 0.f)
        ++curr.x, hit = BlockSide::Front, progress.x = .0f;
      else
        --curr.x, hit = BlockSide::Back, progress.x = 1.f;
      break;
    case Changed::Y:
      if(dir.y > 0.f)
        ++curr.y, hit = BlockSide::Left, progress.y = .0f;
      else
        --curr.y, hit = BlockSide::Right, progress.y = 1.f;
      break;
    case Changed::Z:
      if(dir.z > 0.f)
        ++curr.z, hit = BlockSide::Bottom, progress.z = .0f;
      else
        --curr.z, hit = BlockSide::Top, progress.z = 1.f;
      break;
    default:
      // Rip.
      break;
    }

    // #TODO: not make this assume that the block is a unit cube, this will not work for non-full blocks.
    if(auto b = blockAt<true>(curr.x, curr.y, curr.z); b)
      return {b, hit, curr.x, curr.y, curr.z, travelled};
  }

  return {nullptr, BlockSide::Top, 0, 0, 0, .0f};
}

constexpr ChunkIndex World::getChunkIndexBlock(BlockCoord x, BlockCoord y, BlockCoord z) {
  return ChunkIndex(Chunk::decomposeChunkFromBlock(x), Chunk::decomposeChunkFromBlock(y), Chunk::decomposeChunkFromBlock(z));
}

void World::worldgen() {
  auto const makeChunk = [&](BlockCoord cx, BlockCoord cy, BlockCoord cz, ChunkIndex ci) {
    auto c = std::make_shared<Chunk>(cx, cy, cz, this);
    {
      std::lock_guard<std::mutex> lck(chunkMutex);
      chunks[ci] = c;

      // Hunt for adjacent chunks!
      for(auto const &[dx, dy, dz]: Vdxyz) {
        if(auto adjC = getChunk<true>(cx + dx, cy + dy, cz + dz)) {
          adjC->onAdjacentChunkLoad(-dx, -dy, -dz, std::weak_ptr<Chunk>{c});
          c->onAdjacentChunkLoad(dx, dy, dz, adjC);
        }
      }
    }
  };

  while(generating) {
    std::vector<std::future<void>> futs;

    for(auto x = static_cast<BlockCoord>(position->x / ChunkSize - WorldgenDist - 1);
        x <= static_cast<BlockCoord>(position->x / ChunkSize + WorldgenDist) && generating; ++x) {
      for(auto y = static_cast<BlockCoord>(position->y / ChunkSize - WorldgenDist - 1);
          y <= static_cast<BlockCoord>(position->y / ChunkSize + WorldgenDist) && generating; ++y) {
        for(auto z = std::max(0, static_cast<BlockCoord>(position->z / ChunkSize - WorldgenDist - 1));
            z <= static_cast<BlockCoord>(position->z / ChunkSize + WorldgenDist) && generating; ++z) {
          auto const xd = .5f + x - position->x / ChunkSize;
          auto const yd = .5f + y - position->y / ChunkSize;
          auto const zd = .5f + z - position->z / ChunkSize;
          if(xd * xd + yd * yd + zd * zd > WorldgenDist * WorldgenDist)
            continue;
          auto height = Chunk::blockHeight(Chunk::getBlerpWorldgenVal(x * ChunkSize, y * ChunkSize, this, PerlinInstance::Height));
          auto ci     = ChunkIndex(x, y, z);
          bool create;
          {
            std::lock_guard<std::mutex> lck(chunkMutex);
            create = (chunks.find(ci) == chunks.end());
          }

          if constexpr(isDebugging) {
            if(create)
              makeChunk(x, y, z, ci);
          }
          else {
            if(create)
              futs.push_back(std::async(makeChunk, x, y, z, ci));
          }
        }
      }
    }

    if constexpr(!isDebugging) {
      for(auto &f: futs)
        f.get();
      futs.clear();
    }

    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(300));
  }
}

float getWorldgenVal(BlockCoord x, BlockCoord y, World &world, PerlinInstance instance) {
  int num;
  auto *cache = [&]() {
    if(instance == PerlinInstance::Height) {
      num = HeightPrecision.Num;
      return &world.heightmap;
    }

    else if(instance == PerlinInstance::Temperature) {
      num = TemperaturePrecision.Num;
      return &world.temperaturemap;
    }

    else if(instance == PerlinInstance::Humidity) {
      num = HumidityPrecision.Num;
      return &world.humiditymap;
    }

    assert(false);
    return static_cast<decltype(&world.heightmap)>(nullptr);
  }();

  x -= Posmod(x, num);
  y -= Posmod(y, num);

  auto const comp = ((static_cast<long long>(x) << 32)) | (static_cast<long long>(y) & 0x00000000ffffffff);

  // If cached, return it
  {
    std::lock_guard<std::mutex> lck(world.worldgenMapMutex);
    auto const it = cache->find(comp);
    if(it != cache->end()) { return it->second; }
  }

  // If not cached, compute and store it
  float val = 0;
  switch(instance) {
  case PerlinInstance::Height:
    val = PerlinNoise<HeightPrecision.NoiseArg>(x, y, world.seed, static_cast<int>(instance));
    break;
  case PerlinInstance::Humidity:
    val = PerlinNoise<HumidityPrecision.NoiseArg>(x, y, world.seed, static_cast<int>(instance));
    break;
  case PerlinInstance::Temperature:
    val = PerlinNoise<TemperaturePrecision.NoiseArg>(x, y, world.seed, static_cast<int>(instance));
    break;
  }

  std::lock_guard<std::mutex> lck(world.worldgenMapMutex);
  (*cache)[comp] = val;

  return val;
}
