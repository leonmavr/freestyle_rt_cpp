#ifndef RAY_TRACER_HPP_
#define RAY_TRACER_HPP_


#include "vec.hpp"
#include "mat3x3.hpp"
#include "camera.hpp"
#include "ray.hpp"
#include <vector>

struct Sphere {
    Vec3f origin; 
    float rad;
};

class RayTracer {
public:
    RayTracer(const Camera& camera) : camera_(camera) {}
    // TODO: object
    void AddObject(const Sphere& object) {
        objects_.push_back(object);
    }

    void Trace() {
        for (int x = -camera_.width()/2; x < camera_.width()/2; ++x) {
            for (int y = -camera_.width()/2; y < camera_.width()/2; ++y) {

            }
        }
    }

private:
    const Camera &camera_;
    // TODO: of objects
    std::vector<Sphere> objects_;
};


#endif // RAY_TRACER_HPP_
