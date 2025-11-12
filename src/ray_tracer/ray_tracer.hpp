#ifndef RAY_TRACER_HPP_
#define RAY_TRACER_HPP_

#include "helpers.hpp"
#include "common.hpp"
#include "objects.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "ray.hpp"
#include "common.hpp"
#include <vector>
#include <limits> // numeric_limits


struct TraceRecord {
  Vec3u8 color{0, 0, 0};      // final color
  bool hit{false};        
  float t{std::numeric_limits<float>::infinity()}; // hit distance
  Vec3f hit_point{};       
  Vec3f normal{};             // surface normal at hit
  const Sphere* obj{nullptr}; // hit object (nullptr if no hit)
};

class RayTracer {
public:
  RayTracer(const Camera& camera, Lights& lights) :
    camera_(camera),
    image_(camera.width(), camera.height()),
    lights_(lights) {}
  // TODO: object
  void AddObject(const Sphere& object) { objects_.push_back(object); }
  Image image() const { return image_; }

  void Trace(int max_reflections = 5) {
    lights_.Normalize();
    for (int x = -camera_.width()/2; x < camera_.width()/2; ++x) {
      for (int y = -camera_.height()/2; y < camera_.height()/2; ++y) {
        auto point_world = camera_.Unproject(x, y);
        Ray ray(camera_.center(), point_world);
        auto result = TraceRay(ray, max_reflections);      
        if (result.hit) {
          const auto w = camera_.width();
          const auto h = camera_.height();
          // map from camera plane to image coords
          int col = Map(x, -w/2, w/2, 0, w-1);
          int row = Map(y, -h/2, h/2, 0, h-1);
          // assuming one ray per pixel -
          // else we need a depth buffer for the closest hit
          image_.at(row, col) = result.color;
        }
      }
    }
  }

private:
  TraceRecord TraceRay(const Ray& ray, int depth, float ior_current = 1.0f) {
    TraceRecord ret;
    // find nearest intersection
    for (const auto& obj : objects_) {
      auto hit = Intersects(ray, obj);
      // reject hits that are behind the ray's origin
      if (!hit.is_hit || hit.t <= 0) continue;
    
      if (hit.t < ret.t) {
        ret.t = hit.t;
        ret.hit = true;
        ret.hit_point = hit.where;
        ret.obj = &obj;
        ret.normal = obj.NormalAt(hit.where);
      }
    }
    if (!ret.hit)
      return ret; // background color and no hit

    float trans = std::clamp(ret.obj->transparency, 0.0f, 1.0f);
    // Direct lighting (surface shading) due diffusion/specular, based
    // on the object's color. Highly transparent objects (>0.5)
    // suppress it so they don't paint themselves.
    Vec3u8 direct = (trans > 0.5f)
                  ? Vec3u8{0,0,0}
                  : lights_.ColorAt(objects_, *ret.obj, ret.hit_point, camera_);

    float refl = std::clamp(ret.obj->reflective, 0.0f, 1.0f);
    // index of refraction of the hit object
    float ior_next = ret.obj->refractive_index;

    // final ray bounce or nothing to reflect/refract
    if (depth <= 1 || (refl < eps && trans < eps)) {
      ret.color = direct;
      return ret;
    }

    //----------------------------------------------------------------
    // Orient the normal and determine n1, n2 for refraction
    //----------------------------------------------------------------
    Vec3f N = ret.normal;
    // incident (pointing away from origin toward surface)
    Vec3f I = ray.dir; 

    auto surrounding_ior = [&](const Vec3f& p, const Sphere* self, const Vec3f& outward_normal) -> float {
      // sample slightly off the outward normal
      Vec3f probe = p + outward_normal * eps * 4.0f;
      float ior = 1.0f; // default is air
      for (const auto& other : objects_) {
        if (&other == self) continue;
        Vec3f pc = probe - other.center;
        // TODO: generalize for other objects
        if (pc.Dot(pc) < other.radius*other.radius) {
          // inside another object
          ior = other.refractive_index;
        }
      }
      return ior;
    };

    // going inside the hit object?
    bool entering = N.Dot(I) < 0.0f;
    Vec3f N_oriented = entering ? N : -N;
    // determine n1 (incident medium) and n2 (transmitted medium)
    float n1 = entering ? surrounding_ior(ret.hit_point, ret.obj, N) : ior_current;
    float n2 = entering ? ior_next : surrounding_ior(ret.hit_point, ret.obj, -N);
    float eta = n1 / n2;
    float cos_i = -N_oriented.Dot(I); // safer to clamp
   
    //----------------------------------------------------------------
    // Schlick reflectance approximation for refraction/reflection
    //----------------------------------------------------------------
    float R0 = (n1 - n2) / (n1 + n2) * (n1 - n2) / (n1 + n2);
    float R_fresnel = R0 + (1.0f - R0) * std::pow(1.0f - cos_i, 5.0f);

    Ray ray_refl({}, {});
    ray_refl.dir = I.ReflectAbout(N_oriented).Unit();
    Vec3f hemi_refl = N_oriented.Dot(ray_refl.dir) > 0 ?
                      N_oriented :
                      -N_oriented;
    // slightly push reflection off the surface to avoid self-intersection
    ray_refl.origin = ret.hit_point + hemi_refl * eps * 4.0f;
    //  -----> child ray (1): reflect for this medium
    Vec3u8 refl_col = TraceRay(ray_refl, depth - 1, n1).color;

    // k := 1 - eta^2 * (1 - cos_i^2) < 0 => total internal reflection
    float k = 1.0f - eta * eta * (1.0f - cos_i * cos_i);
    
    float trans_weight = trans * (1.0f - R_fresnel);
    float refl_weight = refl + R_fresnel * trans;
    bool tir = k < 0.0f;
    //----------------------------------------------------------------
    // refract child ray or do TIR
    //----------------------------------------------------------------
    Vec3u8 refr_color{0, 0, 0};
    if (!tir && trans > eps) {
      float cos_t = std::sqrt(std::max(0.0f, k));
      // vectorized Snell's law for refraction
      Ray refr_ray({}, {});
      refr_ray.dir = (I * eta +
                      N_oriented * (eta * cos_i - cos_t)).Unit();
      refr_ray.origin = ret.hit_point + refr_ray.dir * eps * 4.0f;
      // -----> child ray (2): refract in the next medium
      refr_color = TraceRay(refr_ray, depth - 1, n2).color;
      // tint heuristic (weight) to paint transparent objects
      float tint_w = ret.obj->tint * trans;
      auto apply_tint = [&](uint8_t col_next, uint8_t color_curr)->uint8_t{
        float curr_norm = static_cast<float>(color_curr) / 255.0f;
        float w = (1.0f - tint_w) + tint_w * curr_norm;
        return static_cast<uint8_t>(std::min(255.0f, col_next * w));
      };
      auto color_current = ret.obj->color;
      refr_color = Vec3u8{
        apply_tint(refr_color.x, color_current.x),
        apply_tint(refr_color.y, color_current.y),
        apply_tint(refr_color.z, color_current.z)
      };
    } else if (tir) {
      // all energy goes to reflection if TIR
      trans_weight = 0.0f;
      refl_weight = std::min(1.0f, refl_weight + trans);
    }

    //----------------------------------------------------------------
    // blend direct, reflected and refracted colors
    //----------------------------------------------------------------
    float total = refl_weight + trans_weight;
    // normalize the weights for direct and reflected/refracted light
    float w_direct = 1.0f - std::min(total, 1.0f);
    ret.color = Vec3u8{
      static_cast<uint8_t>(direct.x * w_direct +
                           refl_col.x * refl_weight +
                           refr_color.x * trans_weight),
      static_cast<uint8_t>(direct.y * w_direct +
                           refl_col.y * refl_weight +
                           refr_color.y * trans_weight),
      static_cast<uint8_t>(direct.z * w_direct +
                           refl_col.z * refl_weight +
                           refr_color.z * trans_weight)
    };
    return ret;
  }

  const Camera &camera_;
  std::vector<Sphere> objects_;
  // image buffer to store the final colors
  Image image_;
  Lights& lights_;
};

#endif // RAY_TRACER_HPP_
