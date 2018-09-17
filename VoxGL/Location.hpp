#pragma once

#include <type_traits>

namespace PositionImpl {

  template<typename T, int scale, struct Tag>
  struct Vec {
    T x, y, z;
    Vec(Vec const &other) = default;
    Vec(int x, int y, int z): x{x}, y{y}, z{z} { }

    static constexpr int pointScale = scale;

    constexpr void operator+=(Vec const &other) {
      x += other.x;
      y += other.y;
      z += other.z;
    }

    constexpr bool operator==(Vec const &other) const {
      return (x == other.x) & (y == other.y) & (z == other.z);
    }

    template<typename Scalar>
    constexpr Vec operator*(Scalar s) const {
      return Vec(x * s, y * s, z * s);
    }
  };
  
  struct RelTag { };
  template<typename T, int scale>
  struct RelPos : Vec<T, scale, RelTag> { using Vec<T, scale, RelTag>::Vec; };

  struct AbsTag { };
  template<typename T, int scale>
  struct AbsPos : Vec<T, scale, AbsTag> { using Vec<T, scale, AbsTag>::Vec; };

  template<typename lTag, typename rTag>
  struct AllowAddition : std::true_type { };

  template<>
  struct AllowAddition<AbsTag, AbsTag> : std::false_type { };

  template<typename T, int lScale, typename lTag, int rScale, typename rTag>
  auto constexpr operator+(Vec<T, lScale, lTag> const &lhs, Vec<T, rScale, rTag> const &rhs) {
    if constexpr(!AllowAddition<lTag, rTag>{}()) {
      static_assert(false, "You cannot add two absolute positions.");
    }
    if constexpr(lScale == rScale) {
      auto l = lhs;
      l += rhs;
      return l;
    }
    else if constexpr(lScale < rScale) {
      Vec<T, lScale, lTag> temp{(rhs.x * rScale) / lScale,
                                (rhs.y * rScale) / lScale,
                                (rhs.z * rScale) / lScale};
      temp += lhs;
      return temp;
    }
    else {
      return rhs + lhs;
    }
  }
}
