#include "camera.hpp"
#include "vec.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  Camera cam(100, 80, 60);
  std::cout << "camera's bounding box:" << std::endl;
  std::cout << cam.WorldAABB().min << std::endl;
  std::cout << cam.WorldAABB().max << std::endl << std::endl;;
  std::cout << cam.width() << ", " << cam.height() << std::endl;
  for (int i = -10; i < 10; ++i) {
    Vec3f p = {200, 200 * i, 1400};
    std::cout << p << " -> ";
    std::cout << cam.Project(p).first << ", " << cam.Project(p).second << std::endl;
  }
}
