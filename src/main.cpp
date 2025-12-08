#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include "common.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <cmath>

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
      std::cerr << "Invalid camera center arguments, using defaults\n";
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
      std::cerr << "Invalid camera rotation arguments, using defaults\n";
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
  
  spheres.emplace_back(
      Vec3f{0, -400, 2100},
      400.0f,
      Material{
        .color = {19, 204, 237},
        .specular = 100,
        .reflective = 0.8f,
      });
 
  spheres.emplace_back(
      Vec3f{1100, 200, 1800},
      600.0f,
      Material{
        .color = {26, 240, 87},
        .specular = 50,
        .reflective = 0.5f,
        .transparency = 0.6f,
        .refractive_index = 1.1f,
        .tint = 0.6f,
      });
  
  spheres.emplace_back(
      Vec3f{-1100, 200, 1800},
      600.0f,
      Material{
        .color = {26, 240, 87},
        .specular = 50,
        .reflective = 0.5f,
        .transparency = 0.6f,
        .refractive_index = 1.4f,
        .tint = 0.6f,
      });
 
  spheres.emplace_back(
      Vec3f{-1300, -700, 5600},
      600.0f,
      Material{
        .color = {240, 34, 181},
        .specular = 50,
        .reflective = 0.2f,
      });

  auto tex_earth = std::make_shared<Image>();
  Ppm::Read(*tex_earth, "resources/textures/earth.ppm");
  spheres.emplace_back(
      Vec3f{1300, -700, 5600},
      600.0f,
      Material{
        .color = {240, 34, 181},
        .specular = 50,
      });
  // set texture on the last sphere
  spheres.back().SetTexture(tex_earth);

  spheres.emplace_back(
      Vec3f{-4000, 1200, 3200},
      950.0f,
      Material{
        .color = {20, 179, 227},
        .specular = 50,
        .reflective = 0.3f,
        .transparency = 0.55f,
        .refractive_index = 1.4f,
        .tint = 0.75f,
      });
  
  auto white_marble = std::make_shared<Image>();
  Ppm::Read(*white_marble, "resources/textures/marble_01.ppm");
  blocks.emplace_back(
      Vec3f{-1700, 800, 450},
      150, 700, 70,
      Mat3x3{},
      Material{
        .color = {50, 235, 220},
        .specular = 2,
        .reflective = 0.1f,
      });
  blocks.back().SetTexture(white_marble, 1, 3);

  blocks.emplace_back(
      Vec3f{1700, 800, 450},
      150, 700, 70,
      Mat3x3{},
      Material{
        .color = {50, 235, 220},
        .specular = 2,
        .reflective = 0.1f,
      });
  blocks.back().SetTexture(white_marble, 1, 3);

  blocks.emplace_back(
      Vec3f{0, 1450, 1000},
      220, 220, 180,
      Mat3x3{0.1f, 0.3f, 0.5f},
      Material{
        .color = {50, 235, 220},
        .specular = 10,
      });
  blocks.back().SetTexture(white_marble);

  auto tex_granite = std::make_shared<Image>(1,1);
  Ppm::Read(*tex_granite, "resources/textures/granite.ppm");
  blocks.emplace_back(
      Vec3f{2000, -4500, 7000},
      500, 500, 400,
      Mat3x3{1.2f, -0.4f, 0.8f},
      Material{});
  blocks.back().SetTexture(tex_granite);

  blocks.emplace_back(
      Vec3f{-3500, -3200, 8000},
      350, 400, 400,
      Mat3x3{0.3f, 0.0f, -0.6f},
      Material{
        .color = {50, 235, 220},
        .specular = 1,
      });
  blocks.back().SetTexture(tex_granite);
  
  blocks.emplace_back(
      Vec3f{4500, -2600, 4000},
      180, 230, 280,
      Mat3x3{1.5f, 1.0f, -0.3f},
      Material{});
  blocks.back().SetTexture(tex_granite);

  auto tex = std::make_shared<Image>();
  Ppm::Read(*tex, "resources/textures/checkerboard_02.ppm");
  Block ground({0, 1500, 4000}, 6500, 20, 8000);
  ground.material.specular = 2;
  ground.material.reflective = 0.3f;
  ground.material.transparency = 0.1f; // opaque
  ground.SetTexture(tex, 10, 10);
  ray_tracer.AddObject(std::move(ground));

  //ray_tracer.ReadBackground("resources/bg/03.ppm");
  for (auto& s : spheres)
    ray_tracer.AddObject(std::move(s));
  for (auto & b : blocks)
    ray_tracer.AddObject(std::move(b));
 
  ray_tracer.Trace(2);
  ray_tracer.GammaCorrect(0.8);
  Ppm::Write(ray_tracer.image(), output_file);
}
