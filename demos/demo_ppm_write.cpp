#include "ppm_writer.hpp"
#include "common.hpp"
#include <iostream>

int main() {
  const unsigned width = 256;
  const unsigned height = 256;
  Mat<Rgb> image(width, height);

  for (unsigned y = 0; y < height; ++y) {
    uint8_t value = static_cast<uint8_t>((y * 255) / (height - 1));
    for (unsigned x = 0; x < width; ++x) {
      image.at(y, x) = Rgb{value, 0, value}; // grayscale vertical gradient
    }
  }

  try {
    Ppm::SaveMat("gradient.ppm", image);
    std::cout << "=== Output written to gradient.ppm ===" << std::endl;
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }

  return 0;
}

