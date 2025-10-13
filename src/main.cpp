#include "mat3x3.hpp"
#include "vec.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  Mat3x3 mat1, mat2;
  mat1.RotateX(3.14/6);
  std::cout << mat1 << std::endl;
}
