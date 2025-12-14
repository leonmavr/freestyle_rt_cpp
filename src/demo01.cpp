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

  std::vector<Sphere> spheres;

  spheres.emplace_back(Vec3f{-600, -200, 1500}, 300,
    MaterialBuilder().Color(0, 255, 0).Specular(5).Reflective(0.25f).Transparency(0.0f).Build());

  spheres.emplace_back(Vec3f{160, 190, 500}, 100,
    MaterialBuilder().Specular(20).Reflective(0.3f).Transparency(0.5f)
                     .RefractiveIndex(1.3f).Tint(0.2f).Build());

  spheres.emplace_back(Vec3f{-300, 400, 2000}, 250,
    MaterialBuilder().Color(255, 255, 0).Specular(20).Reflective(0.7f)
                      .Transparency(0.4f).RefractiveIndex(1.4f)
                      .Tint(0.1f).Build());

  spheres.emplace_back(Vec3f{400, -300, 1600}, 200,
    MaterialBuilder().Color(200, 0, 200).Specular(20).Reflective(0.4f).Transparency(0.7f).RefractiveIndex(1.5f).Build());

  spheres.emplace_back(Vec3f{0, 0, 4800}, 1800,
    MaterialBuilder().Color(180, 190, 200).Specular(80).Build());

  for (auto& s : spheres)
    ray_tracer.AddObject(std::move(s));

  std::vector<Block> blocks;
  blocks.emplace_back(Vec3f{250, -100, 1100}, 200, 300, 100, Mat3x3{-0.65f, 0.35f, 0.2f}, 
    MaterialBuilder().Specular(50).Reflective(0.3f).Transparency(0.45f).RefractiveIndex(1.3f).Tint(0.2f).Build());
  blocks.emplace_back(Vec3f{0, 0, 2000}, 400, 250, 100, Mat3x3{0.2f, -0.9f, 0.3f}, 
    MaterialBuilder().Color(255, 0, 0).Specular(150).Reflective(0.7f).Transparency(0.0f).Tint(0.1f).Build());
  for (auto& b : blocks)     
    ray_tracer.AddObject(std::move(b));

  ray_tracer.Trace(3);
  ray_tracer.GammaCorrect(0.7f);
  Ppm::Write(ray_tracer.image(), output_file);
}
