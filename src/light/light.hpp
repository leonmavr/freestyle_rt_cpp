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
    
      // check for shadows before computing diffuse/specular component
      float shadow_harshness = ShadowFactor(light, objects,
                                            sphere, at, N);
      if (shadow_harshness < 1e-4)
          continue; // fully occluded - save computation time
    
      // diffuse light direction 
      Vec3f dir = (light.type == LightType::POINT) 
                ? (*light.data - at).Unit()
                : *light.data;
    
      // ref: 
      // gabrielgambetta.com/computer-graphics-from-scratch/03-light.html
      float ndotl = std::max(N.Dot(dir), 0.0f);
      diffuse_intensity += light.intensity * ndotl * shadow_harshness;
      const bool normal_facing_light = ndotl > 0;
      if (sphere.specular > 0.0f && normal_facing_light) {
        Vec3f reflected = (N * 2.0f * ndotl - dir).Unit();
        float refl_dot_view = std::max(reflected.Dot(view_dir), 0.0f);
        specular_intensity += light.intensity *
                              std::pow(refl_dot_view, sphere.specular) *
                              shadow_harshness;
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

  static float ShadowFactor(const Light& light,
                            const std::vector<Sphere>& objects,
                            const Sphere& sphere,
                            const Vec3f& at,
                            const Vec3f& normal) {
    constexpr float EPS = 1e-2f;
    // TODO: offset along shadow dir to avoid shadow when enclused?
    // shift up the origin a bit - avoid self-intersection (shadow acne)
    Vec3f origin = at + normal * EPS;
    constexpr float bright_min = 0.0, bright_max = 1.0;
    float ret = bright_max; // no shadow

    if (light.type == LightType::POINT) {
      // shadow ray is directed from intersection to light source
      Ray shadow_ray(origin, *light.data);
      float light_distance = (*light.data - origin).Norm();
      
      float nearest_t = std::numeric_limits<float>::infinity();
      bool any_hit = false;
      for (const auto &obj : objects) {
        // skip self-intersection
        if (&obj == &sphere) continue;
        HitRecord hit = Intersects(shadow_ray, obj);
        // check if object blocks the light (0 < t < light_distance)
        // i.e. object is between surface and the point source
        if (hit.is_hit && hit.t > 0 && hit.t < light_distance) {
          any_hit = true;
          if (hit.t < nearest_t) nearest_t = hit.t;
        }
      }
      if (!any_hit)
        return bright_max;

      // Shadow brightness heuristic;
      // for brightness = 1 we have no shadow, for 0 it's fully dark.
      // Use (1) the distance to source 
      // and (2) the relative direction between the normal and the
      // direction to source to compute a brightness factor within [0,1]
      Vec3f dir_to_light = (*light.data - origin).Unit();
      // how perpendicular the ray is to the normal (2)
      float ndotl = std::clamp(normal.Dot(dir_to_light), 0.0f, 1.0f);
      // normalized distance from target to source
      float u = std::clamp(nearest_t / light_distance, 0.0f, 1.0f);
      ret = ndotl * u;
    } else if (light.type == LightType::DIRECTIONAL) {
      Ray shadow_ray {origin, {0, 0, 0}};
      // shadow ray has travels in the opposite direction as the light
      shadow_ray.dir = -*light.data;
      
      // if the shadow ray intersects another object, cast a shadow
      // for directional lights, any hit with t > 0 means shadow
      bool any_hit = std::any_of(objects.begin(), objects.end(),
                     [&](const auto& obj) {
                       auto hit = Intersects(shadow_ray, obj);
                       return &obj != &sphere &&
                       hit.is_hit &&
                       hit.t > 0;
                    });
      if (!any_hit)
        return bright_max;

      const Vec3f light_dir = *light.data;
      // same as before, however use only part (2) of the heuristic
      float ndotl = std::clamp(normal.Dot(light_dir), 0.0f, 1.0f);
      ret = ndotl;
    }
    return std::clamp(ret, bright_min, bright_max);
  }

};

#endif // LIGHT_HPP_

