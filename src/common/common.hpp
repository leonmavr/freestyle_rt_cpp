#ifndef COMMON_HPP
#define COMMON_HPP

#include "vec.hpp"
#include <stdexcept>
#include <vector>

// error tolerance
inline constexpr float eps = 1e-3f;
// how much to extend a normal to avoid self-intesection/probe other
// materials; N becomes (1 + eps_normal)N
inline constexpr float eps_normal = 5e-2f;

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
  Mat() : width(0), height(0), data() {}
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
  std::vector<T> data;
};

using Image = Mat<Vec3u8>;

#endif  // COMMON_HPP
