#ifndef RAY_TRACER_HPP_
#define RAY_TRACER_HPP_

#include "common.hpp"
#include "objects.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "ray.hpp"
#include "common.hpp"
#include <vector>
#include <memory>
#include <limits> // numeric_limits


struct TraceRecord {
  Vec3u8 color{0, 0, 0};      // final color
  bool hit{false};        
  float t{std::numeric_limits<float>::infinity()}; // hit distance
  Vec3f hit_point{};          // where the hit occured
  Vec3f normal{};             // surface normal at hit
  const Object* obj{nullptr}; // hit object (nullptr if no hit)
};

class RayTracer {
public:
  RayTracer(const Camera& camera, Lights& lights) :
    camera_(camera),
    image_(camera.width(), camera.height()),
    lights_(lights) {}
  template <typename T>
  void AddObject(T&& obj) {
    // with std::forward to preserve rvalue/lvalue nature
    objects_.push_back(std::make_unique<T>(std::forward<T>(obj)));
  }
  const Image& image() { return image_; }

  void Trace(int max_reflections = 5) {
    lights_.Normalize();
    // current camera plane corners (world-space)
    auto corners = camera_.CornersWorld();
    // local camera axes in world space for rasterization
    Vec3f tl = corners[0];
    Vec3f tr = corners[1];
    Vec3f bl = corners[2];
    // horizontal (u) and vertical (v) world span vectors
    Vec3f span_h = tr - tl;
    Vec3f span_v = bl - tl;
    int w = camera_.width();
    int h = camera_.height();
    for (int col = 0; col < w; ++col) {
      // normalized column coordinate
      float u = static_cast<float>(col) / static_cast<float>(w - 1);
      for (int row = 0; row < h; ++row) {
        float v = static_cast<float>(row) / static_cast<float>(h - 1);
        // bilinear point on the (possibly rotated) image plane
        Vec3f point_world = tl + span_h * u + span_v * v;
        Ray ray(camera_.center(), point_world);
        auto result = TraceRay(ray, max_reflections);
        if (result.hit)
          image_.at(row, col) = result.color;
      }
    }
  }

  // gamma < 1 brightens the low intensitites of the image,
  // gamma > 1 darkens the high intensities
  void GammaCorrect(float gamma) {
    for (int row = 0; row < image_.height; ++row) {
      for (int col = 0; col < image_.width; ++col) {
        Vec3u8& px = image_.at(row, col);
        px.x = static_cast<uint8_t>(std::pow(px.x / 255.0f, gamma)
                                    * 255.0f);
        px.y = static_cast<uint8_t>(std::pow(px.y / 255.0f, gamma)
                                    * 255.0f);
        px.z = static_cast<uint8_t>(std::pow(px.z / 255.0f, gamma)
                                    * 255.0f);
      }
    }
  }


private:
  // get the corrent IOR (index of refraction) and normal arrangement
  // for refraction calculations
  struct OrientationInfo {
    bool entering{true};
    // oriented normal, i.e. pointing away from refracted medium
    Vec3f normal{};
    float n1{1.0f};
    float n2{1.0f};
    float eta{1.0f};
    float cos_i{0.0f};
  };

  // probe the refractive index of the surrounding medium slightly off
  // the surface
  float SurroundingIOR(const Vec3f& where,
                       const Object* self,
                       const Vec3f& outward_normal) const {
    Vec3f probe = where + outward_normal * eps * eps_factor;
    float ret = 1.0f; // default is air
    for (const auto& other : objects_) {
      if (other.get() == self) continue;
      if (other->IsInside(probe)) {
        ret = other->material.refractive_index;
      }
    }
    return ret;
  }

  // determine normal orientation and the IOR (index of refraction)
  // pair, given that the normal ray should point towadds the incident
  // plane
  OrientationInfo RayOrientation(const TraceRecord& record, 
                                 const Vec3f& I,
                                 float ior_current) const {
    OrientationInfo ret;
    ret.entering = record.normal.Dot(I) < 0;
    // normal; pointing away from  the refracted medium
    ret.normal = ret.entering ? record.normal : -record.normal;
    float n_obj = record.obj->material.refractive_index;
    float n_surrounding = SurroundingIOR(record.hit_point,
                                         record.obj,
                                         record.normal);
    ret.n1 = !ret.entering ? n_surrounding : ior_current;
    ret.n2 = !ret.entering ? n_obj : ior_current;
    ret.eta = ret.n1 / ret.n2;
    ret.cos_i = -ret.normal.Dot(I);
    return ret;
  }

  // returns refracted vector (Snell's vectorized law) and whether TIR
  // occused
  std::pair<bool, Vec3f>
  Refract(const Vec3f &incident, /* incident ray direction */
          const Vec3f &N,        /* normal at intersection */
          float eta,             /* relative refr index n1/n2 */
          float cosi             /* cos of incident ray */) const {
    //cosi = -N.Dot(incident);
    float k = 1.0f - eta * eta * (1.0f - cosi * cosi);
    if (k < 0.0f) return {true, {}}; // TIR
    float cost = std::sqrt(std::max(0.0f, k));
    // formula for trasnmitted (refracted) ray
    Vec3f trans = (incident * eta + N * (eta * cosi - cost)).Unit();
    return {false, trans};
  };

  // Schlick's approximation of Fresnel reflectance
  float Schlick(float n1, float n2, float cos_i) const {
    float r0 = (n1 - n2) / (n1 + n2);
    r0 *= r0;
    return r0 + (1.0f - r0) * std::pow(1.0f - cos_i, 5.0f);
  }

  TraceRecord TraceRay(const Ray& ray, int depth, float ior_current = 1.0f) const {
    TraceRecord ret;
    // find nearest intersection
    for (const auto& obj : objects_) {
      auto hit = obj->Intersects(ray);
      // reject hits that are behind the ray's origin
      if (!hit.is_hit || hit.t <= 0) continue;
    
      if (hit.t < ret.t) {
        ret.t = hit.t;
        ret.hit = true;
        ret.hit_point = hit.where;
        ret.obj = obj.get();
        ret.normal = obj->NormalAt(hit.where);
      }
    }
    if (!ret.hit)
      return ret; // background color and no hit

    // material parameters
    float mat_trans = std::clamp(ret.obj->material.transparency,
                                 0.0f, 1.0f);
    float mat_refl = std::clamp(ret.obj->material.reflective,
                                0.0f, 1.0f);

    // direct lighting (surface shading) from diffuse and specular light
    Vec3u8 direct = lights_.ColorAt(objects_, *ret.obj, ret.hit_point,
                                    camera_);
    // transparency reduces the brightness
    direct.x *= (1 - mat_trans);
    direct.y *= (1 - mat_trans);
    direct.z *= (1 - mat_trans);

    // no need for more ray bounces (base case), only direct ligting 
    if (depth <= 0 || (mat_refl < eps && mat_trans < eps)) {
      ret.color = direct;
      return ret;
    }

    // incident (pointing away from origin, against the normal)
    Vec3f inc = ray.dir; 
    auto ori = RayOrientation(ret, inc, ior_current);
    // cos_i refers to the angle between normal and incident
    float n1 = ori.n1, n2 = ori.n2, eta = ori.eta, cos_i = ori.cos_i;

    //------------------------------------------------------------------
    // reflection
    //------------------------------------------------------------------
    Ray ray_refl{{}, {}};
    ray_refl.dir = inc.ReflectAbout(ori.normal);
    // offset slightly along the normal to avoid self-intersection
    ray_refl.origin = ret.hit_point + ori.normal * eps * eps_factor;
    // -----> child ray (1): reflection
    Vec3u8 refl_col = TraceRay(ray_refl, depth - 1, n1).color;

    //------------------------------------------------------------------
    // refraction 
    //------------------------------------------------------------------
    // Fresnel blending via Schlik approximation for the weights
    float R_fresnel = Schlick(n1, n2, cos_i);
    // (1 - R) * trans = fraction that got trasnmitted
    float trans_weight = mat_trans * (1.0f - R_fresnel);
    // R * trans = fraction that got reflected instead of refracted
    float refl_weight = mat_refl + R_fresnel * mat_trans;
    bool tir = false;
    Vec3u8 refr_color{0, 0,0};
    if (mat_trans > eps) {
      auto [is_tir, refr_dir] = Refract(inc, ori.normal, eta, cos_i);
      tir = is_tir;
      if (!tir) {
        Ray refr_ray{{}, {}};
        refr_ray.origin = ret.hit_point + refr_dir * eps * eps_factor;
        refr_ray.dir = refr_dir;
        // -----> child ray (2): refraction
        refr_color = TraceRay(refr_ray, depth - 1, n2).color;
        // tint heuristic: w = (1 - tint_w) + tint_w * (color / 255)
        float tint_w = ret.obj->material.tint * mat_trans;
        auto ApplyTint = [&](uint8_t col_next,
                             uint8_t color_curr)->uint8_t{
          float curr_norm = static_cast<float>(color_curr) / 255.0f;
          float w = (1.0f - tint_w) + tint_w * curr_norm;
          return static_cast<uint8_t>(std::min(255.0f, col_next * w));
        };
        auto color_current = ret.obj->material.color;
        refr_color = Vec3u8{
          ApplyTint(refr_color.x, color_current.x),
          ApplyTint(refr_color.y, color_current.y),
          ApplyTint(refr_color.z, color_current.z)
        };
      } else {
        // total internal reflection:
        // send transparency energy into reflection
        trans_weight = 0.0f;
        refl_weight = std::min(1.0f, refl_weight + mat_trans);
      }
    }

    float total = refl_weight + trans_weight;
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
  std::vector<std::unique_ptr<Object>> objects_;
  // image buffer to store the final colors
  Image image_;
  Lights& lights_;
  // how much to scale the eps of a normal to avoid self-intersection
  static constexpr const float eps_factor = 40.0;
};

#endif // RAY_TRACER_HPP_
