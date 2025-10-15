#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "mat3x3.hpp"
#include "vec.hpp"
#include <cmath> // M_PI

// pinhole camera model
class Camera {
  float Deg2Rad(float deg) { return deg * M_PI / 180; }

public:
  Camera(float focal_length, float fovx_deg, float fovy_deg,
         Vec3f center = {0, 0, 0}, Mat3x3 rot = {})
      : center_(center),
        rot_(rot),
        focal_length_(focal_length),
        plane_height_(2 * focal_length * std::tan(Deg2Rad(fovy_deg) / 2)),
        plane_width_(2 * focal_length * std::tan(Deg2Rad(fovx_deg) / 2)) {}
  unsigned width() const { return plane_width_; }
  unsigned height() const { return plane_height_; }
  // perspective (pinhole) transform
  Vec3i32 Project(Vec3f point) {
    Vec3f point_c = World2Cam(point); // in camera (3D) coords
    // to camera plane (2D)
    return 
        Vec3i32{static_cast<int>(focal_length_ * point_c.x / point_c.z),
                static_cast<int>(focal_length_ * point_c.y / point_c.z),
                0};
  }
  template <typename T> bool IsVisible(Xyz<T> point) {
    Vec3f pc = World2Cam(point);
    // if it's behind the camera it's never visible
    if (pc.z <= 0.0f)
      return false;
    // normalized direction ratios
    float tx = pc.x / pc.z;
    float ty = pc.y / pc.z;
    float half_fovx = std::atan(plane_width_ / (2 * focal_length_));
    float half_fovy = std::atan(plane_height_ / (2 * focal_length_));
    return std::abs(tx) <= std::tan(half_fovx) &&
           std::abs(ty) <= std::tan(half_fovy);
  }

  void Translate(Vec3f vec) {} // TODO
  void Rotate(Mat3x3 mat) {}   // TODO
private:
  // center of projection
  Vec3f center_;
  Mat3x3 rot_;
  float focal_length_;
  unsigned plane_height_;
  unsigned plane_width_;
  template <typename T>
  // world coordinates to camera-centered and rotated coordinates
  Vec3f World2Cam(Xyz<T> point) {
    // P_C = R(P_W - C)
    return rot_ * (point - center_);
  }
};

#endif // CAMERA_HPP_
