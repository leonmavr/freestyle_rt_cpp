#ifndef OBJECTS_HPP_
#define OBJECTS_HPP_

#include "vec.hpp"

struct Object {
  Vec3u8 color;
  float specular;        // specular = 10 -> matte, specular = 100 -> shiny
  float reflective;      // 0 -> non reflective, 1 -> mirror
};

struct Sphere : Object {
  Vec3f center; 
  float radius;
  // assuming it's on the sphere
  Vec3f NormalAt(const Vec3f &at, bool at_origin = true) const {
    Vec3f normal = (at - center).Unit();
    if (at_origin)
      return normal;
    else
      return at + normal * 1e-4f; // tiny bit above the intersection 
  }
};


#endif // OBJECTS_HPP_
