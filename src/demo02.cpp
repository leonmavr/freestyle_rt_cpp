#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include "material_builder.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iostream>

// Minimally renders .obj files

// Minimal OBJ loader: supports ONLY v (positions) and f (faces, aka
// triangles).
// Normals and textures are ignored, quads are triangulated i.e. split
// into two triangles.
static std::vector<Triangle>
LoadObjTriangles(const std::string& path,const Material& mat,
                 float scale = 400) {
  std::ifstream in(path);
  std::vector<Vec3f> verts;
  std::vector<Triangle> tris;
  if (!in) {
    std::cerr << "Error reading file: " << path << std::endl;
    return tris;
  }
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream iss(line);
    std::string tag;
    iss >> tag;
    if (tag == "v") {
      // vertex to refer to (index) later
      float x, y, z;
      if (iss >> x >> y >> z) verts.push_back({x, y, z});
    } else if (tag == "f") {
      // parse up to 4 indices; supports formats: i, i/j, i//k, i/j/k
      auto readIndex = [&](const std::string& token) -> int {
        if (token.empty()) return 0;
        std::istringstream ts(token);
        std::string a;
        // keep only the first component of i/j, i//k, or i/j/k
        std::getline(ts, a, '/');
        int idx = std::stoi(a);
        // handle negative indexing
        idx = (idx < 0) ? idx + verts.size() + 1 : idx;
        return idx;
      };
      std::string t1, t2, t3, t4;
      iss >> t1 >> t2 >> t3;
      if (t1.empty() || t2.empty() || t3.empty()) continue;
      int i1 = readIndex(t1) - 1;
      int i2 = readIndex(t2) - 1;
      int i3 = readIndex(t3) - 1;
      if (i1 >= 0 && i1 < verts.size() &&
          i2 >= 0 && i2 < verts.size() &&
          i3 >= 0 && i3 < verts.size()) {
        tris.emplace_back(verts[i1]*scale, verts[i2]*scale,
                          verts[i3]*scale, mat);
      }
      // triangulate quad 1 2 3 4 as  two triangles (1,2,3) and (1,3,4)
      if (iss >> t4 && !t4.empty()) {
        int i4 = readIndex(t4) - 1;
        if (i1 >= 0 && i3 >= 0 && i4 >= 0 && i1 < verts.size() &&
            i3 < verts.size() && i4 < verts.size()) {
          tris.emplace_back(verts[i1]*scale, verts[i3]*scale,
                            verts[i4]*scale, mat);
        }
      }
    }
  }
  return tris;
}


int main(int argc, char** argv) {
  constexpr int depth = 300, fovx_deg = 120, fovy_deg = 120;
  Vec3f cam_center{0, 0, -1500};
  Mat3x3 cam_rot{0.0f, 0.0f, M_PI};
  std::string output_file = "output.ppm";
  std::string obj_path = "resources/obj/teapot.obj";

  if (argc > 1 && argv[1] != nullptr)
    output_file = std::string(argv[1]);
  if (argc >= 3 && argv[2] != nullptr)
    obj_path = std::string(argv[2]);
  if (argc >= 6) {
    try {
      cam_center.x = std::stof(argv[3]);
      cam_center.y = std::stof(argv[4]);
      cam_center.z = std::stof(argv[5]);
    } catch (...) {
      std::cerr << "Invalid camera center, using defaults."
                << std::endl;
    }
  }
  if (argc >= 9) {
    try {
      float rx = Deg2Rad(std::stof(argv[6]));
      float ry = Deg2Rad(std::stof(argv[7]));
      float rz = Deg2Rad(std::stof(argv[8]));
      cam_rot = Mat3x3(rx, ry, rz);
    } catch (...) {
      std::cerr << "Invalid camera rotation, using defaults."
                << std::endl;
    }
  }
  Camera cam(depth, fovx_deg, fovy_deg, cam_center, cam_rot);
  Lights lights;
  lights.AddAmbient(0.7);
  lights.AddDir(0.6, -0.2, -0.3, 0.4);
  lights.AddPoint(0.4, -1200, 600, -800);
  lights.AddPoint(0.3, 600, -1000, -1600);

  RayTracer ray_tracer(cam, lights);

  // same sample material for all OBJ triangle elements
  Material porcelain = MaterialBuilder()
                         .Color(220, 220, 240)
                         .Specular(40.0f)
                         .Reflective(0.2f)
                         .Transparency(0.0f)
                         .Build();

  auto tris = LoadObjTriangles(obj_path, porcelain);
  if (tris.empty()) {
    std::cerr << "No triangles loaded from " << obj_path << std::endl;
    return -1;
  }
  for (auto& t : tris)
    ray_tracer.AddObject<Triangle>(std::move(t));

  constexpr int nreflections = 3;
  //ray_tracer.ReadBackground("resources/bg/01.ppm");
  ray_tracer.Trace(nreflections);
  ray_tracer.GammaCorrect(0.8f);
  Ppm::Write(ray_tracer.image(), output_file);
}
