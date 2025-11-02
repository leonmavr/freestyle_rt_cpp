#ifndef LIGHT_HPP_
#define LIGHT_HPP_

#include "ray.hpp"
#include "helpers.hpp"
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

  // In your ColorAt function, add shadow checking before computing lighting

Vec3u8 ColorAt(const std::vector<Sphere>& objects,
               const Sphere &sphere,
               const Vec3f &at,
               const Camera &camera) const {
  float diffuse_intensity = 0.0;
  float specular_intensity = 0.0;
  Vec3f N = sphere.NormalAt(at);
  Vec3f view_dir = (camera.center() - at).Unit();
  
  for (const auto &light: lights_) {
    if (light.type == LightType::AMBIENT) {
      diffuse_intensity += light.intensity;
      continue; // ambient light isn't affected by shadows
    }
    
    // check for shadows before computing diffuse/specular
    bool in_shadow = false;
    if (light.type == LightType::POINT) {
      // shadow ray from intersection to light source
      Ray shadow_ray(at, *light.data);
      float light_distance = (*light.data - at).Norm();
      
      for (const auto &obj : objects) {
        // skip self-intersection
        if (&obj == &sphere) continue;
        HitRecord hit = Intersects(shadow_ray, obj);
        // check if object blocks the light (0 < t < light_distance)
        // i.e. object is between surface and the point source
        if (hit.is_hit && hit.t > 1e-4 && hit.t < light_distance) {
          in_shadow = true;
          break;
        }
      }
    } else if (light.type == LightType::DIRECTIONAL) {
      Ray shadow_ray(at, {0, 0, 0});
      // shadow ray has direction opposite to the light's
      shadow_ray.dir = -*light.data;
      
      for (const auto &obj : objects) {
        if (&obj == &sphere) continue;
        HitRecord hit = Intersects(shadow_ray, obj);
        // for directional lights, any hit with t > 0 means shadow
        if (hit.is_hit && hit.t > 0) {
          in_shadow = true;
          break;
        }
      }
    }
    // skip diffuse/specular accumulation if in shadow
    if (in_shadow) continue;
    
    // diffuse light direction 
    Vec3f dir = (light.type == LightType::POINT) 
              ? (*light.data - at).Unit()
              : *light.data;
    
    float ndotl = std::max(N.Dot(dir), 0.0f);
    diffuse_intensity += light.intensity * ndotl;
    const bool normal_facing_light = ndotl > 0;
    if (sphere.specular > 0.0f && normal_facing_light) {
      Vec3f reflected = (N * 2.0f * ndotl - dir).Unit();
      float refl_dot_view = std::max(reflected.Dot(view_dir), 0.0f);
      specular_intensity += light.intensity *
                            std::pow(refl_dot_view, sphere.specular);
    }
  }
  
  diffuse_intensity = std::min(diffuse_intensity, 1.0f);
  specular_intensity = std::min(specular_intensity, 1.0f);
  
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
