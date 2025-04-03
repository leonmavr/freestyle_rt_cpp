#include "rt_math.hpp"
#include <iomanip>
#include <iostream>
#include <vector>

// Function to demonstrate vector operations
void demonstrateVectorOperations() {
  std::cout << "\n===== BASIC VECTOR OPERATIONS =====\n";

  // Create some vectors
  Vec3f v1(1.0f, 2.0f, 3.0f);
  Vec3f v2(4.0f, 5.0f, 6.0f);

  std::cout << "v1 = " << v1 << std::endl;
  std::cout << "v2 = " << v2 << std::endl;

  // Vector arithmetic
  std::cout << "\nVector arithmetic:\n";
  std::cout << "v1 + v2 = " << v1 + v2 << std::endl;
  std::cout << "v1 - v2 = " << v1 - v2 << std::endl;
  std::cout << "v1 * v2 = " << v1 * v2 << " (element-wise)" << std::endl;
  std::cout << "v1 / v2 = " << v1 / v2 << " (element-wise)" << std::endl;

  // Vector-scalar operations
  std::cout << "\nVector-scalar operations:\n";
  std::cout << "v1 * 2.5 = " << v1 * 2.5f << std::endl;
  std::cout << "v1 / 2.0 = " << v1 / 2.0f << std::endl;
  std::cout << "3.0 + v1 = " << 3.0f + v1 << std::endl;
  std::cout << "10.0 - v2 = " << 10.0f - v2 << std::endl;

  // Vector properties
  std::cout << "\nVector properties:\n";
  std::cout << "v1 dot v2 = " << v1.dot(v2) << std::endl;
  std::cout << "v1 cross v2 = " << v1.cross(v2) << std::endl;
  std::cout << "Magnitude of v1 = " << v1.norm() << std::endl;
  std::cout << "Unit vector of v1 = " << v1.unit() << std::endl;
  std::cout << "Angle between v1 and v2 = " << v1.angle(v2) * 180.0f / M_PI
            << " degrees" << std::endl;

  // Reflection
  Vec3f normal(0.0f, 1.0f, 0.0f);     // Normal pointing up along z-axis
  Vec3f incoming(-2.0f, -2.0f, 0.0f); // Incoming vector
  Vec3f reflected = incoming;
  reflected.reflectAbout(normal);

  std::cout << "\nReflection:\n";
  std::cout << "Normal vector = " << normal << std::endl;
  std::cout << "Incoming vector = " << incoming << std::endl;
  std::cout << "Reflected vector = " << reflected << std::endl;
}

// Function to demonstrate matrix operations
void demonstrateMatrixOperations() {
  std::cout << "\n===== MATRIX OPERATIONS =====\n";

  // Create rotation matrices
  Matrix3x3 rotX(M_PI / 4, 0, 0);                   // 45 degrees around X
  Matrix3x3 rotY(0, M_PI / 4, 0);                   // 45 degrees around Y
  Matrix3x3 rotZ(0, 0, M_PI / 4);                   // 45 degrees around Z
  Matrix3x3 combined(M_PI / 4, M_PI / 6, M_PI / 3); // Combined rotation

  std::cout << "Rotation matrix around X (45°):\n" << rotX << std::endl;
  std::cout << "\nRotation matrix around Y (45°):\n" << rotY << std::endl;
  std::cout << "\nRotation matrix around Z (45°):\n" << rotZ << std::endl;
  std::cout << "\nCombined rotation matrix (45°, 30°, 60°):\n"
            << combined << std::endl;

  // Rotate vectors with matrices
  Vec3f unitX(1.0f, 0.0f, 0.0f);
  Vec3f unitY(0.0f, 1.0f, 0.0f);
  Vec3f unitZ(0.0f, 0.0f, 1.0f);

  std::cout << "\nRotating unit vectors:\n";
  std::cout << "X-axis rotated by combined matrix: " << combined * unitX
            << std::endl;
  std::cout << "Y-axis rotated by combined matrix: " << combined * unitY
            << std::endl;
  std::cout << "Z-axis rotated by combined matrix: " << combined * unitZ
            << std::endl;

  // Combine rotations by matrix multiplication
  Matrix3x3 xy = rotY * rotX;         // First rotate around X, then around Y
  Matrix3x3 xyz = rotZ * rotY * rotX; // X, then Y, then Z

  std::cout << "\nCombined rotation matrices through multiplication:\n";
  std::cout << "X then Y rotation:\n" << xy << std::endl;
  std::cout << "\nX then Y then Z rotation:\n" << xyz << std::endl;
}

// Function to simulate a simple particle system with gravity and reflection
void simulateParticleSystem() {
  std::cout << "\n===== PARTICLE SYSTEM SIMULATION =====\n";

  struct Particle {
    Vec3f position;
    Vec3f velocity;
    float mass;
  };

  const int numParticles = 5;
  std::vector<Particle> particles;

  // Initialize particles with random positions and velocities
  srand(static_cast<unsigned int>(time(nullptr)));
  for (int i = 0; i < numParticles; i++) {
    float x = static_cast<float>(rand() % 10) - 5.0f;
    float y = static_cast<float>(rand() % 10) - 5.0f;
    float z = static_cast<float>(rand() % 10);

    float vx = static_cast<float>(rand() % 6) - 3.0f;
    float vy = static_cast<float>(rand() % 6) - 3.0f;
    float vz = static_cast<float>(rand() % 6) - 3.0f;

    particles.push_back({{x, y, z}, {vx, vy, vz}, 1.0f});
  }

  // Simulation parameters
  const Vec3f gravity(0.0f, 0.0f, -9.8f);
  const float timeStep = 0.1f;
  const int numSteps = 10;
  const float groundZ = 0.0f; // Ground plane at z=0
  const Vec3f groundNormal(0.0f, 0.0f, 1.0f);

  // Run simulation
  for (int step = 0; step < numSteps; step++) {
    std::cout << "\nTime step " << step << ":\n";

    for (int i = 0; i < numParticles; i++) {
      Particle &p = particles[i];

      // Print current state
      std::cout << "Particle " << i << ": pos=" << p.position
                << ", vel=" << p.velocity << std::endl;

      // Update velocity (apply gravity)
      p.velocity = p.velocity + gravity * timeStep;

      // Update position
      p.position = p.position + p.velocity * timeStep;

      // Check for ground collision
      if (p.position.z < groundZ) {
        // Move back to ground level
        p.position.z = groundZ;

        // Reflect velocity about ground normal
        p.velocity.reflectAbout(groundNormal);

        // Apply some damping (energy loss)
        p.velocity = p.velocity * 0.8f;
      }
    }
  }
}

// Function to demonstrate 3D transformations
void demonstrate3DTransformations() {
  std::cout << "\n===== 3D TRANSFORMATIONS =====\n";

  // Create a square in the xy-plane
  std::vector<Vec3f> square = {{-1.0f, -1.0f, 0.0f},
                               {1.0f, -1.0f, 0.0f},
                               {1.0f, 1.0f, 0.0f},
                               {-1.0f, 1.0f, 0.0f}};

  std::cout << "Original square vertices:" << std::endl;
  for (const auto &vertex : square) {
    std::cout << vertex << std::endl;
  }

  // Rotate the square around the z-axis by 30 degrees
  Matrix3x3 rotZ(0.0f, 0.0f, M_PI / 6);
  std::vector<Vec3f> rotatedZ;
  for (const auto &vertex : square) {
    rotatedZ.push_back(rotZ * vertex);
  }

  std::cout << "\nSquare after 30° Z rotation:" << std::endl;
  for (const auto &vertex : rotatedZ) {
    std::cout << vertex << std::endl;
  }

  // Then rotate around the x-axis by 45 degrees
  Matrix3x3 rotX(M_PI / 4, 0.0f, 0.0f);
  std::vector<Vec3f> rotatedZX;
  for (const auto &vertex : rotatedZ) {
    rotatedZX.push_back(rotX * vertex);
  }

  std::cout << "\nSquare after 30° Z and 45° X rotation:" << std::endl;
  for (const auto &vertex : rotatedZX) {
    std::cout << vertex << std::endl;
  }

  // Demonstrate using a single combined rotation matrix
  Matrix3x3 combined(M_PI / 4, 0.0f, M_PI / 6); // Same rotations in one matrix
  std::vector<Vec3f> rotatedCombined;
  for (const auto &vertex : square) {
    rotatedCombined.push_back(combined * vertex);
  }

  std::cout << "\nSquare using combined rotation matrix:" << std::endl;
  for (const auto &vertex : rotatedCombined) {
    std::cout << vertex << std::endl;
  }
}

int main() {
  std::cout << std::fixed << std::setprecision(3);

  demonstrateVectorOperations();
  demonstrateMatrixOperations();
  simulateParticleSystem();
  demonstrate3DTransformations();

  return 0;
}
