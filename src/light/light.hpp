#ifndef LIGHT_HPP_
#define LIGHT_HPP_

#include "vec.hpp"
#include "objects.hpp"
#include <vector>
#include <optional>
#include <algorithm>

enum class LightType : int {
    AMBIENT = 0,
    POINT,
    DIRECTIONAL,
};

struct Light {
    LightType type;
    float intensity;
    // only valid for point/directional light
    std::optional<Vec3f> data{};
};

class Lights {
public:
  void AddAmbient(float intensity) {
    lights_.push_back(Light{.type = LightType::AMBIENT,
                            .intensity = intensity,
                            .data = std::nullopt});
  }
  void AddPoint(float intensity, float posx, float posy, float posz) {
    lights_.push_back(Light{.type = LightType::POINT,
                            .intensity = intensity,
                            .data = Vec3f{posx, posy, posz}});

  }
  void AddDir(float intensity, float dirx, float diry, float dirz) {
    lights_.push_back(Light{.type = LightType::DIRECTIONAL,
                            .intensity = intensity,
                            .data = Vec3f{dirx, diry, dirz}.Unit()});
  }

  // to be called after having added all lights
  void Normalize() {
    float total = 0.0;
    for (const auto &light: lights_) total += light.intensity;
    if (std::abs(total) < 0.001) return;
    for (auto &light: lights_) light.intensity /= total;
  }

  // TODO: generalize to any object
  Vec3u8 ColorAt(const Sphere &sphere, const Vec3f &at) const {
    float intensity = 0.0;
    for (const auto &light: lights_) {
      if (light.type == LightType::AMBIENT) {
        intensity += light.intensity;
      } else {
        Vec3f dir = (light.type == LightType::POINT) ? (*light.data - at).Unit()
                                                     : *light.data;
          intensity += light.intensity *
                       std::max(sphere.NormalAt(at).Dot(dir), 0.0f);
      }
    }

    return Vec3u8{static_cast<uint8_t>(sphere.color.x * intensity),
                  static_cast<uint8_t>(sphere.color.y * intensity),
                  static_cast<uint8_t>(sphere.color.z * intensity)}; 
  }


private:
  std::vector<Light> lights_;
};


#endif // LIGHT_HPP_
