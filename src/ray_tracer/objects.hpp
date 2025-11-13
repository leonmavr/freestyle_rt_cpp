#ifndef OBJECTS_HPP_
#define OBJECTS_HPP_

#include "vec.hpp"
#include "ray.hpp"
#include <limits> // std::numeric_limits

// simple hit record in world coordinates between a ray and an object
struct HitRecord {
  Vec3f where{};
  bool is_hit{false};
  // distance from ray origin to intersection point
  float t{std::numeric_limits<float>::infinity()};
};

struct Material {
  Vec3u8 color{50, 235, 220};
  float specular{20};    // 10 -> matte, 100 -> shiny
  float reflective{0};   // 0 -> non reflective, 1 -> mirror
  float transparency{0}; // how much light transmitted through it 0 to 1
  // how much the material refracts light - 1 not at all, > 1 more
  float refractive_index{1};
  float tint{0.1f};      // color tint for refraction (0 to 0.5)
};

struct Object {
  virtual Vec3f NormalAt(const Vec3f &at) const = 0;
  virtual bool IsInside(const Vec3f &point) const = 0;
  virtual HitRecord Intersects(const Ray& ray) const = 0;
  Vec3f center;
  Material material;
};

struct Sphere : Object {
  float radius;
  // assuming it's on the sphere
  virtual Vec3f NormalAt(const Vec3f &at) const override {
    return (at - center).Unit();
  }
  virtual bool IsInside(const Vec3f &point) const override {
    return (point - center).Dot(point - center) < radius * radius;
  }
  virtual HitRecord Intersects(const Ray& ray) const override {
    HitRecord ret; // empty by default (no intersection)
    // notation of ray-sphere intersection formula
    auto C = center;
    auto r = radius;
    auto L = ray.origin - C;
    float a = ray.dir.Dot(ray.dir);
    float b = 2 * ray.dir.Dot(L);
    float c = L.Dot(L) - r * r;

    float discriminant = b*b - 4*a*c;
    ret.is_hit = discriminant > 0;
    if (!ret.is_hit)
      return ret;

    float sqrt_disc = std::sqrt(discriminant);
    float t1 = (-b - sqrt_disc) / (2 * a);
    float t2 = (-b + sqrt_disc) / (2 * a);

    // nearest positive intersection - both must be in front of the ray
    ret.is_hit |= (t1 > 0) || (t2 > 0);
    if (!ret.is_hit)
      return ret;
    float tmin = std::numeric_limits<float>::infinity();
    ret.t = (t1 > 0 && t2 > 0) ? std::min(t1, t2) :
            (t1 > 0 ? t1 :
            (t2 > 0 ? t2 : tmin));
    ret.where = ray.origin + ray.dir * ret.t;
    return ret;
  }
};

#endif // OBJECTS_HPP_
