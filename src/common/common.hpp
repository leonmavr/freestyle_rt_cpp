#ifndef COMMON_HPP
#define COMMON_HPP

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <iostream>

struct Rgb {
  uint8_t r, g, b;
};

template <typename T>
T Lerp(T from, T to, float t) { return (1 - t)*from + t*to; }

// linearly map x from range [a, b] to [c, d]
template <typename T>
T Map(T x, T a, T b, T c, T d) {
  if (a == b || c == d) [[unlikely]]
    return x;
  return c + (d - c) * (x - a) / (b - a);
}

template <typename T>
struct Mat {
  std::vector<T> data;
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

  unsigned width;
  unsigned height;
};

using Image = Mat<Rgb>;

#endif  // COMMON_HPP
