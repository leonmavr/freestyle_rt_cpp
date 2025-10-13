#ifndef MATH_3x3_HPP_
#define MATH_3x3_HPP_

#include <cstdint>
#include <iostream>
#include <array>
#include "vec.hpp"

//-----------------------------------------------------------------------
// Matrix operations
//-----------------------------------------------------------------------
struct Mat3x3 {
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
    return Xyz<float>((*this)[0].Dot(v),
                      (*this)[1].Dot(v),
                      (*this)[2].Dot(v));
  }

  Mat3x3 operator*(const Mat3x3& other) const {
  Mat3x3 result;
  Mat3x3 other_t = other.Transpose(); // so we can access columns easily
                        std::cout << other << std::endl;

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
        std::cout << rows[i]  << " * "  << other_t[j] << " = " << 
rows[i].Dot(other_t[j]) << std::endl;
      result[i][j] = rows[i].Dot(other_t[j]);
    }
  }
  return result;
}
  Mat3x3 Transpose() const {
    Mat3x3 t;
    t.rows[0] = Xyz<float>(rows[0][0], rows[1][0], rows[2][0]);
    t.rows[1] = Xyz<float>(rows[0][1], rows[1][1], rows[2][1]);
    t.rows[2] = Xyz<float>(rows[0][2], rows[1][2], rows[2][2]);
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
    const float c = std::cos(angle), s = std::sin(angle);
    Mat3x3 rotX;
    rotX[0] = Xyz<float>(1, 0, 0);
    rotX[1] = Xyz<float>(0, c, -s);
    rotX[2] = Xyz<float>(0, s, c);
    *this = rotX * (*this);
  }
  void RotateY(float angle) {
    const float c = std::cos(angle), s = std::sin(angle);
    Mat3x3 rotY;
    rotY[0] = Xyz<float>(c , 0, s);
    rotY[1] = Xyz<float>(0 , 1, 0);
    rotY[2] = Xyz<float>(-s, 0, c);
    *this = rotY * (*this);
  }
  void RotateZ(float angle) {
    const float c = std::cos(angle), s = std::sin(angle);
    Mat3x3 rotZ;
    rotZ[0] = Xyz<float>(c, -s, 0);
    rotZ[1] = Xyz<float>(s,  c, 0);
    rotZ[2] = Xyz<float>(0,  0, 1);
    *this = rotZ * (*this);
  }
};


#endif  // MATH_3x3_HPP_
