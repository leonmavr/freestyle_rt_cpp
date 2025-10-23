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
  int width() const { return plane_width_; }
  int height() const { return plane_height_; }
  Vec3f center() const { return center_; }

  // perspective (pinhole) transform
  std::pair<Vec3i32, bool> Project(Vec3f point) {
    Vec3f point_c = World2Cam(point); // in camera (3D) coords
    // to camera plane (2D)
    auto projected =  
        Vec3i32{static_cast<int>(focal_length_ * point_c.x / point_c.z),
                static_cast<int>(focal_length_ * point_c.y / point_c.z),
                0};
    bool is_visible = (point_c.z > 0) && // invisible behind the camera
                      (projected.x >= -plane_width_/2) &&
                      (projected.x < plane_width_/2) &&
                      (projected.y >= -plane_height_/2) &&
                      (projected.y < plane_height_/2);
    return std::make_pair(projected, is_visible);
  }

  // unproject a point from the camera plane to world coordinates
  Vec3f Unproject(float plane_x, float plane_y) const {
    // no need to use the inverse perspective transform as the point is
    // already at z=f - simply recover it from world to camera eq/n 
    Vec3f p_cam{plane_x, plane_y, focal_length_};
    Mat3x3 R_inv = rot_.Transpose(); // inverse = transpose in this case
    return center_ + R_inv * p_cam;
  }

  void Translate(Vec3f dxdydz) { center_ += dxdydz; } 
  void Rotate(float dx_rad, float dy_rad, float dz_rad) { 
    Mat3x3 mat(dx_rad, dy_rad, dz_rad);
    rot_ = mat * rot_;
  }

private:
  // center of projection
  Vec3f center_;
  Mat3x3 rot_;
  float focal_length_;
  int plane_height_;
  int plane_width_;

  template <typename T>
  // world coordinates to camera-centered and rotated coordinates
  Vec3f World2Cam(Xyz<T> point) {
    // P_c = R(P_W - C)
    return rot_ * (point - center_);
  }
};

#endif // CAMERA_HPP_
