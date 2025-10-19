#ifndef COMMON_HPP
#define COMMON_HPP

#include <cstdint>
#include <stdexcept>
#include <vector>

struct Rgb {
  uint8_t r, g, b;
};

struct Image {
  std::vector<Rgb> data;
  Image(unsigned w, unsigned h) : width(w), height(h), data(w * h) {}

  Rgb& at(unsigned row, unsigned col) {
    if (row >= height || col >= width) [[unlikely]]
      throw std::out_of_range("Mat::at(row, col): index out of bounds");
    return data[row * width + col];
  }
  const Rgb& at(unsigned row, unsigned col) const {
    if (row >= height || col >= width) [[unlikely]]
      throw std::out_of_range("Mat::at(row, col): index out of bounds");
    return data[row * width + col];
  }

  unsigned width;
  unsigned height;
};

#endif  // COMMON_HPP
