#ifndef OBJECTS_HPP_
#define OBJECTS_HPP_

#include "vec.hpp"
#include "mat3x3.hpp"
#include "ray.hpp"
#include "common.hpp"
#include <limits> // std::numeric_limits
#include <array>


// simple hit record in world coordinates between a ray and an object
struct HitRecord {
  Vec3f where{};
  bool is_hit{false};
  // distance from ray origin to intersection point
  float t{std::numeric_limits<float>::infinity()};
};

//---------------------------------------------------------------------
// Helper objects
//---------------------------------------------------------------------
struct Triangle {
  Vec3f v0, v1, v2;

  HitRecord Intersect(const Ray& ray) const {
    // Moller-Trumbore ray-triangle intersection algorithm
    // notation: TODO link my tutorial
    // ref: web.engr.oregonstate.edu/~mjb/vulkan/Handouts/RayTriangleIntersection.1pp.pdf
    Vec3f edge1 = v1 - v0;
    Vec3f edge2 = v2 - v0;
    // solve the system [edge1, edge2, -ray.dir]^T[u, v, t] = 
    //                  [ray.origin, -V0] 
    // using Cramer's method and the properties of mixed product
    Vec3f h = ray.dir.Cross(edge2);
    float a = edge1.Dot(h);
    if (std::abs(a) < eps)
      return {}; // ray is parallel to triangle

    float ainv = 1 / a;
    Vec3f s = ray.origin - v0;
    // barycetric coordinate 0..1 of a point along edge 1
    float u = ainv * s.Dot(h);
    if (u < 0 || u > 1)
      return {};

    Vec3f q = s.Cross(edge1);
    // barycetric coordinate 0..1 of a point along edge 2
    float v = ainv * ray.dir.Dot(q);
    if (v < 0 || u + v > 1)
      return {};
    float t = ainv * edge2.Dot(q);
    if (t > eps) { // ray intersection
      return {ray.origin + ray.dir * t, true, t};
    } else { // intersection lies behind the ray's origin 
      return {};
    }
  }
};

struct Quad {
  Vec3f v0, v1, v2, v3;
  HitRecord Intersect(const Ray& ray) const {
    std::array<HitRecord, 2> hits;
    Triangle t1{v0, v1, v2};
    Triangle t2{v0, v2, v3};
    hits[0] = t1.Intersect(ray);
    hits[1] = t2.Intersect(ray);
    if (hits[0].is_hit)
      return hits[0];
    if (hits[1].is_hit)
      return hits[1];
    return {};
  }
};

//---------------------------------------------------------------------
// Objects to render
//---------------------------------------------------------------------

struct Material {
  Vec3u8 color{50, 235, 220};
  float specular{20};    // 10 -> matte, 100 -> shiny
  float reflective{0};   // 0..1, 0 -> non reflective, 1 -> mirror
  float transparency{0}; // how much light transmitted through it 0..1
  // how much the material refracts light - 1 not at all, > 1 more
  float refractive_index{1};
  float tint{0.1f};      // color tint for refraction (0..1)
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
        const Mat3x3& mrot = {}) : half_w(half_x),
                                   half_h(half_y),
                                   half_d(half_z),
                                   rot(mrot) {
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
    // TODO: find the 6 quads of the block, intrsect with it,
    // return nearest intersection
    return {};
  }
  std::array<Vec3f, 4> vertices;
  Vec3f axisx, axisy, axisz;
  float half_w, half_h, half_d;
  Mat3x3 rot;
};

#endif // OBJECTS_HPP_
