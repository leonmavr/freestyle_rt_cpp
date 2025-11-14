#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"

int main() {
  Camera cam(400, 100, 80, {0, 0, -200}, {0.2, -0.2, 0.4});
  
  // Large red sphere in the center back
  Sphere sphere1;
  sphere1.center = {0, 0, 2000};
  sphere1.material.color = {255, 0, 0};
  sphere1.radius = 500;
  sphere1.material.specular = 150;
  sphere1.material.reflective = 0.5;
  sphere1.material.transparency = 0.3;
  sphere1.material.tint = 0.4f;
  
  // Green sphere to the left, slightly forward (will cast shadow on red)
  Sphere sphere2;
  sphere2.center = {-600, -200, 1500};
  sphere2.material.color = {0, 255, 0};
  sphere2.radius = 300;
  sphere2.material.specular = 5;
  sphere2.material.reflective = 0.25;
  
  // Blue sphere to the right, in front (will cast shadow on both)
  Sphere sphere3;
  sphere3.center = {500, 100, 1200};
  //sphere3.material.color = {0, 100, 255};
  sphere3.radius = 250;
  sphere3.material.specular = 20;
  sphere3.material.reflective = 0.3f;
  sphere3.material.transparency = 0.5;
  
  // Small yellow sphere upper left, very forward (small shadow caster)
  Sphere sphere4;
  sphere4.center = {-300, 400, 1000};
  sphere4.material.color = {255, 255, 0};
  sphere4.radius = 150;
  sphere4.material.specular = 20;
  sphere4.material.reflective = 0.6f;
  sphere4.material.transparency = 0.8f;
  sphere4.material.refractive_index = 1.4f;
  
  // Purple sphere lower right, mid-distance
  Sphere sphere5;
  sphere5.center = {400, -300, 1600};
  sphere5.material.color = {200, 0, 200};
  sphere5.radius = 200;
  sphere5.material.specular = 20;
  sphere5.material.reflective = 0.4;
  sphere5.material.transparency = 0.7f;     // mostly transparent
  sphere5.material.refractive_index = 1.5f; // glass

  // Huge sphere at the bottom as a base
  Sphere sphere6;
  sphere6.center = {0, 3600, 2200};
  sphere6.material.color = {180, 190, 200};
  sphere6.radius = 3000;
  sphere6.material.specular = 80;


  
  Lights lights;
  lights.AddAmbient(0.15);  // slightly brighter ambient to see shadowed areas
  lights.AddDir(0.6, -0.1, -0.2, 0.3);  // main directional from upper left
  lights.AddPoint(0.4, -800, 200, -800);  // point light from left front
  lights.AddPoint(0.3, 600, -400, -1000); // softer point light from right
  
  RayTracer ray_tracer(cam, lights);
  ray_tracer.AddObject(sphere1);
  ray_tracer.AddObject(sphere2);
  ray_tracer.AddObject(sphere3);
  ray_tracer.AddObject(sphere4);
  ray_tracer.AddObject(sphere5);
  ray_tracer.AddObject(sphere6);
  
  ray_tracer.Trace(8);
  Ppm::SaveAs(ray_tracer.image(), "output6.ppm");
}
