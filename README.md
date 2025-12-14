# Freestyle raytracer

My first fully working ray tracer running on the CPU.  
I wrote it by relying on various sources and my own intuition, hence the name.

[![C/C++ CI](https://github.com/leonmavr/freestyle_raytracer/actions/workflows/c-cpp.yml/badge.svg?branch=master)](https://github.com/leonmavr/freestyle_raytracer/actions/workflows/c-cpp.yml)

## Features

* Phong lighting model (ambient, diffuse, specular light)
* Shadows
* Reflection and  refraction
* Various solids and shapes (sphere, block, quad, triangle)
* Texture mapping
* Moving camera
* Misc: background image, gamma correction

Future ideas:
- [ ] Render .obj files
- [ ] Custom background color
- [ ] Fuzziness


## Build instructions

You can build this project with or without (by default) textures.

You will need GNU make (`make`) and a C++17 compiler.

Optionally (to render textures), you will need `imagemagick` to convert
the pre-rendered textures to .ppm.

### Without textures

Each demo is found at `src/demoXX.cpp`. The binaries are found at
`build/demoXX`. To target a specific demo, set make's `DEMO` variable
(by default demo00). You can execute:

```
make                 # by default builds demo00
make run             # by default builds and runs demo00
make DEMO=demo01     # change the target to demo01
make run DEMO=demo01 # build and run demo01
```
You can run it manually as:
```
./build/demoXX
```
Demo files also accept command like arguments, such as the camera position and rotation - read their source for more.

Other commands:
```
make clean            # remove objects and executables
make rebuild          # clean and build
```
The outputs are generated as .ppm files.

### With textures

demo00 can be run with texture mapping enabled.
Texture files are large so first you will need to fetch them from git LFS.
To install git LFS if you haven't already:

| Arch-based | Debian-based |
|---|---|
| `pacman -S --noconfirm git-lfs` | `apt-get install -y git-lfs` |

Update LFS and fetch textures:

```bash
git lfs install
git lfs fetch --all
git lfs checkout
```

At this stage, most files will be fetched as jpg/png. You will need `imagemagick` installed 
to convert them to .ppm, so with `imagemagick` installed run:

```
make convert # calls ./scripts/textures2ppm.sh
```
Then you can continue by following the instructions from the previous subsection but with `use-textures` appended to the arguments, i.e.:

```
make run use-textures
```

If you're having trouble, you can refer to how the
[CI pipeline](https://github.com/leonmavr/freestyle_raytracer/blob/master/.github/workflows/c-cpp.yml) does it.

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
5. [D. Sokolov's tinyraytracer](https://github.com/ssloy/tinyraytracer)
