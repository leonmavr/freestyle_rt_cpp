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

  void Trace(int max_reflections = 3) {
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
    if (ret.obj->reflective < eps || depth <= 1) {
      return ret;
    }

    // to view we shoot a ray back to the ray's origin (viewer)
    Vec3f view_dir = -ray.dir;
    Vec3f N = ret.normal;
    // reflect the view direction about the normal and construct a ray
    // out of this
    Ray refl_ray({}, {});
    refl_ray.dir = view_dir.ReflectAbout(N).Unit();
    // without this offset along the normal and reflection we run into
    // self-intersection
    auto offset =  (N + refl_ray.dir) * eps;
    refl_ray.origin = ret.hit_point + offset;
    
    // recurse to compute the next or final intersection and color
    TraceRecord reflection = TraceRay(refl_ray, depth - 1);

    // blend direct and reflection based on material reflectivity -
    // the higher the relfection index, the stronger the primary color
    float r = ret.obj->reflective;
    ret.color = Vec3u8{
      static_cast<uint8_t>(ret.color.x * (1-r) + reflection.color.x * r),
      static_cast<uint8_t>(ret.color.y * (1-r) + reflection.color.y * r),
      static_cast<uint8_t>(ret.color.z * (1-r) + reflection.color.z * r)
    };
    return ret;
  }
  const Camera &camera_;
  // TODO: of objects
  std::vector<Sphere> objects_;
  // image buffer to store the final colors
  Image image_;
  Lights& lights_;
};

#endif // RAY_TRACER_HPP_
