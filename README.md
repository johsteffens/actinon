# Rayflux - Raytracer

![Rayflux Image](https://raw.githubusercontent.com/johsteffens/rayflux/master/image/wine_glass.ray.png "Image created with Rayflux")

## What it is
Rayflux is a ray-tracer based on project [beth](https://github.com/johsteffens/beth).

It can render realistic 3D scenes employing techniques like distributed tracing, path tracing and photon mapping.
It handles transparency, reflection, refraction and and diffuse-light processing.
With a special 'thin-coating-model' for opaque objects yields a realistic polished-surface-effect.

Scene construction can be done via a scripting language which was designed for the purpose.

## License
The source code in this project, including rayflux-script, is licensed under the Apache 2.0 License. 

Images and videos uploaded into this project and depicting results of the raytracer shall be licensed under the [Creative Commons Attribution-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-sa/4.0/) License.

Images or videos you created with rayflux from your own scene sources are yours.

## Motivation
I've always been fascinated by computer graphics, particluarly employing physics in order to achieve realism. About 25 years ago, I experimented with algorithms using lambertian light distribution on rays casted in a virtual 3D scene in order to generate realistic visual effects. Back then I was even oblivious to the fact that the technique was well-known under the label "ray-tracing".

Today computers are powerful enough to allow deep recursions into the render equation making experimenting in this field more rewarding.

Another objective is demonstrating the capabilities of the (much bigger) project '[beth](https://github.com/johsteffens/beth)', which is a foundation-library to develop advanced applications in 'C' as well as designing meta-languages of which the rayflux-language might be an example.
