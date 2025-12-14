#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include "material_builder.hpp"
#include <vector>
#include <string>
#include <iostream>

int main(int argc, char** argv) {
  constexpr int focal_length = 900, fovx_deg = 70, fovy_deg = 60;
  Vec3f cam_center{0, -600, -400};
  Mat3x3 cam_rot{-0.3f, 0.0f, M_PI};
  std::string output_file = "output.ppm";

  if (argc > 1 && argv[1] != nullptr)
    output_file = std::string(argv[1]);
  if (argc >= 5) {
    try {
      cam_center.x = std::stof(argv[2]);
      cam_center.y = std::stof(argv[3]);
      cam_center.z = std::stof(argv[4]);
    } catch (...) {
      std::cerr << "Invalid camera center args; using defaults." << std::endl;
    }
  }
  if (argc >= 8) {
    try {
      float rx = Deg2Rad(std::stof(argv[5]));
      float ry = Deg2Rad(std::stof(argv[6]));
      float rz = Deg2Rad(std::stof(argv[7]));
      cam_rot = Mat3x3(rx, ry, rz);
    } catch (...) {
      std::cerr << "Invalid camera rotation args; using defaults." << std::endl;
    }
  }
  Camera cam(focal_length, fovx_deg, fovy_deg, cam_center, cam_rot);

  Lights lights;
  lights.AddAmbient(0.65);
  lights.AddDir(0.6, -0.1, -0.2, 0.3);
  lights.AddPoint(0.4, -800, 200, -800);
  lights.AddPoint(0.3, 600, -400, -1000);
  lights.AddPoint(0.3, -200, 400, 1000);
  lights.AddDir(0.6, 0.3, -0.1, -0.3);
  RayTracer ray_tracer(cam, lights);

  Material glass = MaterialBuilder()
                    .Color(255, 255, 255)
                    .Specular(60.0f)
                    .Reflective(0.3f)
                    .Transparency(0.6f)
                    .RefractiveIndex(1.25f)
                    .Tint(0.25f)
                    .Build();
  Material crystal = MaterialBuilder()
                      .Color(255, 255, 255)
                      .Specular(80.0f)
                      .Reflective(0.4f)
                      .Transparency(0.7f)
                      .RefractiveIndex(1.5f)
                      .Tint(0.25f)
                      .Build();

  std::vector<Sphere> spheres;
  spheres.emplace_back(Vec3f{-600, -200, 1500}, 300,
    MaterialBuilder().Color(232, 21, 95).Specular(5).Reflective(0.25f)
                     .Transparency(0.0f).Build());
  spheres.emplace_back(Vec3f{160, 190, 500}, 100, glass);
  {
    Material m = crystal;
    m.color = {34, 235, 23};
    m.specular = 20;
    m.transparency = 0.5f;
    spheres.emplace_back(Vec3f{-300, 400, 2000}, 250, m);
  }
  for (auto& s : spheres)
    ray_tracer.AddObject(std::move(s));

  std::vector<Block> blocks;
  blocks.emplace_back(Vec3f{250, -100, 1100}, 200, 300, 100, Mat3x3{-0.65f, 0.35f, 0.2f}, 
    MaterialBuilder().Specular(50).Reflective(0.3f).Transparency(0.45f).RefractiveIndex(1.3f).Tint(0.2f).Build());
  blocks.emplace_back(Vec3f{0, 0, 2000}, 400, 250, 100, Mat3x3{0.2f, -0.9f, 0.3f}, 
    MaterialBuilder().Color(235, 206, 23).Specular(150).Reflective(0.7f).Transparency(0.0f).Tint(0.1f).Build());
  {
    Material m = crystal;
    m.color = {0, 255, 30};
    m.tint = 0.8;
    m.specular = 20;
    blocks.emplace_back(Vec3f{400, -300, 1400}, 100, 150, 100, Mat3x3{}, m);
  }
  {
    Material m = glass;
    m.color = {40, 15, 60};
    m.transparency = 0.0f;
    m.specular = 40;
    m.reflective = 0.5f;
    blocks.emplace_back(Vec3f{0, 0, 0}, 16000, 6000, 6000, Mat3x3{}, m);
  }
  for (auto& b : blocks)     
    ray_tracer.AddObject(std::move(b));

  constexpr int nreflections = 4;
  ray_tracer.Trace(nreflections);
  ray_tracer.GammaCorrect(0.8f);
  Ppm::Write(ray_tracer.image(), output_file);
}
