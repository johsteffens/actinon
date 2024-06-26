# Actinon - Raytracer
[<img align = "left" width = "640" height = "360" src = "https://raw.githubusercontent.com/johsteffens/actinon/master/image/hanging_lamp02.acn.640_360.jpg">](https://raw.githubusercontent.com/johsteffens/actinon/master/image/hanging_lamp02.acn.640_360.jpg "Image created with Actinon" )

[<img align = "left" width = "300" height = "400" src = "https://raw.githubusercontent.com/johsteffens/actinon/master/image/paraffin_lamp_on_ledge.acn.png">](https://raw.githubusercontent.com/johsteffens/actinon/master/image/paraffin_lamp_on_ledge.acn.png "Image created with Actinon" )

[<img width = "200" height = "200" src = "https://raw.githubusercontent.com/johsteffens/actinon/master/image/wine_glass.acn.png">](https://raw.githubusercontent.com/johsteffens/actinon/master/image/wine_glass.acn.png "Image created with Actinon" )

[<img width = "200" height = "150" src = "https://raw.githubusercontent.com/johsteffens/actinon/master/image/diamond_video.acn.image_000049.png">](https://raw.githubusercontent.com/johsteffens/actinon/master/image/diamond_video.acn.image_000049.png "Image created with Actinon" )

<br>

### [Find more examples here ...](https://github.com/johsteffens/actinon/wiki/Images)

<br>

## Getting Started

```
$ git clone https://github.com/johsteffens/beth
$ git clone https://github.com/johsteffens/actinon
$ cd actinon
$ make

# Run a quick rendering:
$ bin/actinon src_acn/primitives.acn

# Display result using gThumb:
$ gthumb src_acn/primitives.acn.pnm
```

## What it is
Actinon is a lightweight ray-tracing renderer and interpreter of a dedicated scripting language.
It can render 3D scenes, employing techniques like distributed tracing, path tracing, anti-aliasing and others.
It can handle transparency, reflection, refraction, media-transition, diffuse-light,
indirect-light and more.
Various realistic surfaces are achieved by mixing fresnel-reflection, chromatic-reflection,
(Oren-Nayar) diffuse-reflection and by simulating surface roughness. The rendering-engine is multi-threaded.

For scene-design a special language has been developed.
It supports the composition of complex objects from simpler objects.
Geometric operations on objects like translation, scaling and rotations can be applied easily and intuitively.
Vectors and matrices have dedicated types with associated operators supporting arithmetic in a 3D vector space.
The language also allows computing image sequences (e.g. for videos).

Actinon is based on project [beth](https://github.com/johsteffens/beth).

## How it works

Actinon is a console-application. It takes a text-source file as argument and executes its content, 
which normally comprises of the scene design, virtual camera specifications, render specifications 
and instructions how to render one or more images.

It will render the image in multiple passes, gradually improving quality.
After each pass, the image file is updated such that progress can be monitored with an external image-viewing tool.

### First Trial
   * Pick a source file from folder [src_acn](https://github.com/johsteffens/actinon/tree/master/src_acn). Maybe [wine_glass.acn](https://github.com/johsteffens/actinon/blob/master/src_acn/wine_glass.acn).
   * Run in a terminal: `actinon wine_glass.acn`
   * After a short time, it will produce the image file `wine_glass.acn.pnm`. 
   The file gets updated at intervals,
   gradually improving image-quality.
   After around 3 ... 30 min (depending on CPU speed and number of cores), rendering should be completed.
   Interrupt any time with Ctl-C, which will save an intermediate result and terminate. 
   You can resume from an incomplete image later.
   * You may want to convert the image to a more common format with netpbm or similar tool (e.g. `pnmtopng`).
   * A nice tool to view the image is [gThumb](https://en.wikipedia.org/wiki/GThumb).

### Next Steps
   * Learn a bit about the Actinon Language: For the time being, you might want to glean some insight by examining [wine_glass.acn](https://github.com/johsteffens/actinon/blob/master/src_acn/wine_glass.acn), which is inline-commented for that purpose. 
   * Experiment with [provided scenes](https://github.com/johsteffens/actinon/wiki/Images) or design your own scene.
   * **Tip**: While drafting and testing your scene, switch off path tracing `path_samples = 0` and set `direct_samples` to a low value. E.g.  `direct_samples = 10`. This will yield results in seconds.
   
## License
The source code in this repository, including actinon source code, is licensed under
the [Apache 2.0 License](https://github.com/johsteffens/actinon/blob/master/LICENSE).
Images in this repository, depicting results of the raytracer, are licensed under
the [Creative Commons Attribution-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-sa/4.0/) License.
Images or videos you created with actinon from your own script sources are yours.

## Motivation

### Beth
One objective is demonstrating the capabilities of project '[beth](https://github.com/johsteffens/beth)', which is a foundation-library to develop advanced applications in 'C'.

### Sentiment
I've always been fascinated by computer graphics. Particularly combining physics, mathematics and creativity to achieve realism. In the mid 1990s, as physics student, 
I playfully experimented with simulating the optics and Lambertian light distribution and thus conceived algorithms using rays casted in a virtual 3D scene. Back then I was not even aware
that the technique was already well-known under the label "ray-tracing". Today, modern workstations are powerful enough to allow 
deep recursions into the rendering equation, making experimenting in this field even more fun and rewarding.

### About The Name
Actinon is the name of the Radon-219 isotope, which is a radioactive gas. As project-name, it was inspired by a train of thought: ray-tracing -> radiation -> radon -> actinon.
