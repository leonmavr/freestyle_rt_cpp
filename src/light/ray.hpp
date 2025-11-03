#ifndef RAY_HPP_
#define RAY_HPP_

#include "vec.hpp"

// light ray modelled parametrically as r(t) = origin + dir * t
// , where dir = unit(target - origin) is the normalized direction
// => given an object at `target`, at distance d = ||target-origin||
// t = 0 maps to origin
// t = d maps to the object
struct Ray {
    Ray(const Vec3f &from, const Vec3f &to) {
      dir = (to - from).Unit();
      origin = from;
    }
    Vec3f At(float t) { return origin + dir*t; }

    Vec3f origin;
    Vec3f dir;
};

#endif // RAY_HPP_
