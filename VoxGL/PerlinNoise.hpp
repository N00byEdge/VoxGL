#pragma once

#include "Block.hpp"

#include <random>

template<int Resolution>
float PerlinNoise(BlockCoord x, BlockCoord y, BlockCoord seed, int instance) {
  constexpr auto range = .5f;
  std::uniform_real_distribution<float> const dist(-range, range);
  auto result = range;
  x += 1 << 30, y += 1 << 30;

  BlockCoord coordMask = ~((~0) ^ 1 << (sizeof(BlockCoord) * 8 - 1));

  for(int octave = sizeof(BlockCoord) * 8 - Resolution; --octave; coordMask >>= 1) {
    auto eSeed{static_cast<unsigned long long>(x & coordMask) << 32 ^ (y & coordMask) ^ (seed) ^ static_cast<int>(instance) ^ (octave)};
    std::ranlux48 mt(eSeed);
    result += dist(mt) / (1 << octave);
  }

  return result;
}
