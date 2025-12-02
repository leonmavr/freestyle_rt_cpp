#ifndef COMMON_HPP
#define COMMON_HPP

#include "vec.hpp"
#include <stdexcept>
#include <vector>

// error tolerance
const float eps = 1e-3f;

template <typename T>
T Lerp(T from, T to, float t) { return (1 - t)*from + t*to; }

// linearly map x from range [a, b] to [c, d]
template <typename T>
T Map(T x, T a, T b, T c, T d) {
  if (a == b || c == d) [[unlikely]]
    return x;
  return c + (d - c) * (x - a) / (b - a);
}

static float Deg2Rad(float deg) { return deg * M_PI / 180; }

template <typename T>
struct Mat {
  Mat() : data(), width(0), height(0) {}
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

  std::vector<T> data;
  unsigned width;
  unsigned height;
};

using Image = Mat<Vec3u8>;

#endif  // COMMON_HPP
