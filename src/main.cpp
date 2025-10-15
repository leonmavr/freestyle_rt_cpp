#include "camera.hpp"
#include "mat3x3.hpp"
#include "vec.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  Mat3x3 mat1, mat2;
  Camera cam(100, 80, 60);
  Vec3f p = {200, -100, 900};
  Vec3f p2 = {200, -1000, 450};
  std::cout << cam.Project(p) << std::endl;
  std::cout << cam.IsVisible(p2) << ", " << cam.Project(p2) << std::endl;

  mat1.RotateX(3.14 / 6);
  std::cout << mat1 << std::endl;
}
