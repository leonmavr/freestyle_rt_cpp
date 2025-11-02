#ifndef RAY_TRACER_HPP_
#define RAY_TRACER_HPP_


#include "helpers.hpp"
#include "objects.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "ray.hpp"
#include "common.hpp"
#include <vector>
#include <limits>     // numeric_limits
#include <algorithm>  // std::fill



class RayTracer {
public:
  RayTracer(const Camera& camera, Lights& lights) :
    camera_(camera),
    image_(camera.width(), camera.height()),
    depth_(camera.width(), camera.height()),
    lights_(lights) {
      // initialize depth buffer to infinity
      std::fill(depth_.data.begin(), depth_.data.end(),
                std::numeric_limits<double>::infinity());
  }
  // TODO: object
  void AddObject(const Sphere& object) { objects_.push_back(object); }
  Image image() const { return image_; }

  void Trace() {
    lights_.Normalize();
    for (int x = -camera_.width()/2; x < camera_.width()/2; ++x) {
      for (int y = -camera_.height()/2; y < camera_.height()/2; ++y) {
        auto point_world = camera_.Unproject(x, y);
        Ray ray(camera_.center(), point_world);
        for (const auto &object: objects_) {
          auto hit_record = Intersects(ray, object);
          if (hit_record.is_hit) {
            // shadows
            const auto w = camera_.width();
            const auto h = camera_.height();
            auto where_dist = (hit_record.where - camera_.center()).Norm();
            // image and depth buffer indexes
            int col = Map(x, -w/2, w/2, 0, w-1);
            int row = Map(y, -h/2, h/2, 0, h-1);
            if (where_dist < depth_.at(row, col)) {
              depth_.at(row, col) = where_dist;
              auto color = lights_.ColorAt(objects_,
                                           object,
                                           hit_record.where,
                                           camera_);
              image_.at(row, col) = color;
            }
          }
        }
      }
    }
  }

private:
  const Camera &camera_;
  // TODO: of objects
  std::vector<Sphere> objects_;
  // image buffer to store the final colors
  Image image_;
  // depth buffer to record the distance of each hit
  Mat<float> depth_;
  Lights& lights_;
};


#endif // RAY_TRACER_HPP_
