<div align="center">
<img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/logo.jpg" width="600" />
</div>

<div align="center">
  
[![C/C++ CI](https://github.com/leonmavr/freestyle_raytracer/actions/workflows/c-cpp.yml/badge.svg?branch=master)](https://github.com/leonmavr/freestyle_raytracer/actions/workflows/c-cpp.yml)
</div>

My first fully working ray tracer, running on the CPU.  
I wrote it by relying on various sources without faithfully following any of them, often relying on my own intuition - hence the name.
It does not use physically based light models, only approximations.

## Features

* Phong lighting model (ambient, diffuse, specular light)
* Shadows with softness heuristics
* Reflection and  refraction
* Various solids and shapes (sphere, block, quad, triangle)
* Texture mapping
* Moving camera
* Emissive materials (only spheres currently)
* Misc: background image, gamma correction, minimal rendering of OBJ files

Future ideas:
- [ ] Custom background color
- [ ] Fuzziness
- [ ] Cylinders


## Build instructions

<details>
<summary>Show build instructions</summary>

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

Some demos (demo00, demo04) can run with texture mapping enabled.
Texture files are large so first you will need to fetch them from git LFS.
To install git LFS if you haven't already:

| Arch-based | Debian-based |
|---|---|
| `pacman -S --noconfirm git-lfs` | `apt-get install -y git-lfs` |
| `git lfs install`               | `git lfs install`            |


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
make DEMO=demoXX run use-textures
```

If you're having trouble, you can refer to how the
[CI pipeline](https://github.com/leonmavr/freestyle_raytracer/blob/master/.github/workflows/c-cpp.yml) does it.

### Running the demos and moving the camera

If everything went fine, you will build your binary at `./build/demoXX`. Most
demos are run with optional command like arguments as:

```
./build/demoXX <output_file.ppm> <camera_x> <camera_y> <camera_z>
               <camera_rotation_x_degrees> <cam_rot_y_deg> <cam_rot_z_deg>
```
They will write their output as PPM.

</details>

### Building tips:

1. If you want to gain performance at the expense of resolution,
decrease the focal length or define non-reflective/non-transparent materials.

## How this implementation works

<details>
<summary>Implementation details</summary>

In a nutshell, when a ray hits an object, the resulting color consists of the
sum of direct light `D` (which is modelled as the sum of ambient, diffuse, and
specular light and it's straightforward), the reflectiion, and the refraction.
To compute the reflection and the refraction, we keep a depth counter `d` 
(maximum number of ray bounces) and recursively trace the ray by
reflecting it about the normal and refracting it into the new medium (Snell's
law) respectively, decrementing the depth counter. The reflection, refraction,
and direct lights are weighed and summed to compute the final color.

The excerpt below formulates the idea skipping several details (you can 
ignore the fancy T (tint) operator).

<div align="center">
  <img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/docs/tutorials/ray_tracing_algo/algo_cheat_sheet.png" alt="Algorithm cheat sheet" />
</div>

</details>

More technical information in `docs/tutorials`.

## Demo scenes

| demo name   | description                                                                                                       | textures |
|-------------|-------------------------------------------------------------------------------------------------------------------|----------|
| `demo00`    | Challenging scene with objects floating above a plane, overlap between an object and the plane, various materials, reflectivities and transparencies. Textures can be toggled. | ✓      |
| `demo01`    | Smaller scene with objects enclosed in a huge box; try moving around the camera to spot reflections on the inner walls.                              | ✗        |
| `demo02`    | Minimal .obj file renderer. Handles vertices and faces only, ignoring normals and textures.                      | ✗        |
| `demo03`    | Text rendered as tightly packed spheres.                                                                         | ✗        |
| `demo04`    | Emissive spheres in a randomly generated arrangement of cubes.                                                   | ✓        |


## Gallery

|        |       |   |   |   |
|--------|-------|---|---|---|
| demo00 | <img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo00_textures/out_001.jpg" width="300" /> | <img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo00_textures/out_077.jpg" width="600" /> | <img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo00_textures/out_320.jpg" width="600" /> | <img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo00_textures/out_001_no_textures.jpg" width="600" /> 
| demo01 | <img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo01/output_demo01_00.jpg" width="300" /> |<img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo01/output_demo01_02.jpg" width="600" />  |<img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo01/output_demo01_03.jpg" width="600" />   |   |
| demo02 | <img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo02/out_068.png" width="600" /> |   |   |   |
| demo03 | <img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo03/output.jpg" width="600" />  |   |   |   |
| demo04 | <img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo04/start.jpg" width="600" />   | <img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo04/end.jpg" width="600" /> | *<img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo04/start_no_textures.jpg" width="600" /> | **<img src="https://raw.githubusercontent.com/leonmavr/freestyle_raytracer/refs/heads/master/gallery/demo04/start_magnolia_textures.jpg" width="600" /> |

<sup>* Background color was changed to (97, 24, 0).</sup>

<sup>** The crystal flower (magnolia) is not in the original scene. Found it [here](https://people.sc.fsu.edu/~jburkardt/data/obj/obj.html). </sup>

Below I rendered some precomputed trajectories in the scene of demo00, demo02 and demo04:

https://github.com/user-attachments/assets/fd27e267-274b-4560-ad38-9d659ba376b2

https://github.com/user-attachments/assets/1ba50575-3636-45cb-96cf-993deae79462

https://github.com/user-attachments/assets/03360831-f8e3-4316-8e42-f1d67b59de47


## References

1. [G. Gambeta's "Computer Graphics From Scratch"](https://gabrielgambetta.com/computer-graphics-from-scratch/)
2. [Reflections and Refractions in Ray Tracing - B. de Greve](https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf)
3. [Fresnel equations derivation P263](http://physics.gmu.edu/~ellswort/p263/feqn.pdf)
4. [Pfister's slides](https://www.doc.ic.ac.uk/~dfg/graphics/graphics2010/GraphicsSlides11.pdf)
5. [D. Sokolov's tinyraytracer](https://github.com/ssloy/tinyraytracer)
