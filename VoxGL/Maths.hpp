#pragma once

constexpr float Lerp(float const f0, float const f1, float const alpha) { return f0 + alpha * (f1 - f0); }

constexpr float Blerp(float const f00, float const f01, float const f10, float const f11, float const ax, float const ay) {
  return f00 * (1.f - ax) * (1.f - ay) + f10 * ax * (1.f - ay) + f01 * (1.f - ax) * ay + f11 * ax * ay;
}

template<typename T1, typename T2>
constexpr T1 Posmod(T1 lhs, T2 rhs) { return ((lhs % rhs) + rhs) % rhs; }

static_assert(Posmod(5, 16) == 5, "Posmod assertion failed");
static_assert(Posmod(5 + 16, 16) == 5, "Posmod assertion failed");
static_assert(Posmod(-7, 16) == 9, "Posmod assertion failed");
static_assert(Posmod(-7 - 16, 16) == 9, "Posmod assertion failed");

template<typename T1, typename T2>
constexpr T1 Posfmod(T1 lhs, T2 rhs) { return fmod((fmod(lhs, rhs)) + rhs, rhs); }

inline float Length(glm::vec3 const v) { return std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z)); }

template<int Exp, typename T>
constexpr T Pow(T base) {
  if constexpr(Exp == 1)
    return base;
  else if constexpr(Exp == 0)
    return 1;
  else if constexpr(Exp % 2 == 0)
    return Pow<Exp / 2>(base * base);
  else
    return base * Pow<Exp - 1>(base);
}

static_assert(Pow<0>(5) == 1, "Pow assertion failed");
static_assert(Pow<1>(5) == 5, "Pow assertion failed");
static_assert(Pow<2>(5) == 5 * 5, "Pow assertion failed");
static_assert(Pow<3>(5) == 5 * 5 * 5, "Pow assertion failed");
static_assert(Pow<4>(5) == 5 * 5 * 5 * 5, "Pow assertion failed");
static_assert(Pow<5>(5) == 5 * 5 * 5 * 5 * 5, "Pow assertion failed");
