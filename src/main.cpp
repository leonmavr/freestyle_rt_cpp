#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include <vector>

int main() {
  constexpr int focal_length = 1400, fovx_deg = 120, fovy_deg = 100,
    camz = -1000;
  Camera cam(focal_length, fovx_deg, fovy_deg, {0, 0, camz});
  Lights lights;
  lights.AddAmbient(0.85);
  lights.AddDir(0.6, 0.1, -0.7, -0.3);
  lights.AddDir(0.6, -0.3, -0.5, -0.3);
  lights.AddDir(0.6, 0, 0.2, -0.3);
  lights.AddPoint(1.6, -3000, 0 , 2000);
  lights.AddPoint(1, -1800, -3700, 5000);
  lights.AddPoint(0.45, -4000, -4000, 1000);
  RayTracer ray_tracer(cam, lights);
  std::vector<Sphere> spheres;
  std::vector<Block> blocks;
  
  Sphere sphere1;
  sphere1.center = {0, -400, 2100};
  sphere1.material.color = {19, 204, 237};
  sphere1.radius = 400;
  sphere1.material.specular = 100;
  sphere1.material.reflective = 0.8;
  spheres.emplace_back(sphere1);
 
  Sphere sphere2;
  sphere2.center = {1100, 200, 1800};
  sphere2.material.color = {26, 240, 87};
  sphere2.radius = 600;
  sphere2.material.transparency = 0.6;
  sphere2.material.tint = 0.6;
  sphere2.material.refractive_index = 1.1;
  sphere2.material.specular = 50;
  sphere2.material.reflective = 0.5;
  spheres.emplace_back(sphere2);
  
  Sphere sphere3;
  sphere3.center = {-1100, 200, 1800};
  sphere3.material.color = {26, 240, 87};
  sphere3.radius = 600;
  sphere3.material.transparency = 0.6;
  sphere3.material.tint = 0.6;
  sphere3.material.refractive_index = 1.4;
  sphere3.material.specular = 50;
  sphere3.material.reflective = 0.5;
  spheres.emplace_back(sphere3);
 
  Sphere sphere4;
  sphere4.center = {-1300, -700, 5600};
  sphere4.material.color = {240, 34, 181};
  sphere4.radius = 600;
  sphere4.material.specular = 50;
  sphere4.material.reflective = 0.2;
  spheres.emplace_back(sphere4);

  Sphere sphere5;
  sphere5.center = {1300, -700, 5600};
  sphere5.material.color = {240, 34, 181};
  sphere5.radius = 600;
  sphere5.material.specular = 50;
  sphere5.material.reflective = 0.;
  spheres.emplace_back(sphere5);

  Sphere sphere6;
  sphere6.center = {-4000, 1200, 3200};
  sphere6.material.color = {20, 179, 227};
  sphere6.radius = 950;
  sphere6.material.specular = 50;
  sphere6.material.reflective = 0.3;
  sphere6.material.transparency = 0.55;
  sphere6.material.refractive_index = 1.4;
  sphere6.material.tint = 0.75;
  spheres.emplace_back(sphere6);
  
  auto white_marble = std::make_shared<Image>(1,1);
  Ppm::Read(*white_marble, "resources/textures/marble_01.ppm");
  Block block1({-1700, 800, 450}, 150, 700, 70);
  block1.material.reflective = 0.3;
  block1.material.specular = 200;
  block1.SetTexture(white_marble, 1, 3);
  blocks.emplace_back(block1);

  Block block2({1700, 800, 450}, 150, 700, 70);
  block2.material.reflective = 0.3;
  block2.material.specular = 1;
  block2.SetTexture(white_marble, 1, 3);
  blocks.emplace_back(block2);

  Block block3({0, 1450, 1000}, 220, 220, 180, {0.1, 0.3, 0.5});
  block3.material.specular = 10;
  block3.SetTexture(white_marble);
  blocks.emplace_back(block3);

  auto tex_granite = std::make_shared<Image>(1,1);
  Ppm::Read(*tex_granite, "resources/textures/granite.ppm");
  Block block4({2000, -4500, 7000}, 500, 500, 400, {1.2, -0.4, 0.8});
  block4.SetTexture(tex_granite);
  blocks.emplace_back(block4);

  Block block5({-3500, -3200, 8000}, 350, 400, 400, {0.3, 0, -0.6});
  block5.SetTexture(tex_granite);
  blocks.emplace_back(block5);
  
  Block block6({4500, -2600, 4000}, 180, 230, 280, {1.5, 1, -0.3});
  block6.SetTexture(tex_granite);
  blocks.emplace_back(block6);

  auto tex = std::make_shared<Image>();
  Ppm::Read(*tex, "resources/textures/checkerboard_02.ppm");
  Block ground({0, 1500, 4000}, 6500, 20, 8000);
  ground.material.specular = 20;
  ground.material.reflective = 0.f;
  ground.material.transparency = 0.0f; // opaque
  ground.SetTexture(tex, 10, 10);
  ray_tracer.AddObject(std::move(ground));

  for (auto& s : spheres)
    ray_tracer.AddObject(std::move(s));
  for (auto & b : blocks)
    ray_tracer.AddObject(std::move(b));
 
  ray_tracer.Trace(4);
  ray_tracer.GammaCorrect(0.8);
  Ppm::Write(ray_tracer.image(), "output6.ppm");
}
