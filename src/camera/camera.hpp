#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "mat3x3.hpp"
#include "vec.hpp"
#include "common.hpp"
#include <cmath> // M_PI
#include <utility>
#include <array>


// Perspective projection camera model with a constant field of view
// (FOV).
// The higher the camera depth (z of the camera plane), the higher the
// resolution of the projected image.
class Camera {
public:
  Camera(float depth, float fovx_deg, float fovy_deg,
         Vec3f center = {0, 0, 0}, Mat3x3 rot = {});
  void Translate(Vec3f dxdydz) { center_ += dxdydz; }
  void Rotate(float dx_rad, float dy_rad, float dz_rad) {
    Mat3x3 mat(dx_rad, dy_rad, dz_rad);
    rot_ = mat * rot_;
  }

  // unproject a point from the camera plane to world coordinates
  Vec3f Unproject(float plane_x, float plane_y) const;
  // the 4 corners of the image (camera) plane in world coords
  std::array<Vec3f, 4> CornersWorld() const;
  // axis-aligned bounding box of the current image plane corners in
  // world space
  std::pair<Vec3f, Vec3f> AABBWorld() const;
  float depth() const { return depth_; }
  std::size_t width() const { return plane_width_; }
  std::size_t height() const { return plane_height_; }
  Vec3f center() const { return center_; }

private:
  // center of projection
  Vec3f center_;
  Mat3x3 rot_;
  float depth_;
  std::size_t plane_height_;
  std::size_t plane_width_;

  template <typename T>
  // world coordinates to camera-centered and rotated coordinates
  Vec3f World2Cam(Xyz<T> point) {
    // P_c = R(P_W - C)
    return rot_ * (point - center_);
  }
};

#endif // CAMERA_HPP_
