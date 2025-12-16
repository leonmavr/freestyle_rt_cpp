#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"
#include "material_builder.hpp"
#include "common.hpp"
#include <string>
#include <vector>
#include <unordered_map>

// letters as 7x5 character masks (X = filled, . = empty)
// feel free to add more letters
static const std::unordered_map<char, std::vector<std::string>> kFont = {
  {'F', {"XXXXX","X....","XXXX.","X....","X....","X....","X...."}},
  {'R', {"XXXX.","X...X","XXXX.","XX...","X.X..","X..X.","X...X"}},
  {'E', {"XXXXX","X....","X....","XXXX.","X....","X....","XXXXX"}},
  {'S', {".XXXX","X....","X....",".XXX.","....X","X...X",".XXX."}},
  {'T', {"XXXXX","..X..","..X..","..X..","..X..","..X..","..X.."}},
  {'Y', {"X...X",".X.X.","..X..","..X..","..X..","..X..","..X.."}},
  {'L', {"X....","X....","X....","X....","X....","X....","XXXXX"}},
  {'A', {".XXX.","X...X","X...X","XXXXX","X...X","X...X","X...X"}},
  {'C', {".XXXX","X....","X....","X....","X....","X....",".XXXX"}},
};

static std::vector<std::string> FontMask(char c) {
  auto it = kFont.find(c);
  if (it != kFont.end()) return it->second;
  // else empty font element
  return std::vector<std::string>(7, std::string(5, '.'));
}

int main(int argc, char** argv) {
  constexpr int focal_length = 700, fovx_deg = 130, fovy_deg = 80;
  Vec3f cam_center{450, 800, -100};
  Mat3x3 cam_rot;
  std::string output_file = "output.ppm";

  if (argc > 1 && argv[1] != nullptr)
    output_file = std::string(argv[1]);
  if (argc >= 5) {
    try {
      cam_center.x = std::stof(argv[2]);
      cam_center.y = std::stof(argv[3]);
      cam_center.z = std::stof(argv[4]);
    } catch (...) {
      std::cerr << "Invalid camera center, using defaults." << std::endl;
    }
  }
  if (argc >= 8) {
    try {
      float rx = Deg2Rad(std::stof(argv[5]));
      float ry = Deg2Rad(std::stof(argv[6]));
      float rz = Deg2Rad(std::stof(argv[7]));
      cam_rot = Mat3x3(rx, ry, rz);
    } catch (...) {
      std::cerr << "Invalid camera rotation, using defaults." << std::endl;
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
  const float base_z = 2500.0f;         // depth in front of camera
  const float base_y = 200.0f;          // vertical origin (top of first line)
  const float base_x = -3200.0f;        // left margin
  const float cell_w = 160.0f;          // horizontal cell size
  const float cell_h = 160.0f;          // vertical cell size
  const float letter_spacing = cell_w;  // spacing between letters
  const float line_spacing = 200.0f;    // spacing between lines
  const float radius = 70.0f;           // sphere radius per cell

  auto add_letter = [&](char ch, float x0, float y0, float z0) {
    auto mask = FontMask(ch);
    const int rows = static_cast<int>(mask.size());
    const int cols = static_cast<int>(mask.empty() ? 0 : mask[0].size());
    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        if (mask[r][c] == 'X') {
          float x = x0 + c * cell_w;
          float y = y0 + r * cell_h;
          spheres.emplace_back(Vec3f{x, y, z0}, radius,
                              MaterialBuilder()
                                .Color(11, 227, 227)
                                .Specular(40.0f)
                                .Reflective(0.3f) 
                                .Transparency(0.0f)
                                .Build());
        }
      }
    }
  };

  auto add_text_line = [&](const std::string& text, float x0, float y0, float z0) {
    float cursor_x = x0;
    for (size_t i = 0; i < text.size(); ++i) {
      char ch = text[i];
      auto mask = FontMask(ch);
      int cols = static_cast<int>(mask.empty() ? 5 : mask[0].size());
      add_letter(ch, cursor_x, y0, z0);
      cursor_x += cols * cell_w + letter_spacing;
    }
  };

  add_text_line("FREESTYLE", base_x, base_y, base_z);
  // second line: shift by character's height in rows plus spacing
  add_text_line("RAYTRACER", base_x,
                             base_y + FontMask('F').size() * cell_h +
                                      line_spacing,
                             base_z);
  for (auto& s : spheres)
    ray_tracer.AddObject(std::move(s));
  // and a big glass sphere in the foreground
  Sphere s(Vec3f{0, 200, 1000}, 800,
                    MaterialBuilder()
                      .Color(200, 230, 255)
                      .Specular(80.0f)
                      .Reflective(0.4f)
                      .Transparency(0.7f)
                      .RefractiveIndex(1.5f)
                      .Tint(0.2f)
                      .Build());
  ray_tracer.AddObject(std::move(s));

  constexpr int nreflections = 3;
  ray_tracer.Trace(nreflections);
  ray_tracer.GammaCorrect(0.8f);
  Ppm::Write(ray_tracer.image(), output_file);
  return 0;
}
