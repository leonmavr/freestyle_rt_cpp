#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include "material_builder.hpp"
#include "common.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <cmath>

// Challenging scene testing all of the capabilities of the ray tracer
int main(int argc, char** argv) {
  constexpr int focal_length = 400, fovx_deg = 120, fovy_deg = 100,
    camz = -1000;
  // default camera parameters
  Vec3f cam_center{0, 0, camz};
  Mat3x3 cam_rot; // identity
  std::string output_file = "output.ppm";

  // first argument is the output file
  if (argc > 1 && argv[1] != nullptr)
    output_file = std::string(argv[1]);
  // next 3 arguments are the camera center
  if (argc >= 5) {
    try {
      cam_center.x = std::stof(argv[2]);
      cam_center.y = std::stof(argv[3]);
      cam_center.z = std::stof(argv[4]);
    } catch (...) {
      std::cerr << "Invalid camera center, using defaults."
                << std::endl;
    }
  }
  // next 3 are the camera rotation
  if (argc >= 8) {
    try {
      float rx = Deg2Rad(std::stof(argv[5]));
      float ry = Deg2Rad(std::stof(argv[6]));
      float rz = Deg2Rad(std::stof(argv[7]));
      cam_rot = Mat3x3(rx, ry, rz);
    } catch (...) {
      std::cerr << "Invalid camera rotation, using defaults."
                << std::endl;
    }
  }
  Camera cam(focal_length, fovx_deg, fovy_deg, cam_center, cam_rot);

  Lights lights;
  lights.AddAmbient(0.85);
  lights.AddDir(0.6, 0.1, -0.7, -0.3);
  lights.AddDir(0.6, -0.3, -0.5, -0.3);
  lights.AddDir(0.6, 0, 0.2, -0.3);
  lights.AddPoint(1, -3000, 0 , 2000);
  lights.AddPoint(1, -1800, -3700, 5000);
  lights.AddPoint(0.45, -4000, -4000, 1000);
  RayTracer ray_tracer(cam, lights);
  std::vector<Sphere> spheres;
  std::vector<Block> blocks;
  Material plastic = MaterialBuilder()
                       .Color(19, 204, 237)
                       .Specular(100.0f)
                       .Reflective(0.8f)
                       .Build();
  Material glass = MaterialBuilder()
                     .Color(255, 255, 255)
                     .Specular(80.0f)
                     .Reflective(0.12f)
                     .Transparency(0.7f)
                     .RefractiveIndex(1.5f)
                     .Tint(0.25f)
                     .Build();
  Material crystal = MaterialBuilder()
                       .Color(20, 179, 227)
                       .Specular(90.0f)
                       .Reflective(0.3f)
                       .Transparency(0.85f)
                       .RefractiveIndex(1.7f)
                       .Tint(0.75f)
                       .Build();
  Material marble = MaterialBuilder()
                      .Color(230, 230, 230)
                      .Specular(10.0f)
                      .Reflective(0.175f)
                      .Transparency(0.0f)
                      .RefractiveIndex(1.0f)
                      .Tint(0.0f)
                      .Build();
#ifdef USE_TEXTURES
  auto tex_earth = std::make_shared<Image>();
  Ppm::Read(*tex_earth, "resources/textures/earth.ppm");
  auto tex_marble = std::make_shared<Image>();
  Ppm::Read(*tex_marble, "resources/textures/marble_01.ppm");
  auto tex_granite = std::make_shared<Image>(1,1);
  Ppm::Read(*tex_granite, "resources/textures/granite.ppm");
  auto tex_checker = std::make_shared<Image>();
  Ppm::Read(*tex_checker, "resources/textures/checkerboard_01.ppm");
#endif

  spheres.emplace_back(Vec3f{0, -400, 2100}, 400.0f, plastic);
  {
    auto m = glass;
    m.color = {26, 240, 87};
    spheres.emplace_back(Vec3f{1100, 200, 1800}, 600.0f, m);
  }
  spheres.emplace_back(Vec3f{-1100, 200, 1800}, 600.0f, glass);
  {
    auto m = plastic;
    m.color = {240, 34, 181};
    m.reflective = 0.0f;
    spheres.emplace_back(Vec3f{-1300, -700, 5600}, 600.0f, m);
    m.specular = 50.0f;
    m.reflective = 0.1f;
    spheres.emplace_back(Vec3f{1300, -700, 5600}, 600.0f, m);
  #ifdef USE_TEXTURES
    spheres.back().SetTexture(tex_earth);
  #endif
  }
  spheres.emplace_back(Vec3f{-4000, 1200, 3200}, 950.0f, crystal);
  
  // pillars
  blocks.emplace_back(Vec3f{-1700, 800, 450}, 150, 700, 70, Mat3x3{},
                      marble);
#ifdef USE_TEXTURES
  blocks.back().SetTexture(tex_marble, 1, 3);
#endif
  blocks.emplace_back(Vec3f{1700, 800, 450}, 150, 700, 70, Mat3x3{},
                      marble);
#ifdef USE_TEXTURES
  blocks.back().SetTexture(tex_marble, 1, 3);
#endif
  // "ruin"
  {
    auto m = marble;
    m.reflective = 0.0f;
    blocks.emplace_back(Vec3f{0, 1450, 1000}, 220, 220, 180,
                        Mat3x3{0.1f, 0.3f, 0.5f}, m);
  }
#ifdef USE_TEXTURES
  blocks.back().SetTexture(tex_marble);
#endif
  // floating rocks
  blocks.emplace_back(Vec3f{2000, -4500, 7000}, 500, 500, 400,
                      Mat3x3{1.2f, -0.4f, 0.8f}, marble);
  {
    auto m = marble;
    m.specular = 1.0f;
    blocks.emplace_back(Vec3f{-3500, -3200, 8000}, 350, 400, 400,
                        Mat3x3{0.3f, 0.0f, -0.6f}, m);
  }
  blocks.emplace_back(Vec3f{4500, -2600, 4000}, 180, 230, 280,
                      Mat3x3{1.5f, 1.0f, -0.3f}, marble);
#ifdef USE_TEXTURES
  for (size_t i = blocks.size() - 3; i < blocks.size(); ++i)
    blocks[i].SetTexture(tex_granite);
#endif
  // ground
  Block ground({0, 1500, 4000}, 6500, 20, 8000);
  ground.material = MaterialBuilder()
                      .Color(30, 30, 30)
                      .Specular(2.0f)
                      .Reflective(0.2f)
                      .Build(); 
#ifdef USE_TEXTURES
  ground.SetTexture(tex_checker, 10, 10);
#endif
  ray_tracer.AddObject(std::move(ground));

  for (auto& s : spheres)
    ray_tracer.AddObject(std::move(s));
  for (auto& b : blocks)
    ray_tracer.AddObject(std::move(b));

#ifdef USE_TEXTURES
  ray_tracer.ReadBackground("resources/bg/01.ppm");
#endif
  constexpr int nreflections = 3;
  ray_tracer.Trace(nreflections);
  ray_tracer.GammaCorrect(0.8);
  Ppm::Write(ray_tracer.image(), output_file);
}
