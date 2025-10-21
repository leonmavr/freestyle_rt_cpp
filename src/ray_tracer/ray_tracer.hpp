#ifndef RAY_TRACER_HPP_
#define RAY_TRACER_HPP_


#include "vec.hpp"
#include "camera.hpp"
#include "ray.hpp"
#include "common.hpp"
#include <vector>
#include <utility>

struct Sphere {
  Vec3f center; 
  float radius;
};


static std::tuple<Vec3f, bool>
Intersects(const Ray& ray, const Sphere& sphere) {
  Vec3f where;
  bool does_intersect = false;

  // ray-sphere intersection formula
  auto O = ray.origin;
  auto D = ray.dir;
  auto C = sphere.center;
  auto r = sphere.radius;
  auto L = O - C;
  float a = D.Dot(D);
  float b = 2 * D.Dot(L);
  float c = L.Dot(L) - r * r;

  float discriminant = b * b - 4 * a * c;
  does_intersect = discriminant > 0;
  if (!does_intersect)
    return std::make_pair(where, does_intersect);

  float sqrt_disc = std::sqrt(discriminant);
  float t1 = (-b - sqrt_disc) / (2 * a);
  float t2 = (-b + sqrt_disc) / (2 * a);
  float tmin = 0;

  // nearest positive intersection - both must be in front of the ray
  does_intersect |= (t1 > 0) || (t2 > 0);
  if (!does_intersect)
    return std::make_pair(where, does_intersect);
  tmin = (t1 > 0 && t2 > 0) ? std::min(t1, t2) :
         (t1 > 0 ? t1 :
         (t2 > 0 ? t2 : tmin));
  where = O + D * tmin;
  return std::make_pair(where, does_intersect);
}


class RayTracer {
public:
  RayTracer(const Camera& camera) : camera_(camera),
                                    image_(camera.width(),
                                           camera.height()) {}
  // TODO: object
  void AddObject(const Sphere& object) { objects_.push_back(object); }
  Image image() const { return image_; }

  void Trace() {
    for (int x = -camera_.width()/2; x < camera_.width()/2; ++x) {
      for (int y = -camera_.width()/2; y < camera_.width()/2; ++y) {
        auto point_world = camera_.Unproject(x, y);
        Ray ray(camera_.center(), point_world);
        for (const auto &object: objects_) {
          auto [where, intersects] = Intersects(ray, object);
          if (intersects) {
            const auto w = camera_.width();
            const auto h = camera_.height();
            image_.at(Map(y, -h/2, h/2, 0, h-1),
                      Map(x, -w/2, w/2, 0, w-1)) = {255, 0, 0};
            // TODO: for each light source, compute the color 
          }
        }
      }
    }
  }

private:
  const Camera &camera_;
  // TODO: of objects
  std::vector<Sphere> objects_;
  Image image_;
};


#endif // RAY_TRACER_HPP_
