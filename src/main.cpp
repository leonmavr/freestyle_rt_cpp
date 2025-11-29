#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include <vector>

int main() {
  Camera cam(400, 100, 100, {0, 0, -1000});
  

  Lights lights;
  lights.AddAmbient(0.85);
  lights.AddDir(0.6, 0.1, -0.7, -0.3);
  lights.AddDir(0.6, -0.3, -0.5, -0.3);
  lights.AddDir(0.6, 0, 0.2, -0.3);
  lights.AddPoint(1, 0, -3000, 2000);
  lights.AddPoint(0.45, -4000, -4000, 1000);
  RayTracer ray_tracer(cam, lights);

  std::vector<Sphere> spheres;
  
  spheres.resize(spheres.size() + 1);
  Sphere sphere1;
  sphere1.center = {0, -400, 1500};
  sphere1.material.color = {19, 204, 237};
  sphere1.radius = 400;
  sphere1.material.specular = 100;
  sphere1.material.reflective = 0.5;
  spheres.emplace_back(sphere1);

  spheres.resize(spheres.size() + 1);
  Sphere sphere2;
  sphere2.center = {1100, 200, 1500};
  sphere2.material.color = {26, 240, 87};
  sphere2.radius = 600;
  sphere2.material.transparency = 0.6;
  sphere2.material.tint = 0.6;
  sphere2.material.refractive_index = 1.4;
  sphere2.material.specular = 50;
  sphere2.material.reflective = 0.5;
  spheres.emplace_back(sphere2);
  

  // TODO: add a no-argument c/tor for image
  // size here will be overwritten by Read
  auto tex = std::make_shared<Image>(1,1);
  Ppm::Read(*tex, "textures/checkerboard_02.ppm");
  Block block({0, 1500, 4000}, 6500, 20, 8000); // center, half sizes
  block.material.specular = 20; // shinier to see highlights
  block.material.reflective = 0.f;
  block.material.transparency = 0.0f; // opaque
  block.SetTexture(tex, 10);
  ray_tracer.AddObject(std::move(block));

  for (auto& s : spheres)
    ray_tracer.AddObject(std::move(s));
 
  ray_tracer.Trace(5);
  ray_tracer.GammaCorrect(0.6);
  Ppm::Write(ray_tracer.image(), "output6.ppm");
}
