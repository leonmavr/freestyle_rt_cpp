#include "camera.hpp"
#include "mat3x3.hpp"
#include "vec.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  Camera cam(100, 80, 60);
  for (int i = 0; i < 10; ++i) {
    Vec3f p = {200, 200 * i, 600};
    std::cout << cam.Project(p).first << ", " << cam.Project(p).second << std::endl;
  }
}
