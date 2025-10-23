#ifndef LIGHT_HPP_
#define LIGHT_HPP_

#include "vec.hpp"
#include "objects.hpp"
#include "camera.hpp"
#include <vector>
#include <optional>
#include <algorithm>

enum class LightType : int {
    AMBIENT,
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

  // call it having added all lights to weigh their intensities
  void Normalize() {
    float total = 0.0;
    for (const auto &light: lights_) total += light.intensity;
    if (std::abs(total) < 0.001) return;
    for (auto &light: lights_) light.intensity /= total;
  }

  // TODO: generalize to any object
  Vec3u8 ColorAt(const Sphere &sphere,
                 const Vec3f &at,
                 const Camera &camera) const {
    float diffuse_intensity = 0.0;
    float specular_intensity = 0.0;
    Vec3f N = sphere.NormalAt(at);
    Vec3f view_dir = (camera.center() - at).Unit();
    
    // TODO: update comment for specular
    // for ambient and diffusive light, the final intensity is:
    // I = sum_i(I_amb) + sum_j(I_diff_j * dot(L_j, N))
    // for j such that dot(L_j, N) > 0
    // L_i: light direction, N: normal at intersection. See derivation:
    // gabrielgambetta.com/computer-graphics-from-scratch/03-light.html
    for (const auto &light: lights_) {
      if (light.type == LightType::AMBIENT) {
        diffuse_intensity += light.intensity;
      } else {
        Vec3f dir = (light.type == LightType::POINT) 
                  ? (*light.data - at).Unit()
                  : *light.data;
        // diffuse component
        float ndotl = std::max(N.Dot(dir), 0.0f);
        diffuse_intensity += light.intensity * ndotl;
        // specular component
        const bool normal_facing_light = ndotl > 0;
        if (sphere.specular > 0.0f && normal_facing_light) {
          Vec3f reflected = (N * 2.0f * ndotl - dir).Unit();
          float refl_dot_view = std::max(reflected.Dot(view_dir), 0.0f);
          specular_intensity += light.intensity *
                                std::pow(refl_dot_view, sphere.specular);
        }
      }
    }
    // clamp intensities
    diffuse_intensity = std::min(diffuse_intensity, 1.0f);
    specular_intensity = std::min(specular_intensity, 1.0f);
    // combine; diffuse affects object color,
    // specular is modelled as additive white light
    return Vec3u8{
      static_cast<uint8_t>(std::min(sphere.color.x * diffuse_intensity +
                                    255*specular_intensity , 255.0f)),
      static_cast<uint8_t>(std::min(sphere.color.y * diffuse_intensity +
                                    255*specular_intensity , 255.0f)),
      static_cast<uint8_t>(std::min(sphere.color.z * diffuse_intensity +
                                    255*specular_intensity , 255.0f)),
    };
  }

private:
  std::vector<Light> lights_;
};


#endif // LIGHT_HPP_
