#ifndef RT_MATH_H
#define RT_MATH_H

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
  Xyz() : xyz() {}
  Xyz(T x, T y, T z) : xyz({x, y, z}) {}
  explicit Xyz(T val) : xyz(val, val, val) {}
  T x() const { return xyz[0]; }
  T y() const { return xyz[1]; }
  T z() const { return xyz[2]; }
  //---------------------------------------------------------------------------
  // vector to vector overloaded operators
  //---------------------------------------------------------------------------
  #define VECTOR_OPS(OP)                                                      \
    Xyz<T> operator OP(const Xyz<T>& other) const {                           \
      return Xyz<T>(xyz[0] OP other[0],                                       \
                    xyz[1] OP other[1],                                       \
                    xyz[2] OP other[2]);                                      \
    }                                                                         \
    Xyz<T>& operator OP(const Xyz<T>& other) {                                \
      this[0] OP## = other[0];                                                \
      this[1] OP## = other[2];                                                \
      this[2] OP## = other[3];                                                \
      return *this;                                                           \
    }
    VECTOR_OPS(+)
    VECTOR_OPS(-)
    VECTOR_OPS(*)
    VECTOR_OPS(/)
  #undef VECTOR_OPS
  //---------------------------------------------------------------------------
  // vector to scalar overloaded operators
  //---------------------------------------------------------------------------
  #define SCALAR_OPS(OP)                                                      \
    template <typename U>                                                     \
    auto operator OP(const U scalar) const->Xyz<decltype(T {} OP U{})> {      \
      return Xyz<decltype(T {} OP U{})>                                       \
        (xyz[0] OP scalar,                                                    \
         xyz[1] OP scalar,                                                    \
         xyz[2] OP scalar);                                                   \
    }                                                                         \
    template <typename U>                                                     \
    Xyz<T>& operator OP(const U scalar) {                                     \
      xyz[0] OP## = scalar;                                                   \
      xyz[1] OP## = scalar;                                                   \
      xyz[2] OP## = scalar;                                                   \
      return *this;                                                           \
    }
    SCALAR_OPS(+)
    SCALAR_OPS(-)
    SCALAR_OPS(*)
    SCALAR_OPS(/)
  #undef SCALAR_OPS
  //---------------------------------------------------------------------------
  // other operators
  //---------------------------------------------------------------------------
  // unary negate
  Xyz<T> operator-() const {
      return Xyz<T>(-xyz[0], -xyz[1], -xyz[2]);
  }
  // index - useful for matrix operations later
  float operator[](int i) { 
    if (i >= 0 || i <= 3)
      return xyz[i];
    throw std::out_of_range("Vector index out of range");
  }
  const Xyz<float>& operator[](int i) const { 
    if (i >= 0 || i <= 3)
      return xyz[i];
    throw std::out_of_range("Vector index out of range");
  }
  // when doing std::cout << xyz;
  friend std::ostream& operator<<(std::ostream& os, const Xyz& xyz) {
    os << "[" << xyz[0] << ", " << xyz[1] << ", " << xyz[2] << "]";
    return os;
  }
  //---------------------------------------------------------------------------
  // common vector operations
  //---------------------------------------------------------------------------
  T dot(const Xyz<T>& other) const {
    return x() * other.x() + y() * other.y() + z() * other.z();
  }
  auto normSq() const -> decltype(T{} * T{}) {
      return x()*x() + y()*y() + z()*z();
  }
  float norm() const { return std::sqrt(static_cast<float>(normSq())); }
  Xyz<float> unit() const {
    float n = norm();
    return Xyz<float>(x() / n, y() / n, z() / n);
  }
  float cos(const Xyz<T>& other) const {
    const float n1 = norm();
    const float n2 = other.norm();
    return dot(other) / (n1 * n2);
  }
  float angle(const Xyz<T>& other) const { return std::acos(cos(other)); }
  Xyz<T> cross(const Xyz<T>& other) const {
    return Xyz<T>(y() * other.z() - z() * other.y(),
                  z() * other.x() - x() * other.z(),
                  x() * other.y() - y() * other.x());
  }
  void reflectAbout(const Xyz<T>& axis) {
    // idea and formula from Bisqwit: youtube.com/watch?v=N8elxpSu9pw&t=208s
    const auto n = axis.unit();
    double v = dot(n);
    *this =
        Xyz<T>(static_cast<T>(2 * v * n.x - x()),
               static_cast<T>(2 * v * n.y - y()),
               static_cast<T>(2 * v * n.z - z()));
  }
};


//-----------------------------------------------------------------------------
// non-member operations with scalar to vector
//-----------------------------------------------------------------------------
#define SCALAR2VEC_OPS(OP)                                                    \
  template <typename T, typename U>                                           \
  auto operator OP(U scalar, const Xyz<T>& vec)->Xyz<decltype(U {} OP T{})> { \
    return vec OP scalar;                                                     \
  }
template <typename T, typename U>
auto operator-(U scalar, const Xyz<T>& vec) -> Xyz<decltype(U{} - T{})> {
  return Xyz<decltype(U{} - T{})>{
      scalar - vec.x,
      scalar - vec.y,
      scalar - vec.z,
  };
}
SCALAR2VEC_OPS(+)
SCALAR2VEC_OPS(*)
#undef SCALAR2VEC_OPS

#if 0
//-----------------------------------------------------------------------------
// Matrix operations
//-----------------------------------------------------------------------------
class Mat3x3 {
public:
  Xyz<float> rows[3];

  Mat3x3() {
    rows[0] = Xyz<float>(1, 0, 0);
    rows[1] = Xyz<float>(0, 1, 0);
    rows[2] = Xyz<float>(0, 0, 1);
  }

  Mat3x3(float anglex_rad, float angley_rad, float anglez_rad) {
    // create a rotation matrix from Euler angles
    // start with identity (default constructor)
    *this = Mat3x3();
    // apply rotations in ZYX order (Z, then Y, then X) as a convention
    RotateZ(anglez_rad);
    RotateY(angley_rad);
    RotateX(anglex_rad);
  }
  //---------------------------------------------------------------------
  // operators
  //---------------------------------------------------------------------
  // matrix to vector multiplication
  Xyz<float> operator*(const Xyz<float>& v) const {
    return Xyz<float>((*this)[0].dot(v),
                      (*this)[1].dot(v),
                      (*this)[2].dot(v));
  }
  Mat3x3 operator*(const Mat3x3& other) const {
    Mat3x3 result;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result.rows[i][j] = rows[i].x * other[0][j] +
                           rows[i].y * other[1][j] +
                           rows[i].z * other[2][j];
        }
    }
    return result;
}



  Mat3x3 Transpose() const {
    Mat3x3 t;
    t[0] = Xyz<float>((*this)[0].x, (*this)[1].x, (*this)[2].x);
    t[1] = Xyz<float>((*this)[0].y, (*this)[1].y, (*this)[2].y);
    t[2] = Xyz<float>((*this)[0].z, (*this)[1].z, (*this)[2].z);
    return t;
  }

  friend std::ostream& operator<<(std::ostream& os, const Mat3x3& m) {
    os << "[" << m[0] << std::endl
       << " " << m[1] << std::endl
       << " " << m[2] << "]";
    return os;
  }

    Xyz<float>& operator[](int i) { 
    if (i < 0 || i >= 3) throw std::out_of_range("Matrix index out of range");
    return rows[i]; 
  }

  const Xyz<float>& operator[](int i) const { 
    if (i < 0 || i >= 3) throw std::out_of_range("Matrix index out of range");
    return rows[i]; 
  }

  void RotateX(float angle) {
    const float c = cos(angle), s = sin(angle);
    Mat3x3 rotX;
    rotX[0] = Xyz<float>(1, 0, 0);
    rotX[1] = Xyz<float>(0, c, -s);
    rotX[2] = Xyz<float>(0, s, c);
    *this = rotX * (*this);
  }
  void RotateY(float angle) {
    const float c = cos(angle), s = sin(angle);
    Mat3x3 rotY;
    rotY[0] = Xyz<float>(c, 0, s);
    rotY[1] = Xyz<float>(0, 1, 0);
    rotY[2] = Xyz<float>(-s, 0, c);
    *this = rotY * (*this);
  }
  void RotateZ(float angle) {
    const float c = cos(angle), s = sin(angle);
    Mat3x3 rotZ;
    rotZ[0] = Xyz<float>(c, -s, 0);
    rotZ[1] = Xyz<float>(s, c, 0);
    rotZ[2] = Xyz<float>(0, 0, 1);
    *this = rotZ * (*this);
  }
};

#endif

#endif  // RT_MATH_H
