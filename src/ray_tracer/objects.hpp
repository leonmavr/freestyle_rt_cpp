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
  Vec3f NormalAt(const Vec3f &at) const {
    // unit vector starting at the center pointing to intersection point
    return (at - center).Unit();
  }
};




#endif // OBJECTS_HPP_
