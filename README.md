# Actinon - Raytracer

![Wine Glass](https://raw.githubusercontent.com/johsteffens/actinon/master/image/wine_glass.acn.png "Image created with Actinon")
![Diamond](https://raw.githubusercontent.com/johsteffens/actinon/master/image/diamond.acn.png "Image created with Actinon")

### [Find more images here ...](https://github.com/johsteffens/actinon/wiki/Images)

## What it is
Actinon is a ray-tracing renderer and interpreter of a dedicated scripting language. It can render 3D scenes, employing techniques like distributed tracing, path tracing, antialiasing and others. It can visualize transparency, reflection, refraction, media-transition, diffuse-light, indirect-light and more. Realistic surfaces are achieved by mixing fresnel-reflection, chromatic-reflection, diffuse-reflection observing energy conservation.

For scene-design a special scripting language has been developed. It is aimed to simplify object definition and composition. It supports the construction of complex objects from simpler objects. Geometric operations like translation, scaling and rotations can be realized simply and intuitively. 3D vectors and higher order tensors are dedicated objects with predefined operators supporting basic vector arithmetic. The language also allows computing image sequences (e.g. for videos).

Actinon is based on project [beth](https://github.com/johsteffens/beth).

## How it works

Actinon is a console-application. It takes a script file as argument and executes its content, which normally comprises of the scene design, virtual camera specifications, render specifications and instructions how to render one or more images.

It will render the image in multiple passes, during which intermediate results can be inspected with an image-viewing tool.

(More details will follow ...)

## How to use it

Instructions below apply to a Posix-like environment (e.g. Linux).

### Build
   * Download [beth](https://github.com/johsteffens/beth) and [actinon](https://github.com/johsteffens/actinon).
   * For simplicity I'd suggest to place all *.c and *.h files of both projects into the same folder. (Makefiles arr in  planning.)
   * Build the actinon binary with `-std=c11 -lm -lpthread`. E.g.: `gcc -std=c11 -O3 *.c -lm -lpthread -o actinon`

### First Trial
   * Pick a script file from folder [scr_acn](https://github.com/johsteffens/actinon/tree/master/src_acn). Maybe [wine_glass.acn](https://github.com/johsteffens/actinon/blob/master/src_acn/wine_glass.acn).
   * Run: `actinon wine_glass.acn`
   * After 1 ... 2 minutes, it will produce the image file `wine_glass.acn.pnm`. The file gets updated at intervals gradually improving image-quality. After around 20 ... 60 min (depending on CPU speed), rendering should be completed.
   * You may want to convert the image to a more common format with netpbm or similar tool (e.g. `pnmtopng`).

### Next Steps
   * Learn a bit about the Actinon language syntax: A documentation on the syntax is planned. For the time being you might want to glean some insight about its syntax by examining [wine_glass.acn](https://github.com/johsteffens/actinon/blob/master/src_acn/wine_glass.acn), which is commented for that purpose. 
   * Try to design your own scene.
   * Tip: While drafting and testing your scene, switch off path tracing `path_samples = 0` and reduce direct_samples to a low value. E.g.  `direct_samples = 10`. This will yield results in seconds.

## License
The source code in this project, including actinon-script, is licensed under the Apache 2.0 License.

Images and videos uploaded into this project and depicting results of the raytracer are licensed under the [Creative Commons Attribution-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-sa/4.0/) License.

Images or videos you created with actinon from your own script sources are yours.

## Motivation
I've always been fascinated by computer graphics, particluarly employing physics in order to achieve realism. About 25 years ago, I experimented with algorithms using lambertian light distribution on rays casted in a virtual 3D scene in order to generate realistic visual effects. Back then I was even oblivious to the fact that the technique was well-known under the label "ray-tracing".

Today, modern workstations are powerful enough to allow deep recursions into the render equation making experimenting in this field more rewarding.

Another objective is demonstrating the capabilities of the (much bigger) project '[beth](https://github.com/johsteffens/beth)', which is a foundation-library to develop advanced applications in 'C' as well as designing meta-languages of which the actinon-script might be an example.
