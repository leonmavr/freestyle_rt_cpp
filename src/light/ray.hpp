#ifndef RAY_HPP_
#define RAY_HPP_

#include "vec.hpp"

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
