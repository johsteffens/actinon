# Actinon - Raytracer
[<img align = "left" width = "640" height = "360" src = "https://raw.githubusercontent.com/johsteffens/actinon/master/image/hanging_lamp02.acn.640_360.jpg">](https://raw.githubusercontent.com/johsteffens/actinon/master/image/hanging_lamp02.acn.640_360.jpg "Image created with Actinon" )

[<img align = "left" width = "300" height = "400" src = "https://raw.githubusercontent.com/johsteffens/actinon/master/image/paraffin_lamp_on_ledge.acn.png">](https://raw.githubusercontent.com/johsteffens/actinon/master/image/paraffin_lamp_on_ledge.acn.png "Image created with Actinon" )

[<img width = "200" height = "200" src = "https://raw.githubusercontent.com/johsteffens/actinon/master/image/wine_glass.acn.png">](https://raw.githubusercontent.com/johsteffens/actinon/master/image/wine_glass.acn.png "Image created with Actinon" )

[<img width = "200" height = "150" src = "https://raw.githubusercontent.com/johsteffens/actinon/master/image/diamond_video.acn.image_000049.png">](https://raw.githubusercontent.com/johsteffens/actinon/master/image/diamond_video.acn.image_000049.png "Image created with Actinon" )

<br>

### [Find more examples here ...](https://github.com/johsteffens/actinon/wiki/Images)

<br>

## What it is
Actinon is a lightweight ray-tracing renderer and interpreter of a dedicated scripting language.
It can render 3D scenes, employing techniques like distributed tracing, path tracing, anti-aliasing and others.
It can handle transparency, reflection, refraction, media-transition, diffuse-light,
indirect-light and more.
Various realistic surfaces are achieved by mixing fresnel-reflection, chromatic-reflection,
(Oren-Nayar) diffuse-reflection and by simulating surface roughness. The rendering-engine is multi-threaded.

For scene-design I developed a special language. It supports the composition of complex objects from simpler objects.
Geometric operations on objects like translation, scaling and rotations can be applied easily and intuitively.
Vectors and matrices have dedicated types with associated operators supporting arithmetic in a 3D vector space.
The language also allows computing image sequences (e.g. for videos).

Actinon is based on project [beth](https://github.com/johsteffens/beth).

## Getting Started

**Build**
```
git clone https://github.com/johsteffens/beth
git clone https://github.com/johsteffens/actinon
cd actinon
make
```

**Quick rendering example**
```
bin/actinon src_acn/primitives.acn
```
It creates [this image](https://raw.githubusercontent.com/johsteffens/actinon/master/image/primitives.acn.png) in pnm-format as `src_acn/primitives.acn.pnm`.

Many image viewers can read the pnm-format directly. (e.g. `gthumb`).

**Viewing with `gthumb`:**
```
#if not already installed
sudo apt install gthumb

gthumb src_acn/primitives.acn.pnm
```

**Converting image format with `netpbm`:**
```
#if not already installed
sudo apt install netpbm

pnmtopng src_acn/primitives.acn.pnm > src_acn/primitives.acn.png
```

## No AI
Actinon does not use any AI agent for rendering. Image creation is purely script-driven. Thus, the image is the result of your own unique creativity.

## How to use it
Actinon is a console-application. It takes a text-source file as argument and executes its content, 
which normally comprises of the scene design, virtual camera specifications, render specifications 
and instructions how to render one or more images.

It will render the image in multiple passes, gradually improving quality.
After each pass, the image file is updated such that progress can be monitored with an external image-viewing tool.

The location and name of the target image is defined in the script file. The example scripts use the path of the script-file and simply append the extension `.pnm`.

### First Trial
   * Pick a source file from folder [src_acn](https://github.com/johsteffens/actinon/tree/master/src_acn). Maybe [wine_glass.acn](https://github.com/johsteffens/actinon/blob/master/src_acn/wine_glass.acn).
   * Run in a terminal: `actinon wine_glass.acn`
   * After a short time, it will produce the image file `wine_glass.acn.pnm`. 
   The file gets updated at intervals,
   gradually improving image-quality.
   After around 3 ... 30 min (depending on CPU speed and number of cores), rendering should be completed.
   Interrupt any time with Ctl-C, which will save an intermediate result and terminate. 
   You can resume from an incomplete image later.
   * A nice tool to view the image is [gThumb](https://en.wikipedia.org/wiki/GThumb).
   * A useful tool-set to convert the image to another format is [ntpbm](https://en.wikipedia.org/wiki/Netpbm).


### Next Steps
   * Learn a bit about the Actinon Language: For the time being, you might want to glean some insight by examining [wine_glass.acn](https://github.com/johsteffens/actinon/blob/master/src_acn/wine_glass.acn), which is inline-commented for that purpose. 
   * Experiment with [provided scenes](https://github.com/johsteffens/actinon/wiki/Images) or design your own scene.
   * **Tip**: While drafting and testing your scene, switch off path tracing `path_samples = 0` and set `direct_samples` to a low value. E.g.  `direct_samples = 10`. This will yield results in seconds.
   
## How it works
The script file defines objects in a virtual 3d vector space, the size of the resulting image and some more parameters affecting image quality and CPU load. 
A scene is given by a collection of shapes, each with surface information such as reflectivity, luminosity, transparency, roughness. Objects with luminosity act as light source.

### Shape
A shape is a 3D Object with a well-defined surface, size, position and orientation. These specifcy the exact surface-area, which cuts the 3D space into two areas: The outside- and inside-area.
Note that the outside or inside-area can either or both be of infinite expansion. In this context, a simple plane is a 3D shape, too, dividing the 3D space into two distinctive areas.

There is a set of basic predefined shapes, called 'primitives'. These are
* Plane
* Sphere
* Ellipsoid
* Cylinder
* Torus
* Hyperboloid

All other possible shapes are composites from these primitives. 

A new shape can be formed from multiple shapes as intersection of their respective 3D-areas. For example: A cube could be defined by combining 6 planes.
There is no limit on how to combine multiple shapes to form new shapes.

### Surfaces
Each primitive shape has its own surface property covering
* Albedo (Color)
* Fresenel-Reflectivity
* Chromatic-Reflectivity (Metallic Surfaces)
* Diffuse-Reflectivity
* Sigma (from Oren-Nayar-Reflectance-Model)
* Roughness
* Transparancy (Color)
* Radiance (for Light Sources)

## License
* The source code in this repository, including actinon source code, is licensed under the [Apache 2.0 License](https://github.com/johsteffens/actinon/blob/master/LICENSE).
* Example images in this repository, depicting results of the raytracer, are licensed under the [Creative Commons Attribution-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-sa/4.0/) License.
* Images or videos, which you designed yourself by your own creative work, are yours and you can distribute them as you like under your own terms.

## Motivation

### Beth
One objective is demonstrating the capabilities of project '[beth](https://github.com/johsteffens/beth)', which is a foundation-library to develop advanced applications in 'C'.

### Sentiment
I've always been fascinated by computer graphics. Particularly combining physics, mathematics and creativity to achieve realism. In the mid 1990s I playfully experimented with simulating the optics and Lambertian light distribution and thus conceived algorithms using rays casted in a virtual 3D scene. Back then I was not yet aware that the technique was already well-known under the label "ray-tracing". Today, modern workstations are powerful enough to allow deep recursion into the rendering equation (path-tracing), making experimenting in this field even more fun and rewarding.

### About The Name
Actinon is the name of the Radon-219 isotope, which is a radioactive gas. As project-name, it was inspired by a train of thought: ray-tracing -> radiation -> radon -> actinon.
