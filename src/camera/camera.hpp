#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "mat3x3.hpp"
#include "vec.hpp"
#include <cmath> // M_PI
#include <utility>


struct AABB {
  Vec3f min;
  Vec3f max;
};

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
                      (projected.x >= -plane_width_/2) &&
                      (projected.x < plane_width_/2) &&
                      (projected.y >= -plane_height_/2) &&
                      (projected.y < plane_height_/2);
    return std::make_pair(projected, is_visible);
  }

  // unproject a point from the camera plane to world coordinates
  // the point at the camera plane is at z = -focal_length
  Vec3f UnprojectPlanePoint(float plane_x, float plane_y) const {
    Vec3f p_cam{plane_x, plane_y, focal_length_};
    Mat3x3 R_inv = rot_.Transpose(); // inverse = transpose in this case
    return center_ + R_inv * p_cam;
  }

  // axis-aligned world-space AABB that encloses the rectangle on the
  // camera plane defined by top-left (tlx,tly) and bottom-right (brx,bry).
  AABB FindWorldAABB(int top_left_x, int top_left_y,
                     int bottom_right_x, int bottom_right_y) const {
    Vec3f pTL = UnprojectPlanePoint(static_cast<float>(top_left_x),
                                    static_cast<float>(top_left_y));
    Vec3f pTR = UnprojectPlanePoint(static_cast<float>(bottom_right_x),
                                    static_cast<float>(top_left_y));
    Vec3f pBL = UnprojectPlanePoint(static_cast<float>(top_left_x),
                                    static_cast<float>(bottom_right_y));
    Vec3f pBR = UnprojectPlanePoint(static_cast<float>(bottom_right_x),
                                    static_cast<float>(bottom_right_y));
    Vec3f mn{
      std::min(std::min(pTL.x, pTR.x), std::min(pBL.x, pBR.x)),
      std::min(std::min(pTL.y, pTR.y), std::min(pBL.y, pBR.y)),
      std::min(std::min(pTL.z, pTR.z), std::min(pBL.z, pBR.z))
    };
    Vec3f mx{
      std::max(std::max(pTL.x, pTR.x), std::max(pBL.x, pBR.x)),
      std::max(std::max(pTL.y, pTR.y), std::max(pBL.y, pBR.y)),
      std::max(std::max(pTL.z, pTR.z), std::max(pBL.z, pBR.z))
    };
    return AABB{mn, mx};
  }

  AABB WorldAABB() const {
    int half_w = plane_width_ / 2;
    int half_h = plane_height_ / 2;
    // top-left (x: -half_w, y: +half_h), bottom-right (x: +half_w - 1, y: -half_h)
    return FindWorldAABB(-half_w, half_h - 1, half_w - 1, -half_h);
  }

  // TODO: re-compute world AABB
  void Translate(Vec3f dxdydz) { center_ += dxdydz; } 
  // TODO: re-compute world AABB
  void Rotate(Mat3x3 mat) { rot_ = rot_ * mat; }

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
    // P_C = R(P_W - C)
    return rot_ * (point - center_);
  }
};

#endif // CAMERA_HPP_
