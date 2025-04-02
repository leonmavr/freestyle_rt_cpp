#ifndef XYZ_H
#define XYZ_H

#include <cmath>
#include <cstdint>
#include <iostream>

template <typename T> struct Xyz;
// aliases for different types of data - use those for operations
using Vec3u8 = Xyz<uint8_t>;
using Vec3i32 = Xyz<int32_t>;
using Vec3f = Xyz<float>;

template <typename T> struct Xyz {
  T x, y, z;
  Xyz() : x(0), y(0), z(0) {}
  Xyz(T x, T y, T z) : x(x), y(y), z(z) {}
  explicit Xyz(T val) : x(val), y(val), z(val) {}
  //------------------------------------------------------------------------------
  // vector to vector overloaded operators
  //------------------------------------------------------------------------------
  #define VECTOR_OPS(OP)                                                         \
    Xyz<T> operator OP(const Xyz<T> &other) const {                              \
        return Xyz<T>(x OP other.x, y OP other.y, z OP other.z);                 \
    }
    VECTOR_OPS(+)
    VECTOR_OPS(-)
    VECTOR_OPS(*)
    VECTOR_OPS(/)
  #undef VECTOR_OPS
  //------------------------------------------------------------------------------
  // vector to scalar overloaded operators
  //------------------------------------------------------------------------------
  #define SCALAR_OPS(OP)                                                         \
    template <typename U>                                                        \
    auto operator OP(const U scalar) const -> Xyz<decltype(T{} OP U{})> {        \
        return Xyz<decltype(T{} OP U{})>(x OP scalar, y OP scalar, z OP scalar); \
    }
    SCALAR_OPS(+)
    SCALAR_OPS(-)
    SCALAR_OPS(*)
    SCALAR_OPS(/)
  #undef SCALAR_OPS
  //------------------------------------------------------------------------------
  // other operators 
  //------------------------------------------------------------------------------
  // unary negate
  Xyz<T> operator-() const { return Xyz<T>(-x, -y, -z); }
  // when doing std::cout << xyz;
  friend std::ostream& operator<<(std::ostream& os, const Xyz& xyz) {
      os << "(" <<  xyz.x << ", " << xyz.y << ", " << xyz.z << ")";
      return os;
  }
  //------------------------------------------------------------------------------
  // common vector operations
  //------------------------------------------------------------------------------
  T dot(const Xyz<T> &other) const { return x * other.x + y * other.y + z * other.z; }
  auto normSq() const -> decltype(T{} * T{}) { return x * x + y * y + z * z; }
  float norm() const { return std::sqrt(static_cast<float>(normSq())); }
  Xyz<float> unit() const {
    float n = norm();
    return Xyz<float>(x / n, y / n, z / n);
  }
  float cos(const Xyz<T> &other) const {
    const float n1 = norm();
    const float n2 = other.norm();
    return dot(other) / (n1 * n2);
  }
  float angle(const Xyz<T> &other) const { return std::acos(cos(other)); }
  Xyz<T> cross(const Xyz<T> &other) const {
    return Xyz<T>(y * other.z - z * other.y, z * other.x - x * other.z,
                  x * other.y - y * other.x);
  }
  void reflectAbout(const Xyz<T> &axis) {
    // idea and formula from Bisqwit: youtube.com/watch?v=N8elxpSu9pw&t=208s
    const auto n = axis.unit();
    double v = dot(n);
    *this = Xyz<T>(
      static_cast<T>(2*v*n.x - x),
      static_cast<T>(2*v*n.y - y),
      static_cast<T>(2*v*n.z - z)
    );
  }
};

//-------------------------------------------------------------------------------
// non-member operations with scalar to vector
//-------------------------------------------------------------------------------
#define SCALAR2VEC_OPS(OP)                                                      \
  template <typename T, typename U>                                             \
  auto operator OP (U scalar, const Xyz<T> &vec) -> Xyz<decltype(U{} OP T{})> { \
    return vec OP scalar; \
  }
  SCALAR2VEC_OPS(+)
  SCALAR2VEC_OPS(-)
  SCALAR2VEC_OPS(*)
#undef SCALAR2VEC_OPS

#endif // XYZ_H
