#pragma once

#include "Block.hpp"

#include <random>
#include <array>

constexpr float range = .5f;

template<int resolution>
inline float perlinNoise(BlockCoord x, BlockCoord y, BlockCoord seed, int instance) {
	float result = range;
	x += 1 << 30, y += 1 << 30;

	BlockCoord coordMask = ~((~0) ^ 1 << (sizeof(BlockCoord) * 8 - 1));

	for (int octave = sizeof(BlockCoord) * 8 - resolution; --octave; coordMask >>= 1) {
		auto mtseed{ (unsigned long long)(x & coordMask) << 32 ^ (y & coordMask) ^ (seed) ^ ((int)instance) ^ (octave) };
		std::mt19937_64 mt(mtseed);
		result += std::uniform_real_distribution<float>(-range, range)(mt) / (1 << octave);
	}

	return result;
}
