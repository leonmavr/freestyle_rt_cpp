#ifndef RAY_TRACER_HPP_
#define RAY_TRACER_HPP_


#include "helpers.hpp"
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

  void Trace() {
    lights_.Normalize();
    for (int x = -camera_.width()/2; x < camera_.width()/2; ++x) {
      for (int y = -camera_.height()/2; y < camera_.height()/2; ++y) {
        auto point_world = camera_.Unproject(x, y);
        Ray primary_ray(camera_.center(), point_world);
      
        auto result = TraceRay(primary_ray);      
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
  TraceRecord TraceRay(const Ray& ray, int depth) {
    TraceRecord ret;
    // find nearest intersection
    for (const auto& obj : objects_) {
      auto hit = Intersects(ray, obj);
      // reject hits that are behind the ray's origin
      if (!hit.is_hit || hit.t <= 0.0f) continue;
    
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
    // combine lighting from all sources
    ret.color = lights_.ColorAt(objects_, 
                                *ret.obj,
                                ret.hit_point,
                                camera_);
    if (ret.obj->reflective < 1e-4f || depth <= 1) {
      return ret;
    }

#if 0
    // reflection ray
    Vec3f V = -ray.dir;  // view direction (normalized)
    Vec3f N = ret.normal;
    Vec3f R = (N * 2.0f * N.Dot(V) - V).Unit();  // reflection direction (ensure normalized)
    
    // Offset origin more significantly along reflection direction
    constexpr float OFFSET = 1e-3f;  // slightly larger offset
    Vec3f reflection_origin = ret.hit_point + (N + R) * OFFSET;  // offset along both normal and reflection
    Ray reflection_ray(reflection_origin, reflection_origin + R);

    // Recursively trace reflected ray
    TraceRecord reflection = TraceRay(reflection_ray, depth - 1);

    // Blend direct and reflection based on material reflectivity
    float r = ret.obj->reflective;
    ret.color = Vec3u8{
      static_cast<uint8_t>(ret.color.x * (1-r) + reflection.color.x * r),
      static_cast<uint8_t>(ret.color.y * (1-r) + reflection.color.y * r),
      static_cast<uint8_t>(ret.color.z * (1-r) + reflection.color.z * r)
    };
    
    return ret;
#endif
  }
  const Camera &camera_;
  // TODO: of objects
  std::vector<Sphere> objects_;
  // image buffer to store the final colors
  Image image_;
  Lights& lights_;
};

#endif // RAY_TRACER_HPP_
