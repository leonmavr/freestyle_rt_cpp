#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include <iostream>

int main() {
  Camera cam(400, 100, 80, {0, 50, 400});
  Sphere sphere;
  sphere.center = {-200, 0, 2000};
  sphere.color = {255, 0, 0};
  sphere.radius = 500;
  Lights lights;
  lights.AddAmbient(0.2);
  //lights.AddDir(0.8, -100, -50, -400);
  lights.AddPoint(0.5, 0, 0, -400);
  lights.AddPoint(0.5, 100, 200, 200);
  RayTracer ray_tracer(cam, lights);
  ray_tracer.AddObject(sphere);
  ray_tracer.Trace();
  Ppm::SaveAs(ray_tracer.image(), "output2.ppm");
}
