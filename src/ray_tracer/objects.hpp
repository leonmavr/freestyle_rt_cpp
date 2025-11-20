#ifndef OBJECTS_HPP_
#define OBJECTS_HPP_

#include "vec.hpp"
#include "mat3x3.hpp"
#include "ray.hpp"
#include "common.hpp"
#include <limits> // std::numeric_limits
#include <array>
#include <utility>

//---------------------------------------------------------------------
// Helper structs
//---------------------------------------------------------------------
struct Triangle {
  Vec3f v0, v1, v2;
  bool IsInTriangle(const Vec3f& point) const {
    // barycentric coordinates method
    // ref: https://users.csc.calpoly.edu/~zwood/teaching/csc471/2017F/barycentric.pdf
    Vec3f u = v1 - v0;
    Vec3f v = v2 - v0;
    Vec3f w = point - v0;

    float uu = u.Dot(u);
    float uv = u.Dot(v);
    float vv = v.Dot(v);
    float wu = w.Dot(u);
    float wv = w.Dot(v);
    float denom = uv * uv - uu * vv;
    if (std::abs(denom) < eps) return false; // degenerate triangle

    // is a point p is in triangle, it can be written as
    // p = (1 - s - t) * v0 + s * v1 + t * v2
    // with s,t >= 0 and s + t <= 1
    float s = (uv * wv - vv * wu) / denom;
    float t = (uv * wu - uu * wv) / denom;
    return (s >= 0) && (t >= 0) && (s + t <= 1);
  }

  std::tuple<Vec3f, bool> RayTriangleIntersection(const Ray& ray) const {
    const auto none = std::make_tuple(Vec3f{}, false);
    // Moller-Trumbore intersection algorithm
    // ref: https://web.engr.oregonstate.edu/~mjb/vulkan/Handouts/RayTriangleIntersection.1pp.pdf
    Vec3f edge1 = v1 - v0;
    Vec3f edge2 = v2 - v0;
    Vec3f h = ray.dir.Cross(edge2);
    float a = edge1.Dot(h);
    if (std::abs(a) < eps)
      return none; // ray is parallel to triangle

    float f = 1.0f / a;
    Vec3f s = ray.origin - v0;
    float u = f * s.Dot(h);
    if (u < 0.0f || u > 1.0f)
      return none;

    Vec3f q = s.Cross(edge1);
    float v = f * ray.dir.Dot(q);
    if (v < 0.0f || u + v > 1.0f)
      return none;
    float t = f * edge2.Dot(q);
    if (t > eps) { // ray intersection
      return std::make_tuple(ray.origin + ray.dir * t, true);
    } else { // that there is a line intersection but not a ray intersection
      return none;
    }
  }
};


//---------------------------------------------------------------------
// Objects to render
//---------------------------------------------------------------------
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
  // only to be overriden for closed solids
  virtual bool IsInside(const Vec3f &point) const { return false;};
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
  Block(Vec3f center, float half_x, float half_y, float half_z,
        const Mat3x3& mrot = {}) : rot(mrot),
                                   half_w(half_x),
                                   half_h(half_y),
                                   half_d(half_z) {
    // CCW normalized vertices
    const int v[8][3] = {
      {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
      {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1},
    };
    for (int i = 0; i < 8; ++i) {
      Vec3f vscaled(v[i][0] * half_x, v[i][1] * half_y, v[i][2] * half_z);
      vertices[i] = center + rot * vscaled;
    }
    axisx = (rot * Vec3f{1,0,0}).Unit();
    axisy = (rot * Vec3f{0,1,0}).Unit();
    axisz = (rot * Vec3f{0,0,1}).Unit();
  }

  virtual Vec3f NormalAt(const Vec3f &at) const override {
      // TODO: find the outward normal at the nearest plane,
      // take into account the whether we're inward or outward of the plane
  }
  virtual bool IsInside(const Vec3f &point) const override {
    
    return true;
  }
  virtual HitRecord Intersects(const Ray& ray) const override {
    // TODO: for each face, define 2 triangles and check 
    // if the point is in each of them
    return {};
  }
  std::array<Vec3f, 4> vertices;
  Vec3f axisx, axisy, axisz;
  Mat3x3 rot;
  float half_w, half_h, half_d;
};

#endif // OBJECTS_HPP_
