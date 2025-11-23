#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include <vector>

int main() {
  Camera cam(400, 100, 100, {0, -1000, -700}, {0.4, -0.7, -0.1});
  std::vector<Sphere> spheres;
  
  spheres.resize(spheres.size() + 1);
  Sphere sphere1;
  sphere1.center = {0, 0, 2000};
  sphere1.material.color = {255, 0, 0};
  sphere1.radius = 400;
  sphere1.material.specular = 150;
  sphere1.material.reflective = 0.7;
  sphere1.material.transparency = 0;
  sphere1.material.tint = 0.1f;
  spheres.emplace_back(sphere1);
  
  spheres.resize(spheres.size() + 1);
  Sphere sphere2;
  sphere2.center = {-600, -200, 1500};
  sphere2.material.color = {0, 255, 0};
  sphere2.radius = 300;
  sphere2.material.specular = 5;
  sphere2.material.reflective = 0.25;
  spheres.emplace_back(sphere2);
  
  spheres.resize(spheres.size() + 1);
  Sphere sphere3;
  sphere3.center = {160, 190, 500};
  //sphere3.material.color = {0, 100, 255};
  sphere3.radius = 100;
  sphere3.material.specular = 20;
  sphere3.material.reflective = 0.3f;
  sphere3.material.transparency = 0.5;
  sphere3.material.refractive_index = 1.3f; // water-like
  sphere3.material.tint = 0.2f;
  spheres.emplace_back(sphere3);
  
  spheres.resize(spheres.size() + 1);
  Sphere sphere4;
  sphere4.center = {-300, 400, 2000};
  sphere4.material.color = {255, 255, 0};
  sphere4.radius = 250;
  sphere4.material.specular = 20;
  sphere4.material.reflective = 0.7f;
  sphere4.material.transparency = 0.4f;
  sphere4.material.refractive_index = 1.4f;
  sphere4.material.tint = 0.1;
  spheres.emplace_back(sphere4);
  
  spheres.resize(spheres.size() + 1);
  Sphere sphere5;
  sphere5.center = {400, -300, 1600};
  sphere5.material.color = {200, 0, 200};
  sphere5.radius = 200;
  sphere5.material.specular = 20;
  sphere5.material.reflective = 0.4;
  sphere5.material.transparency = 0.7f;
  sphere5.material.refractive_index = 1.5f;
  spheres.emplace_back(sphere5);

  spheres.resize(spheres.size() + 1);
  Sphere sphere6;
  sphere6.center = {0, 0, 4800};
  sphere6.material.color = {180, 190, 200};
  sphere6.radius = 1800;
  sphere6.material.specular = 80;
  spheres.emplace_back(sphere6);

  Lights lights;
  lights.AddAmbient(0.65);  // slightly brighter ambient to see shadowed areas
  lights.AddDir(0.6, -0.1, -0.2, 0.3);  // main directional from upper left
  lights.AddPoint(0.4, -800, 200, -800);  // point light from left front
  lights.AddPoint(0.3, 600, -400, -1000); // softer point light from right
  lights.AddPoint(0.3, -200, 400, 1000); // softer point light from right
  lights.AddDir(0.6, 0.3, -0.1, -0.3);  // main directional from upper left
  
  RayTracer ray_tracer(cam, lights);
  for (auto& s : spheres)
    ray_tracer.AddObject(std::move(s));
  Block block{{250, -200, 1100}, 200, 300, 100, {-0.65, 0.35, 0.2} };
  block.material.reflective = 0.3f;
  block.material.specular = 50;
  block.material.refractive_index = 1.3f;
  block.material.transparency = 0.45f;
  block.material.tint = 0.2f;
  ray_tracer.AddObject(std::move(block));

  ray_tracer.Trace(5);
  ray_tracer.GammaCorrect(0.7);
  Ppm::SaveAs(ray_tracer.image(), "output6.ppm");
}
