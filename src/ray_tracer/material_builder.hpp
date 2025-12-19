#ifndef MATERIAL_BUILDER_HPP_
#define MATERIAL_BUILDER_HPP_

#include "objects.hpp"
#include <algorithm>

struct MaterialBuilder {
  Vec3u8 color{50, 235, 220};
  float specular{20.0f};
  float reflective{0.0f};
  float transparency{0.0f};
  float refractive_index{1.0f};
  float tint{0.1f};
  Vec3u8 emission{0, 0, 0};
  float emit_strength{0.0f};

  MaterialBuilder() = default;
  MaterialBuilder& Color(uint8_t r, uint8_t g, uint8_t b) {
    color = Vec3u8{r, g, b}; return *this;
  }
  MaterialBuilder& Specular(float s) { specular = s; return *this; }
  MaterialBuilder& Reflective(float r) { reflective = r; return *this; }
  MaterialBuilder& Transparency(float t) { transparency = t; return *this; }
  MaterialBuilder& RefractiveIndex(float i) { refractive_index = i; return *this; }
  MaterialBuilder& Tint(float t) { tint = t; return *this; }
  MaterialBuilder& Emission(uint8_t r, uint8_t g, uint8_t b, float strength) {
    emission = Vec3u8{r, g, b};
    emit_strength = strength;
    return *this;
  }

  Material Build() const {
    return Material{
      .color = color,
      .specular = std::clamp(specular, 0.0f, 250.0f),
      .reflective = std::clamp(reflective, 0.0f, 1.0f),
      .transparency = std::clamp(transparency, 0.0f, 1.0f),
      .refractive_index = std::max(refractive_index, 1.0f),
      .tint = std::clamp(tint, 0.0f, 1.0f),
      .emission = emission,
      .emit_strength = Map(std::clamp(emit_strength, 0.0f, 4.0f)/4,
                           0.0f, 1.0f, 0.0f, 4.0f)
    };
  }

  operator Material() const { return Build(); }
};

#endif // MATERIAL_BUILDER_HPP_
