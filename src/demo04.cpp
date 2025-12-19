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
#include <algorithm>
#include <random>

int main(int argc, char** argv) {
  constexpr int focal_length = 200, fovx_deg = 120, fovy_deg = 100,
    camz = -200;
  // default camera parameters
  Vec3f cam_center{0, 0, camz};
  Mat3x3 cam_rot{0, 0, 0};
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
  lights.AddDir(0.6, 0.1, -0.7, 0.3);
  lights.AddDir(0.6, 0.3, 0.5, 0.3);
  lights.AddDir(-0.1, 0.3, -0.2, 0.3);
  lights.AddPoint(1, -3000, 1000, 2000);
  lights.AddPoint(1, -2600, 1200, 3000);
  RayTracer ray_tracer(cam, lights);
  std::vector<Sphere> spheres;
  std::vector<Block> blocks;
#ifdef USE_TEXTURES
  auto tex_checker = std::make_shared<Image>();
  Ppm::Read(*tex_checker, "resources/textures/checkerboard_02.ppm");
#endif

  auto make_random_object = [&](int n_blocks, uint32_t seed = 456) {
    // the dimensions are always fixed
    constexpr float half = 150.0f;
    constexpr float step = 300.0f;
    constexpr float xmin = -1200.0f, xmax = 1200.0f;
    constexpr float ymin = -600.0f, ymax = 600.0f;
    constexpr float zmin = 900.0f, zmax = 2100.0f;
    std::vector<Vec3f> centers;
    for (float x = xmin; x <= xmax + 0.5f; x += step) {
      for (float y = ymin; y <= ymax + 0.5f; y += step) {
        for (float z = zmin; z <= zmax + 0.5f; z += step) {
          centers.push_back(Vec3f{x, y, z});
        }
      }
    }

    std::mt19937 rng(seed);
    std::shuffle(centers.begin(), centers.end(), rng);
    // sphere spawn chance, block material choice, sphere mat choice 
    std::uniform_int_distribution<int> sphere_chance(0, 2);
    std::uniform_int_distribution<int> block_choice(0, 1);
    std::uniform_int_distribution<int> sphere_choice(0, 2);
    n_blocks = std::max(0, std::min(n_blocks, static_cast<int>(centers.size())));
    blocks.reserve(blocks.size() + static_cast<size_t>(n_blocks));

    for (int i = 0; i < n_blocks; ++i) {
      const Vec3f& c = centers[static_cast<size_t>(i)];
      Material block_mat;
      bool use_texture = false;
      if (block_choice(rng) == 0) {
        block_mat = MaterialBuilder()
          .Color(230, 230, 230)
          .Specular(50.0f)
          .Reflective(0.4f)
          .Build();
      } else {
        block_mat = MaterialBuilder()
          .Color(240, 209, 125)
          .Specular(100.0f)
          .Reflective(0.15f)
          .Build();
          use_texture = true;
      }
      
      blocks.emplace_back(c, half, half, half, Mat3x3{}, block_mat);
#ifdef USE_TEXTURES
      if (use_texture)
        blocks.back().SetTexture(tex_checker, 2, 2);    
#endif
      if (sphere_chance(rng) == 0) {
        Vec3f sphere_center = Vec3f{c.x, c.y + 2*half, c.z};
        Material sphere_mat;
        switch (sphere_choice(rng)) {
          case 0:
            sphere_mat = MaterialBuilder()
              .Color(191, 55, 222)
              .Specular(120.0f)
              .Reflective(0.3f)
              .Build();
            break;
          case 1:
            sphere_mat = MaterialBuilder()
              .Color(14, 237, 196)
              .Specular(10.0f)
              .Reflective(0.0f)
              .Emission(14, 237, 196, 0.85f)
              .Build();
            break;
          default:
            sphere_mat = MaterialBuilder()
              .Color(9, 203, 237)
              .Specular(70.0f)
              .Emission(255, 255, 255, 0.25f)
              .Build();
            break;
        }
        spheres.emplace_back(sphere_center, half, sphere_mat);
      }
    }
  };
  constexpr int nobjects = 40;
  make_random_object(nobjects);

  for (auto& s : spheres)
    ray_tracer.AddObject(std::move(s));
  for (auto& b : blocks)
    ray_tracer.AddObject(std::move(b));

#ifdef USE_TEXTURES
  ray_tracer.ReadBackground("resources/bg/02.ppm");
#endif
  constexpr int nreflections = 3;
  ray_tracer.Trace(nreflections);
  ray_tracer.GammaCorrect(0.7);
  Ppm::Write(ray_tracer.image(), output_file);
}