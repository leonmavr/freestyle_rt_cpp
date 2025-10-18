#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "mat3x3.hpp"
#include "vec.hpp"
#include <cmath> // M_PI
#include <utility>

// pinhole camera model
class Camera {
  float Deg2Rad(float deg) { return deg * M_PI / 180; }

public:
  Camera(float focal_length, float fovx_deg, float fovy_deg,
         Vec3f center = {0, 0, 0}, Mat3x3 rot = {})
      : center_(center),
        rot_(rot),
        focal_length_(focal_length),
        plane_height_(2 * focal_length *
                      std::tan(std::abs(Deg2Rad(fovy_deg)) / 2)),
        plane_width_(2 * focal_length *
                     std::tan(std::abs(Deg2Rad(fovx_deg)) / 2)) {}
  unsigned width() const { return plane_width_; }
  unsigned height() const { return plane_height_; }

  // perspective (pinhole) transform
  std::pair<Vec3i32, bool> Project(Vec3f point) {
    Vec3f point_c = World2Cam(point); // in camera (3D) coords
    // to camera plane (2D)
    auto projected =  
        Vec3i32{static_cast<int>(focal_length_ * point_c.x / point_c.z),
                static_cast<int>(focal_length_ * point_c.y / point_c.z),
                0};
    bool is_visible = (point.z > 0) && // invisible behind the camera
                      (projected.x >= 0) &&
                      (projected.x < plane_width_) &&
                      (projected.y >= 0) &&
                      (projected.y < plane_height_);
    return std::make_pair(projected, is_visible);
  }

  void Translate(Vec3f dxdydz) { center_ += dxdydz; } 
  void Rotate(Mat3x3 mat) { rot_ = rot_ * mat; }

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
