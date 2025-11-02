#include "camera.hpp"
#include "light.hpp"
#include "ppm_writer.hpp"
#include "ray_tracer.hpp"
#include "vec.hpp"

int main() {
  Camera cam(400, 100, 80, {0, 0, -200});
  
  // Large red sphere in the center back
  Sphere sphere1;
  sphere1.center = {0, 0, 2000};
  sphere1.color = {255, 0, 0};
  sphere1.radius = 500;
  sphere1.specular = 150;
  
  // Green sphere to the left, slightly forward (will cast shadow on red)
  Sphere sphere2;
  sphere2.center = {-600, -200, 1500};
  sphere2.color = {0, 255, 0};
  sphere2.radius = 300;
  sphere2.specular = 1.2;
  
  // Blue sphere to the right, in front (will cast shadow on both)
  Sphere sphere3;
  sphere3.center = {500, 100, 1200};
  sphere3.color = {0, 100, 255};
  sphere3.radius = 250;
  sphere3.specular = 20;
  
  // Small yellow sphere upper left, very forward (small shadow caster)
  Sphere sphere4;
  sphere4.center = {-300, 400, 1000};
  sphere4.color = {255, 255, 0};
  sphere4.radius = 150;
  sphere4.specular = 5;
  
  // Purple sphere lower right, mid-distance
  Sphere sphere5;
  sphere5.center = {400, -300, 1600};
  sphere5.color = {200, 0, 200};
  sphere5.radius = 200;
  sphere5.specular = 20;
  
  Lights lights;
  lights.AddAmbient(0.15);  // slightly brighter ambient to see shadowed areas
  lights.AddDir(0.6, 0.1, -0.2, 0.3);  // main directional from upper left
  lights.AddPoint(0.4, -800, 200, -800);  // point light from left front
  lights.AddPoint(0.3, 600, -400, -1000); // softer point light from right
  
  RayTracer ray_tracer(cam, lights);
  ray_tracer.AddObject(sphere1);
  ray_tracer.AddObject(sphere2);
  ray_tracer.AddObject(sphere3);
  ray_tracer.AddObject(sphere4);
  ray_tracer.AddObject(sphere5);
  
  ray_tracer.Trace();
  Ppm::SaveAs(ray_tracer.image(), "output4.ppm");
}
