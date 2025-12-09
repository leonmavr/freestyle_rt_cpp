#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include "ray_tracer/materials.hpp"
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
      std::cerr << "Invalid camera center arguments, using defaults"
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
      std::cerr << "Invalid camera rotation arguments, using defaults"
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
  
  {
    auto m = Materials::Plastic();
    m.color = {19, 204, 237};
    m.specular = 100.0f;
    m.reflective = 0.8f;
    spheres.emplace_back(Vec3f{0, -400, 2100}, 400.0f, m);
  }
 
  {
    auto m = Materials::Glass();
    m.color = {26, 240, 87};
    m.tint = 0.1f;
    spheres.emplace_back(Vec3f{1100, 200, 1800}, 600.0f, m);
  }
  
  {
    auto m = Materials::Glass();
    m.color = {26, 240, 87};
    m.tint = 0.1f;
    spheres.emplace_back(Vec3f{-1100, 200, 1800}, 600.0f, m);
  }
 
  {
    auto m = Materials::Plastic();
    m.color = {240, 34, 181};
    m.reflective = 0.2f;
    spheres.emplace_back(Vec3f{-1300, -700, 5600}, 600.0f, m);
  }

  auto tex_earth = std::make_shared<Image>();
  Ppm::Read(*tex_earth, "resources/textures/earth.ppm");
  {
    auto m = Materials::Plastic();
    m.color = {240, 34, 181};
    m.specular = 50.0f;
    spheres.emplace_back(Vec3f{1300, -700, 5600}, 600.0f, m);
  }
  spheres.back().SetTexture(tex_earth);

  {
    auto m = Materials::Crystal();
    m.color = {20, 179, 227};
    m.reflective = 0.3f;
    m.transparency = 0.55f;
    m.tint = 0.75f;
    spheres.emplace_back(Vec3f{-4000, 1200, 3200}, 950.0f, m);
  }
  
  // pillars
  auto white_marble = std::make_shared<Image>();
  Ppm::Read(*white_marble, "resources/textures/marble_01.ppm");
  blocks.emplace_back(Vec3f{-1700, 800, 450}, 150, 700, 70, Mat3x3{},
                      Materials::Marble());
  blocks.back().SetTexture(white_marble, 1, 3);

  blocks.emplace_back(Vec3f{1700, 800, 450}, 150, 700, 70, Mat3x3{},
                      Materials::Marble());
  blocks.back().SetTexture(white_marble, 1, 3);
  // "ruin"
  {
    auto m = Materials::Marble();
    m.reflective = 0.0f;
    blocks.emplace_back(Vec3f{0, 1450, 1000}, 220, 220, 180,
                        Mat3x3{0.1f, 0.3f, 0.5f}, m);
  }
  blocks.back().SetTexture(white_marble);

  // floating rock
  auto tex_granite = std::make_shared<Image>(1,1);
  Ppm::Read(*tex_granite, "resources/textures/granite.ppm");
  blocks.emplace_back(Vec3f{2000, -4500, 7000}, 500, 500, 400,
                      Mat3x3{1.2f, -0.4f, 0.8f}, Materials::Marble());
  blocks.back().SetTexture(tex_granite);

  {
    // floating rock
    auto m = Materials::Marble();
    m.specular = 1.0f;
    blocks.emplace_back(Vec3f{-3500, -3200, 8000}, 350, 400, 400,
                        Mat3x3{0.3f, 0.0f, -0.6f}, m);
  }
  blocks.back().SetTexture(tex_granite);
  
  // floating rock
  blocks.emplace_back(Vec3f{4500, -2600, 4000}, 180, 230, 280,
                      Mat3x3{1.5f, 1.0f, -0.3f}, Materials::Marble());
  blocks.back().SetTexture(tex_granite);

  auto tex = std::make_shared<Image>();
  Ppm::Read(*tex, "resources/textures/checkerboard_02.ppm");
  Block ground({0, 1500, 4000}, 6500, 20, 8000);
  ground.material.specular = 2;
  ground.material.reflective = 0.3f;
  ground.material.transparency = 0.1f;
  ground.SetTexture(tex, 10, 10);
  ray_tracer.AddObject(std::move(ground));

  ray_tracer.ReadBackground("resources/bg/01.ppm");
  for (auto& s : spheres)
    ray_tracer.AddObject(std::move(s));
  for (auto & b : blocks)
    ray_tracer.AddObject(std::move(b));
 
  constexpr int nreflections = 5;
  ray_tracer.Trace(nreflections);
  ray_tracer.GammaCorrect(0.8);
  Ppm::Write(ray_tracer.image(), output_file);
}
