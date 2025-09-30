#ifndef COMMON_HPP
#define COMMON_HPP

#include <cstdint>
#include <stdexcept>
#include <vector>

struct Rgb {
  uint8_t r, g, b;
};

template <typename T>
struct Mat {
  unsigned width, height;
  std::vector<T> data;
  Mat(unsigned) = delete;
  Mat(unsigned w, unsigned h) : width(w), height(h), data(w * h) {}

  T& at(unsigned row, unsigned col) {
    if (row >= height || col >= width) [[unlikely]]
      throw std::out_of_range("Mat::at(row, col): index out of bounds");
    return data[row * width + col];
  }
  const T& at(unsigned row, unsigned col) const {
    if (row >= height || col >= width) [[unlikely]]
      throw std::out_of_range("Mat::at(row, col): index out of bounds");
    return data[row * width + col];
  }
};

#endif  // COMMON_HPP
