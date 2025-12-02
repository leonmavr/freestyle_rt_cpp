# freestyle_rt_cpp

My first fully working ray tracer running on the CPU.

[![C/C++ CI](https://github.com/leonmavr/freestyle_raytracer/actions/workflows/c-cpp.yml/badge.svg?branch=master)](https://github.com/leonmavr/freestyle_raytracer/actions/workflows/c-cpp.yml)


## Features

* Phong lighting model (embient, diffuse, specular light).
* Shadows
* Reflection
* Refraction
* Various shapes (sphere, block, etc.)
* Texture mapping on blocks
* Moving camera

## Build (using Make)

* A C++ compiler supporting C++17
* GNU Make

Build and clean commands:

```bash
# build the demo executable
make
# remove build artifacts and the executable
make clean
# clean then build
make rebuild
```
You can run the executable from the project root as follows.

```bash
./demo
```

The output will be written in a .ppm file, as defined in `src/main.cpp`.

## Gallery

| Output 1 | Output 2 |
|---|---|
| ![Output 1](images/output1.png) | ![Output 2](images/output2.png) |
| ![Output 3](images/output3.png) | ![Output 4](images/output4.png) |

## References

1. [Gambeta's book](https://gabrielgambetta.com/computer-graphics-from-scratch/)
2. [refraction - de Greve](https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf)
3. [Fresnel equations derivation](http://physics.gmu.edu/~ellswort/p263/feqn.pdf)
4. [Pfister's slides](https://www.doc.ic.ac.uk/~dfg/graphics/graphics2010/GraphicsSlides11.pdf)
