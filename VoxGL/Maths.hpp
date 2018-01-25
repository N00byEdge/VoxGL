#pragma once

constexpr float lerp(float f0, float f1, float alpha) {
	return f0 + alpha * (f1 - f0);
}

constexpr float blerp(float f00, float f01, float f10, float f11, float ax, float ay) {
	return f00 * (1.f - ax) * (1.f - ay) + f10 * ax * (1.f - ay) + f01 * (1.f - ax) * ay + f11 * ax * ay;
}

template <typename T1, typename T2>
constexpr T1 posmod(T1 lhs, T2 rhs) {
	return ((lhs % rhs) + rhs) % rhs;
}

static_assert(posmod(5,       16) == 5, "Posmod assertion failed");
static_assert(posmod(5 + 16,  16) == 5, "Posmod assertion failed");
static_assert(posmod(-7,      16) == 9, "Posmod assertion failed");
static_assert(posmod(-7 - 16, 16) == 9, "Posmod assertion failed");

template <typename T1, typename T2>
constexpr T1 posfmod(T1 lhs, T2 rhs) {
	return fmod((fmod(lhs, rhs)) + rhs, rhs);
}

inline float length(glm::vec3 v) {
	return std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

template <int exp, typename T>
constexpr T pow(T base) {
	if      constexpr(exp == 1)     return base;
	else if constexpr(exp == 0)     return 1;
	else if constexpr(exp % 2 == 0) return pow<exp / 2>(base * base);
	else                            return base * pow<exp - 1>(base);
}

static_assert(pow<0>(5) == 1, "Pow assertion failed");
static_assert(pow<1>(5) == 5, "Pow assertion failed");
static_assert(pow<2>(5) == 5*5, "Pow assertion failed");
static_assert(pow<3>(5) == 5*5*5, "Pow assertion failed");
static_assert(pow<4>(5) == 5*5*5*5, "Pow assertion failed");
static_assert(pow<5>(5) == 5*5*5*5*5, "Pow assertion failed");
