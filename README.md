# Rayflux - Raytracer

![Rayflux Image](https://raw.githubusercontent.com/johsteffens/rayflux/master/image/wine_glass.ray.png "Image created with Rayflux")

## What it is
Rayflux is a ray-tracer. It is based on project [beth](https://github.com/johsteffens/beth).

It can render realistic 3D scenes employing techniques like distributed tracing, path tracing and photon mapping.
It handles transparency, reflection, refraction and and diffuse-light progression.
It employs a special 'thin-coating-model' for opaque objects to produce realistic polished-surface-effect.
It defines its own scripting language for scene construction.

## License & Attribution
The source code in this project, including rayflux-script, is licensed under the Apache 2.0 License. 

Images and videos uploaded into this project and depicting results of the raytracer shall be licensed under the [Creative Commons Attribution-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-sa/4.0/) License.

Images or videos rendered by rayflux from your own scene sources are yours.

## Motivation
I've always been fascinated by computer graphics, particluarly employing physics in order to achieve realism. About 25 years ago, I experimented with algorithms using lambertian light distribution on rays casted in a virtual 3D scene in order to generate realistic visual effects. Back then I was even oblivious to the fact that the technique was well-known under the label "ray-tracing".

Today computers are powerful enough to allow deep recursions into the render equation making experimenting in this field more rewarding.

Another objective is demonstrating the capabilities of the (much bigger) project '[beth](https://github.com/johsteffens/beth)', which is a foundation-library to develop advanced applications in 'C' as well as designing meta-languages of which the rayflux-language might be an example.
