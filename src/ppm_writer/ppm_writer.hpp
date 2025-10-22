#ifndef PPM_WRITER
#define PPM_WRITER

#include "common.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdint>


namespace Ppm {
struct Rgb {
  uint8_t r, g, b;
};

static void SaveAs(const Image &mat,
                   const std::string &filename = "output.ppm") {
  std::ofstream file(filename);
  if (!file)
    throw std::runtime_error("ERROR: Could not write to file " + filename);

  file << "P3\n";
  file << mat.width << " " << mat.height << "\n";
  file << 255 /* max intensity for uint8 */ << "\n";
  for (unsigned y = 0; y < mat.height; ++y) {
    for (unsigned x = 0; x < mat.width; ++x) {
      auto pixel = mat.at(y, x);
      file << static_cast<int>(pixel.r) << " " << static_cast<int>(pixel.g)
           << " " << static_cast<int>(pixel.b) << " ";
    }
    file << "\n";
  }
  std::cout << "=== Image saved as " + filename + " ===" << std::endl;
}

} // namespace Ppm

#endif // PPM_WRITER
