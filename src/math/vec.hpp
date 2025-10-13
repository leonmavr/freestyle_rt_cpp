#ifndef VEC_HPP_
#define VEC_HPP_

#include <cmath>
#include <cstdint>
#include <iostream>
#include <array>

// forward declaration for aliases
template <typename T> struct Xyz;
// aliases for different types of data - use those for operations
using Vec3u8  = Xyz<uint8_t>;
using Vec3i32 = Xyz<int32_t>;
using Vec3f   = Xyz<float>;


template <typename T>
struct Xyz {
  std::array<T, 3> xyz;
  // references to elements [0], [1], [2] respectively
  // TODO: make them accessors
  T& x, y, z; 
  Xyz() : xyz({T{}, T{}, T{}}), x(xyz[0]), y(xyz[1]), z(xyz[2]) {}
  Xyz(T x, T y, T z) : xyz({x, y, z}), x(xyz[0]), y(xyz[1]), z(xyz[2]) {}
  // explicit to avoid converting floats or ints to Xyz
  explicit Xyz(T val) : xyz({val, val, val}), 
                        x(xyz[0]), y(xyz[1]), z(xyz[2]) {}

  Xyz(const Xyz<T>& other) 
    : xyz(other.xyz), x(xyz[0]), y(xyz[1]), z(xyz[2]) {}
  
  Xyz<T>& operator=(const Xyz<T>& other) {
    if (this != &other) {
      // copy array only - x, y, z already reference it
      xyz = other.xyz;
    }
    return *this;
  }
  
  Xyz(Xyz<T>&& other) noexcept
    : xyz(std::move(other.xyz)), x(xyz[0]), y(xyz[1]), z(xyz[2]) {}
  
  Xyz<T>& operator=(Xyz<T>&& other) noexcept {
    if (this != &other) xyz = std::move(other.xyz);
    return *this;
  }

  //---------------------------------------------------------------------
  // vector to vector overloaded operators
  //---------------------------------------------------------------------
  #define VECTOR_OPS(OP)                                                \
    template <typename U>                                               \
    auto operator OP(const Xyz<U>& other) const ->                      \
    Xyz<decltype(T{} OP U{})> {                                         \
      return Xyz<decltype(T{} OP U{})>(                                 \
          xyz[0] OP other.xyz[0],                                       \
          xyz[1] OP other.xyz[1],                                       \
          xyz[2] OP other.xyz[2]                                        \
      );                                                                \
    }                                                                   \
                                                                        \
    template <typename U>                                               \
    Xyz<T>& operator OP##=(const Xyz<U>& other) {                       \
      xyz[0] OP##= other.xyz[0];                                        \
      xyz[1] OP##= other.xyz[1];                                        \
      xyz[2] OP##= other.xyz[2];                                        \
      return *this;                                                     \
    }

    VECTOR_OPS(+)
    VECTOR_OPS(-)
    VECTOR_OPS(*)
    VECTOR_OPS(/)
  #undef VECTOR_OPS
  //---------------------------------------------------------------------
  // vector to scalar overloaded operators
  //---------------------------------------------------------------------
  // int type OP floating type operations are not properly supported
  #define SCALAR_OPS(OP)                                                \
    template <typename U>                                               \
    auto operator OP(const U scalar) const->                            \
      Xyz<decltype(T {} OP U{})> {                                      \
      return Xyz<decltype(T {} OP U{})>                                 \
        (xyz[0] OP scalar,                                              \
         xyz[1] OP scalar,                                              \
         xyz[2] OP scalar);                                             \
    }                                                                   \
    template <typename U>                                               \
    Xyz<T>& operator OP##=(const U scalar) {                            \
      xyz[0] OP##= scalar;                                              \
      xyz[1] OP##= scalar;                                              \
      xyz[2] OP##= scalar;                                              \
      return *this;                                                     \
    }
    SCALAR_OPS(+)
    SCALAR_OPS(-)
    SCALAR_OPS(*)
    SCALAR_OPS(/)
  #undef SCALAR_OPS
  //--------------------------------------------------------------------
  // equality operators
  //--------------------------------------------------------------------
  bool operator==(const Xyz<T>& other) const {
    if constexpr (std::is_floating_point_v<T>) {
      constexpr T eps = static_cast<T>(1e-4);
      return (std::fabs(xyz[0] - other.xyz[0]) < eps) &&
             (std::fabs(xyz[1] - other.xyz[1]) < eps) &&
             (std::fabs(xyz[2] - other.xyz[2]) < eps);
    } else {
      return (xyz[0] == other.xyz[0]) &&
             (xyz[1] == other.xyz[1]) &&
             (xyz[2] == other.xyz[2]);
    }
  }

  bool operator!=(const Xyz<T>& other) const { return !(*this == other);
  }
  //---------------------------------------------------------------------
  // other operators
  //---------------------------------------------------------------------
  // unary negate
  Xyz<T> operator-() const {
      return Xyz<T>(-xyz[0], -xyz[1], -xyz[2]);
  }

  // index - useful for matrix operations later
  T operator[](int i) const { 
    if (i < 0 || i > 3) [[unlikely]]
      throw std::out_of_range("Vector index out of range");
    return xyz[i];
  }
  T& operator[](int i) { 
    if (i < 0 || i > 3) [[unlikely]]
      throw std::out_of_range("Vector index out of range");
    return xyz[i];
  }

  // when doing std::cout << xyz;
  friend std::ostream& operator<<(std::ostream& os, const Xyz& xyz) {
    os << "[" << xyz[0] << ", " << xyz[1] << ", " << xyz[2] << "]";
    return os;
  }
  //---------------------------------------------------------------------
  // common vector operations
  //---------------------------------------------------------------------
  T Dot(const Xyz<T>& other) const {
    return x * other.x + y * other.y + z * other.z;
  }
  
  float NormSq() const { return x*x + y*y + z*z; }

  float Norm() const { return std::sqrt(NormSq()); }
  Xyz<float> unit() const {
    float n = Norm();
    return Xyz<float>(x / n, y / n, z / n);
  }

  Xyz<float> Unit() const {
    return Xyz<float>{x/Norm(), y/Norm(), z/Norm()};
  }

  float Cos(const Xyz<T>& other) const {
    if (this != &other) [[likely]] {
      const float n1 = Norm();
      const float n2 = other.Norm();
      return Dot(other) / (n1 * n2);
    } else {
      return 1.0;
    }
  }

  float Angle(const Xyz<T>& other) const { return std::acos(Cos(other)); }
  Xyz<T> Cross(const Xyz<T>& other) const {
    return Xyz<T>(y * other.z - z * other.y,
                  z * other.x - x * other.z,
                  x * other.y - y * other.x);
  }

  void ReflectAbout(const Xyz<T>& axis) {
    // idea and formula from Bisqwit:
    // youtube.com/watch?v=N8elxpSu9pw&t=208s
    const auto n = axis.unit();
    double v = Dot(n);
    xyz[0] = static_cast<T>(2 * v * n.x - x);
    xyz[1] = static_cast<T>(2 * v * n.y - y);
    xyz[2] = static_cast<T>(2 * v * n.z - z);
  }
};

#endif  // VEC_HPP_ 
