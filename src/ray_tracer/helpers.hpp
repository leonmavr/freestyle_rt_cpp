#ifndef HELPERS_HPP_
#define HELPERS_HPP_

#include "vec.hpp"
#include "objects.hpp"
#include "ray.hpp"
#include "common.hpp" // eps
#include <limits> // std::numeric_limits

// simple hit record in world coordinates between a ray and an object
struct HitRecord {
    Vec3f where{};
    bool is_hit{false};
    // distance from ray origin to intersection point
    float t{std::numeric_limits<float>::infinity()};
};

HitRecord 
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

  float discriminant = b*b - 4*a*c;
  does_intersect = discriminant > 0;
  if (!does_intersect)
    return HitRecord{}; // empty record - no intersection

  float sqrt_disc = std::sqrt(discriminant);
  float t1 = (-b - sqrt_disc) / (2 * a);
  float t2 = (-b + sqrt_disc) / (2 * a);

  // nearest positive intersection - both must be in front of the ray
  does_intersect |= (t1 > 0) || (t2 > 0);
  if (!does_intersect)
    return HitRecord{};  // empty record - no intersection
  float tmin = std::numeric_limits<float>::infinity();
  tmin = (t1 > 0 && t2 > 0) ? std::min(t1, t2) :
         (t1 > 0 ? t1 :
         (t2 > 0 ? t2 : tmin));
  where = O + D * tmin;
  return {where, does_intersect, tmin};
}

#endif // HELPERS_HPP_
