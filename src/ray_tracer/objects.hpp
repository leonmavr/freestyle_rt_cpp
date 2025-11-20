#ifndef OBJECTS_HPP_
#define OBJECTS_HPP_

#include "vec.hpp"
#include "mat3x3.hpp"
#include "ray.hpp"
#include <limits> // std::numeric_limits
#include <array>

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

struct Block : Object {
  Block(Vec3f center, float half_x, float half_y, float half_z, const Mat3x3& mrot = {}) : rot(mrot) {
    const int v[8][3] = {
      {-1, -1, -1},
      { 1, -1, -1},
      { 1,  1, -1},
      {-1,  1, -1},
      {-1, -1,  1},
      { 1, -1,  1},
      { 1,  1,  1},
      {-1,  1,  1},
    };

  for (int i = 0; i < 8; ++i) {
    Vec3f scaled( v[i][0] * half_x, v[i][1] * half_y, v[i][2] * half_z);
    vertices[i] = center + rot * scaled;
  }

    axisx = (rot * Vec3f{1,0,0}).Unit();
    axisy = (rot * Vec3f{0,1,0}).Unit();
    axisz = (rot * Vec3f{0,0,1}).Unit();

    // These are also face normals
    normals[0] =  axisx;
    normals[1] = -axisx;
    normals[2] =  axisy;
    normals[3] = -axisy;
    normals[4] =  axisz;
    normals[5] = -axisz;
  }

  virtual Vec3f NormalAt(const Vec3f &at) const override {
    // at origin, no orientation
    Vec3f aligned = rot.Transpose() * (p - center);

    // which face is hit
    float ax = std::fabs(aligned.x) - hx;
    float ay = std::fabs(aligned.y) - hy;
    float az = std::fabs(aligned.z) - hz;

    if (std::fabs(ax) < 1e-4f && std::fabs(aligned.x) > std::fabs(aligned.y) && std::fabs(aligned.x) > std::fabs(aligned.z))
        return (aligned.x > 0 ? axisx : -axisx);

    if (std::fabs(ay) < 1e-4f && std::fabs(aligned.y) > std::fabs(aligned.x) && std::fabs(aligned.y) > std::fabs(aligned.z))
        return (aligned.y > 0 ? axisy : -axisy);

    // else Z face
    return (aligned.z > 0 ? axisz : -axisz);
  }
  virtual bool IsInside(const Vec3f &point) const override {
    // TODO
    return true;
  }
  virtual HitRecord Intersects(const Ray& ray) const override {
    // TODO
   /*
    * The parametric line of a ray from from the origin O through
    * point B ('end' of the ray) is:
    * R(t) = O + t(B - O) = tB
    * This ray meets the plane for some t=t0 such that:
    * R(t0) = B*t0
    * Therefore R(t0) validates the equation of the plane.
    * For the plane we know the normal vector n and the offset
    * from the origin d. Any point X on the plane validates its
    * equation, which is:
    * n.X = d
    * Since R(t0) lies on the plane:
    * n.R(t0) = d =>
    * n.B*t0 = d =>
    * t0 = d/(n.B)
    * Finally, the ray meets the plane at point
    * R(t0) = (d/(n.B))*B
    */
    return {};
  }
  std::array<Vec3f, 4> vertices;
  std::array<Vec3f, 6> normals;
  Vec3f axisx, axisy, axisz;
  Mat3x3 rot;
};

#endif // OBJECTS_HPP_
