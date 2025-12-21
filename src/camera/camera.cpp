#include "camera.hpp"
#include <algorithm>


Camera::Camera(float depth, float fovx_deg, float fovy_deg,
               Vec3f center, Mat3x3 rot)
    : center_(center),
      rot_(rot),
      depth_(depth),
      plane_height_(2 * depth *
                    std::tan(std::abs(Deg2Rad(fovy_deg)) / 2)),
      plane_width_(2 * depth *
                   std::tan(std::abs(Deg2Rad(fovx_deg)) / 2)) {}

// unproject a point from the camera plane to world coordinates
Vec3f Camera::Unproject(float plane_x, float plane_y) const {
  // no need to use the inverse perspective transform as the point is
  // already at z=f - simply recover it from world to camera eq/n
  Vec3f p_cam{plane_x, plane_y, depth_};
  Mat3x3 R_inv = rot_.Transpose(); // inverse = transpose in this case
  return center_ + R_inv * p_cam;
}

// the 4 corners of the image (camera) plane in world coords
std::array<Vec3f, 4> Camera::CornersWorld() const {
  const float hw = static_cast<float>(plane_width_) / 2.0f;
  const float hh = static_cast<float>(plane_height_) / 2.0f;
  Vec3f tl = Unproject(-hw, -hh); // y=-hh => top
  Vec3f tr = Unproject(hw, -hh);
  Vec3f bl = Unproject(-hw, hh);
  Vec3f br = Unproject(hw, hh);
  return {tl, tr, bl, br};
}

// axis-aligned bounding box of the current image plane corners in world space
std::pair<Vec3f, Vec3f> Camera::AABBWorld() const {
  auto c = CornersWorld();
  Vec3f mn = c[0], mx = c[0];
  for (const auto& p : c) {
    mn.x = std::min(mn.x, p.x);
    mn.y = std::min(mn.y, p.y);
    mn.z = std::min(mn.z, p.z);
    mx.x = std::max(mx.x, p.x);
    mx.y = std::max(mx.y, p.y);
    mx.z = std::max(mx.z, p.z);
  }
  return {mn, mx};
}
