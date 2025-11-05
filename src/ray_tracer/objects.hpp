#ifndef OBJECTS_HPP_
#define OBJECTS_HPP_

#include "vec.hpp"

struct Object {
  Vec3u8 color{50, 235, 220};
  float specular{20}; // specular = 10 -> matte, specular = 100 -> shiny
  float reflective{0}; // 0 -> non reflective, 1 -> mirror
};

struct Sphere : Object {
  Vec3f center; 
  float radius;
  // assuming it's on the sphere
  Vec3f NormalAt(const Vec3f &at) const {
    return (at - center).Unit();
  }
};


#endif // OBJECTS_HPP_
