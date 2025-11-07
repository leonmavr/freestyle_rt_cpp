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
  TraceRecord TraceRay(const Ray& ray, int depth) {
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
    // direct lighting at this hit
    Vec3u8 direct = lights_.ColorAt(objects_, *ret.obj, ret.hit_point, camera_);

    if (depth <= 1 || ret.obj->reflective <= eps) {
      ret.color = direct;
      return ret;
    }

    // viewer direction; shoot the ray back to the viewer
    Vec3f view_dir = -ray.dir;
    Vec3f N = ret.normal;
    Ray child_ray({}, {});
    child_ray.dir = view_dir.ReflectAbout(N).Unit();
    // child ray origin: push outward slighly above the hemisphere
    Vec3f hemi_normal = (N.Dot(child_ray.dir) > 0 ? N : -N);
    // without this offset, the child ray runs into self-intersection
    auto offset = (hemi_normal + view_dir) * eps * 4; // eps * n to reduce acne
    child_ray.origin = ret.hit_point + offset;

    TraceRecord child = TraceRay(child_ray, depth - 1);

    // linear (Fresnel) blending of direct color and reflection
    float r = Map(ret.obj->reflective, 0.0f, 1.0f, 0.05f, 0.95f);
    ret.color = Vec3u8{
      static_cast<uint8_t>(direct.x * (1 - r) + child.color.x * r),
      static_cast<uint8_t>(direct.y * (1 - r) + child.color.y * r),
      static_cast<uint8_t>(direct.z * (1 - r) + child.color.z * r)
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
