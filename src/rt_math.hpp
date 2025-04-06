#ifndef RT_MATH_H 
#define RT_MATH_H

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
    }                                                                            \
    Xyz<T>& operator OP(const Xyz<T> &other) {                                   \
      x OP##= other.x;                                                           \
      y OP##= other.y;                                                           \
      z OP##= other.z;                                                           \
      return *this;                                                              \
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
    }                                                                            \
    template <typename U>                                                        \
    Xyz<T>& operator OP(const U scalar) {                                        \
      x OP##= scalar;                                                            \
      y OP##= scalar;                                                            \
      z OP##= scalar;                                                            \
      return *this;                                                              \
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
      os << "[" <<  xyz.x << ", " << xyz.y << ", " << xyz.z << "]";
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
  template <typename T, typename U>
  auto operator -(U scalar, const Xyz<T> &vec) -> Xyz<decltype(U{} - T{})> {
    return Xyz<decltype(U{} - T{})> {
       scalar - vec.x, scalar - vec.y, scalar - vec.z, 
    };
  }
  SCALAR2VEC_OPS(+)
  SCALAR2VEC_OPS(*)
#undef SCALAR2VEC_OPS


//-------------------------------------------------------------------------------
// Matrix operations 
//-------------------------------------------------------------------------------
class Matrix3x3 {
public:
  Matrix3x3() {
    rows[0] = Xyz<float>(1.0f, 0.0f, 0.0f);
    rows[1] = Xyz<float>(0.0f, 1.0f, 0.0f);
    rows[2] = Xyz<float>(0.0f, 0.0f, 1.0f);
  }
  Matrix3x3(float anglex_rad, float angley_rad, float anglez_rad) {
    // create a rotation matrix from Euler angles
    // start with identity (default constructor)
    *this = Matrix3x3();
    // apply rotations in ZYX order (Z, then Y, then X) as a convention
    RotateZ(anglez_rad);
    RotateY(angley_rad);
    RotateX(anglex_rad);
  }
  // matrix to vector multiplication 
  Xyz<float> operator*(const Xyz<float>& v) const {
    return Xyz<float>(
      rows[0].dot(v),
      rows[1].dot(v),
      rows[2].dot(v)
    );
  }
  // matrix to matrix multiplication 
  Matrix3x3 operator*(const Matrix3x3& other) const {
    Matrix3x3 result;
    // transpose the other matrix to take dot products later
    Xyz<float> otherCols[3] = {
      Xyz<float>(other.rows[0].x, other.rows[1].x, other.rows[2].x),
      Xyz<float>(other.rows[0].y, other.rows[1].y, other.rows[2].y),
      Xyz<float>(other.rows[0].z, other.rows[1].z, other.rows[2].z)
    };
    for (int i = 0; i < 3; i++) {
      result.rows[i].x = rows[i].dot(otherCols[0]);
      result.rows[i].y = rows[i].dot(otherCols[1]);
      result.rows[i].z = rows[i].dot(otherCols[2]);
    }
    return result;
  }

  friend std::ostream& operator<<(std::ostream& os, const Matrix3x3& m) {
    os << "[" << m.rows[0] << std::endl
       << " " << m.rows[1] << std::endl
       << " " << m.rows[2] << "]";
    return os;
  }

private:
  Xyz<float> rows[3];
  void RotateX(float angle) {
    const float c = cos(angle), s = sin(angle);
    Matrix3x3 rotX;
    rotX.rows[0] = Xyz<float>(1, 0, 0);
    rotX.rows[1] = Xyz<float>(0, c, -s);
    rotX.rows[2] = Xyz<float>(0, s, c);
    *this = rotX * (*this);
  }
  void RotateY(float angle) {
    const float c = cos(angle), s = sin(angle);
    Matrix3x3 rotY;
    rotY.rows[0] = Xyz<float>(c , 0, s);
    rotY.rows[1] = Xyz<float>(0 , 1, 0);
    rotY.rows[2] = Xyz<float>(-s, 0, c);
    *this = rotY * (*this);
  }
  void RotateZ(float angle) {
    const float c = cos(angle), s = sin(angle);
    Matrix3x3 rotZ;
    rotZ.rows[0] = Xyz<float>(c, -s, 0);
    rotZ.rows[1] = Xyz<float>(s, c , 0);
    rotZ.rows[2] = Xyz<float>(0, 0 , 1);
    *this = rotZ * (*this);
  }
};

#endif // RT_MATH_H
