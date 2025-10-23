#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include <iostream>

int main() {
  Camera cam(400, 100, 80, {100, 100, -200});
  Sphere sphere;
  sphere.center = {400, -200, 2000};
  sphere.color = {255, 0, 0};
  sphere.radius = 500;
  sphere.specular = 150;
  Sphere sphere2;
  sphere2.center = {200, 100, 1600};
  sphere2.color = {50, 100, 20};
  sphere2.radius = 300;
  sphere2.specular = 5;
  Lights lights;
  lights.AddAmbient(0.1);
  lights.AddDir(0.8, -100, -50, -400);
  lights.AddPoint(0.5, -500, -300, 400);
  lights.AddPoint(0.5, 100, 200, 200);
  RayTracer ray_tracer(cam, lights);
  ray_tracer.AddObject(sphere);
  ray_tracer.AddObject(sphere2);
  ray_tracer.Trace();
  Ppm::SaveAs(ray_tracer.image(), "output4.ppm");
}
