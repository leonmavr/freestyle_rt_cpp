#ifndef OBJECTS_HPP_
#define OBJECTS_HPP_

#include "vec.hpp"

struct Object {
  Vec3u8 color{50, 235, 220};
  float specular{20};    // 10 -> matte, 100 -> shiny
  float reflective{0};   // 0 -> non reflective, 1 -> mirror
  float transparency{0}; // how much light transmitted through it 0 to 1
  // how much the material refracts light - 1 not at all, > 1 more
  float refractive_index{1};
  float tint{0.1f};      // color tint for refraction (0 to 0.5)
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
