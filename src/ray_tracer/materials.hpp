#ifndef MATERIALS_HPP_
#define MATERIALS_HPP_

#include "objects.hpp"

namespace Materials {

inline Material Plastic() {
  Material m{};
  m.color = {255, 255, 255};
  m.specular = 20.0f;
  m.reflective = 0.05f;
  m.transparency = 0.0f;
  m.refractive_index = 1.0f;
  m.tint = 0.0f;
  return m;
}

inline Material Glass() {
  Material m{};
  m.color = {255, 255, 255};
  m.specular = 80.0f;
  m.reflective = 0.12f;
  m.transparency = 0.7f;
  m.refractive_index = 1.5f;
  m.tint = 0.1f;
  return m;
}

inline Material Crystal() {
  Material m{};
  m.color = {255, 255, 255};
  m.specular = 90.0f;
  m.reflective = 0.2f;
  m.transparency = 0.85f;
  m.refractive_index = 1.7f;
  m.tint = 0.1f;
  return m;
}

inline Material Marble() {
  Material m{};
  m.color = {255, 255, 255};
  m.specular = 10.0f;
  m.reflective = 0.1f;
  m.transparency = 0.0f;
  m.refractive_index = 1.0f;
  m.tint = 0.0f;
  return m;
}

} // namespace

#endif // MATERIALS_HPP_
