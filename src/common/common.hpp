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
