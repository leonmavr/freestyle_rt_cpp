#include "ray_tracer.hpp"
#include <algorithm>

RayTracer::RayTracer(const Camera& camera, Lights& lights) :
  camera_(camera),
  image_(camera.width(), camera.height()),
  lights_(lights),
  has_background_(false) {}

const Image& RayTracer::image() { return image_; }

void RayTracer::Trace(int max_reflections) {
  lights_.Normalize();
  // current camera plane corners (world-space)
  auto corners = camera_.CornersWorld();
  // local camera axes in world space for rasterization
  Vec3f world_tl = corners[0];
  Vec3f world_tr = corners[1];
  Vec3f world_bl = corners[2];
  // horizontal (u) and vertical (v) world span vectors,
  // centerd at origin
  Vec3f span_h = world_tr - world_tl;
  Vec3f span_v = world_bl - world_tl;
  // TODO: cam_forward_, cam_right_, cam_up_ are only needed for
  // background sampling -> move them there
  // unit forward, right, and up unprojection span vectors (world)
  cam_forward_ = (camera_.Unproject(0, 0) - camera_.center()).Unit();
  cam_right_ = (world_tr - world_tl).Unit();
  cam_up_ = (world_bl - world_tl).Unit();
  int w = camera_.width();
  int h = camera_.height();
  for (int col = 0; col < w; ++col) {
    // normalized column coordinate
    float u = static_cast<float>(col) / static_cast<float>(w - 1);
    for (int row = 0; row < h; ++row) {
      float v = static_cast<float>(row) / static_cast<float>(h - 1);
      // bilinear point on the (possibly rotated) image plane
      Vec3f point_world = world_tl + span_h * u + span_v * v;
      Ray ray(camera_.center(), point_world);
      auto result = TraceRay(ray, max_reflections);
      // Always write the color; TraceRay fills background on miss
      image_.at(row, col) = result.color;
    }
  }
}

// gamma < 1 brightens the low intensitites of the image,
// gamma > 1 darkens the high intensities
void RayTracer::GammaCorrect(float gamma) {
  for (int row = 0; row < image_.height; ++row) {
    for (int col = 0; col < image_.width; ++col) {
      Vec3u8& px = image_.at(row, col);
      px.x = static_cast<uint8_t>(std::pow(px.x / 255.0f, gamma)
                                  * 255.0f);
      px.y = static_cast<uint8_t>(std::pow(px.y / 255.0f, gamma)
                                  * 255.0f);
      px.z = static_cast<uint8_t>(std::pow(px.z / 255.0f, gamma)
                                  * 255.0f);
    }
  }
}

void RayTracer::ReadBackground(const std::string& filename) {
  Ppm::Read(background_, filename);
  has_background_ = (background_.width > 0 && background_.height > 0);
}

float RayTracer::SurroundingIOR(const Vec3f& where,
                                const Object* self,
                                const Vec3f& outward_normal) const {
  Vec3f probe = where + outward_normal * eps * eps_factor;
  float ret = 1.0f; // default is air
  for (const auto& other : objects_) {
    if (other.get() == self) continue;
    if (other->IsInside(probe)) {
      ret = other->material.refractive_index;
    }
  }
  return ret;
}

RayTracer::OrientationInfo RayTracer::RayOrientation(const TraceRecord& record,
                                                     const Vec3f& I,
                                                     float ior_current) const {
  OrientationInfo ret;
  ret.entering = record.normal.Dot(I) < 0;
  // normal; pointing away from  the refracted medium
  ret.normal = ret.entering ? record.normal : -record.normal;
  float n_obj = record.obj->material.refractive_index;
  float n_surrounding = SurroundingIOR(record.hit_point,
                                       record.obj,
                                       record.normal);
  ret.n1 = !ret.entering ? n_surrounding : ior_current;
  ret.n2 = !ret.entering ? n_obj : ior_current;
  ret.eta = ret.n1 / ret.n2;
  ret.cos_i = -ret.normal.Dot(I);
  return ret;
}

std::pair<bool, Vec3f> RayTracer::Refract(const Vec3f &incident,
                                          const Vec3f &N,
                                          float eta,
                                          float cosi) const {
  //cosi = -N.Dot(incident);
  float k = 1.0f - eta * eta * (1.0f - cosi * cosi);
  if (k < 0.0f) return {true, {}}; // TIR
  float cost = std::sqrt(std::max(0.0f, k));
  // formula for trasnmitted (refracted) ray
  Vec3f trans = (incident * eta + N * (eta * cosi - cost)).Unit();
  return {false, trans};
}

float RayTracer::Schlick(float n1, float n2, float cos_i) const {
  float r0 = (n1 - n2) / (n1 + n2);
  r0 *= r0;
  return r0 + (1.0f - r0) * std::pow(1.0f - cos_i, 5.0f);
}

TraceRecord RayTracer::TraceRay(const Ray& ray, int depth, float ior_current) const {
  TraceRecord ret;
  // find nearest intersection
  for (const auto& obj : objects_) {
    auto hit = obj->Intersects(ray);
    // reject hits that are behind the ray's origin
    if (!hit.is_hit || hit.t <= 0) continue;

    if (hit.t < ret.t) {
      ret.t = hit.t;
      ret.hit = true;
      ret.hit_point = hit.where;
      ret.obj = obj.get();
      ret.normal = obj->NormalAt(hit.where);
    }
  }
  if (!ret.hit) {
    // no hit; return background color contribution
    if (has_background_) ret.color = SampleBackground(ray.dir);
    return ret;
  }

  // material parameters
  float mat_trans = std::clamp(ret.obj->material.transparency,
                               0.0f, 1.0f);
  float mat_refl = std::clamp(ret.obj->material.reflective,
                              0.0f, 1.0f);

  // direct lighting (surface shading) from diffuse and specular light
  Vec3u8 direct = lights_.ColorAt(objects_, *ret.obj, ret.hit_point,
                                  camera_);
  // transparency reduces the brightness
  direct.x *= (1 - mat_trans);
  direct.y *= (1 - mat_trans);
  direct.z *= (1 - mat_trans);

  // no need for more ray bounces (base case), only direct ligting 
  if (depth <= 0 || (mat_refl < eps && mat_trans < eps)) {
    ret.color = direct;
    return ret;
  }

  // incident (pointing away from origin, against the normal)
  Vec3f inc = ray.dir; 
  auto ori = RayOrientation(ret, inc, ior_current);
  // cos_i refers to the angle between normal and incident
  float n1 = ori.n1, n2 = ori.n2, eta = ori.eta, cos_i = ori.cos_i;

  //------------------------------------------------------------------
  // reflection
  //------------------------------------------------------------------
  Ray ray_refl{{}, {}};
  ray_refl.dir = inc.ReflectAbout(ori.normal);
  // offset slightly along the normal to avoid self-intersection
  ray_refl.origin = ret.hit_point + ori.normal * eps * eps_factor;
  // -----> child ray (1): reflection
  Vec3u8 refl_col = TraceRay(ray_refl, depth - 1, n1).color;

  //------------------------------------------------------------------
  // refraction 
  //------------------------------------------------------------------
  // Fresnel blending via Schlik approximation for the weights
  float R_fresnel = Schlick(n1, n2, cos_i);
  // (1 - R) * trans = fraction that got trasnmitted
  float trans_weight = mat_trans * (1.0f - R_fresnel);
  // R * trans = fraction that got reflected instead of refracted
  float refl_weight = mat_refl + R_fresnel * mat_trans;
  bool tir = false;
  Vec3u8 refr_color{0, 0,0};
  if (mat_trans > eps) {
    auto [is_tir, refr_dir] = Refract(inc, ori.normal, eta, cos_i);
    tir = is_tir;
    if (!tir) {
      Ray refr_ray{{}, {}};
      refr_ray.origin = ret.hit_point + refr_dir * eps * eps_factor;
      refr_ray.dir = refr_dir;
      // -----> child ray (2): refraction
      refr_color = TraceRay(refr_ray, depth - 1, n2).color;
      // tint heuristic: w = (1 - tint_w) + tint_w * (color / 255)
      float tint_w = ret.obj->material.tint * mat_trans;
      auto ApplyTint = [&](uint8_t col_next,
                           uint8_t color_curr)->uint8_t{
        float curr_norm = static_cast<float>(color_curr) / 255.0f;
        float w = (1.0f - tint_w) + tint_w * curr_norm;
        return static_cast<uint8_t>(std::min(255.0f, col_next * w));
      };
      auto color_current = ret.obj->material.color;
      refr_color = Vec3u8{
        ApplyTint(refr_color.x, color_current.x),
        ApplyTint(refr_color.y, color_current.y),
        ApplyTint(refr_color.z, color_current.z)
      };
    } else {
      // total internal reflection:
      // send transparency energy into reflection
      trans_weight = 0.0f;
      refl_weight = std::min(1.0f, refl_weight + mat_trans);
    }
  }

  float total = refl_weight + trans_weight;
  float w_direct = 1.0f - std::min(total, 1.0f);
  ret.color = Vec3u8{
    static_cast<uint8_t>(direct.x * w_direct +
                         refl_col.x * refl_weight +
                         refr_color.x * trans_weight),
    static_cast<uint8_t>(direct.y * w_direct +
                         refl_col.y * refl_weight +
                         refr_color.y * trans_weight),
    static_cast<uint8_t>(direct.z * w_direct +
                         refl_col.z * refl_weight +
                         refr_color.z * trans_weight)
  };
  return ret;
}

// Compute a far sample world point along the ray from the current
// camera center,
// -> map that point's two spherical angles with wrapping
// -> map (scale) those those to background pixel coordinates
Vec3u8 RayTracer::SampleBackground(const Vec3f& dir_world) const {
  if (!has_background_) return {0,0,0};
  // heuristic: choose a point n*focal_length times away in the
  // world's direction to set it as parallax; the larger the n,
  // the less sensitive the background to translation
  const float z_parallax = 15 * camera_.focal_length();
  Vec3f sample_world = camera_.center() + dir_world * z_parallax;
  // Convert the world point to spherical angles relative to world origin.
  float x = sample_world.x;
  float y = sample_world.y;
  float z = sample_world.z;
  // yaw in [-pi, pi], pitch in [-pi/2, pi/2]
  float yaw = std::atan2(x, z);
  float pitch = std::atan2(y, std::sqrt(x*x + z*z));
  // map to [0,1) x [0,1] with horizontal wrap
  float u = (yaw + static_cast<float>(M_PI)) / (2.0f * static_cast<float>(M_PI));
  u = std::fmod(u, 1.0f);
  if (u < 0.0f) u += 1.0f;
  float v = (pitch + static_cast<float>(M_PI) * 0.5f) / static_cast<float>(M_PI);
  v = std::clamp(v, 0.0f, 1.0f);
  // convert to pixel indices (nearest neighbor)
  unsigned w = background_.width;
  unsigned h = background_.height;
  unsigned px = std::min(static_cast<unsigned>(u * w), w ? w - 1 : 0);
  unsigned py = std::min(static_cast<unsigned>(v * h), h ? h - 1 : 0);
  return background_.at(py, px);
}
