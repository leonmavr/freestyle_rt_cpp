#include "camera.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  Camera cam(100, 80, 120, {0, 50, 0});
  cam.Rotate(0, 0.3, 0.1);
  std::cout << cam.width() << ", " << cam.height() << std::endl;
  Sphere sphere;
  sphere.center = {0, 0, 2000};
  sphere.radius = 500;
  RayTracer ray_tracer(cam);
  ray_tracer.AddObject(sphere);
  ray_tracer.Trace();
  Ppm::SaveMat("out4.ppm", ray_tracer.image());
}
