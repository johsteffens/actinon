# Actinon - Raytracer

![Wine Glass](https://raw.githubusercontent.com/johsteffens/actinon/master/image/wine_glass.acn.png "Image created with Actinon")
![Diamond](https://raw.githubusercontent.com/johsteffens/actinon/master/image/diamond.acn.png "Image created with Actinon")

### [Find more images here ...](https://github.com/johsteffens/actinon/wiki/Images)

## What it is
Actinon is a ray-tracer. It can render 3D scenes realistically, employing techniques like distributed tracing, path tracing, antialiasing and others. It can visualize transparency, reflection, refraction, media-transition, diffuse-light, indirect-light and more. Realistic polished surfaces are achieved by mixing Fresnel-reflection, diffuse-reflection and light-absorption while observing energy conservation.

For scene-design a special scripting language was developed. It is aimed to simplify object definition and composition. It supports the construction of complex objects from simpler objects. Geometric operations like translation, scaling and rotations can be realized simply and intuitively. 3D vectors and higher order tensors are dedicated objects with predefined operators supporting basic vector arithmetic. The language also allows computing image sequences (e.g. for videos).

Actinon is based on project [beth](https://github.com/johsteffens/beth).

## License
The source code in this project, including actinon-script, is licensed under the Apache 2.0 License.

Images and videos uploaded into this project and depicting results of the raytracer are licensed under the [Creative Commons Attribution-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-sa/4.0/) License.

Images or videos you created with actinon from your own scene sources are yours.

## Motivation
I've always been fascinated by computer graphics, particluarly employing physics in order to achieve realism. About 25 years ago, I experimented with algorithms using lambertian light distribution on rays casted in a virtual 3D scene in order to generate realistic visual effects. Back then I was even oblivious to the fact that the technique was well-known under the label "ray-tracing".

Today, modern workstations are powerful enough to allow deep recursions into the render equation making experimenting in this field more rewarding.

Another objective is demonstrating the capabilities of the (much bigger) project '[beth](https://github.com/johsteffens/beth)', which is a foundation-library to develop advanced applications in 'C' as well as designing meta-languages of which the actinon-script might be an example.
