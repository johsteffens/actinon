/** Geometry Mathematics (Ray-Intersections, Projections, etc) */

/** Copyright 2017 Johannes Bernhard Steffens
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GMATH_H
#define GMATH_H

#include "quicktypes.h"
#include "vectors.h"

/**********************************************************************************************************************/

void compute_refraction( v3d_s dir_i, v3d_s nor, f3_t rix, f3_t* intensity_r, v3d_s* dir_r, f3_t* intensity_t, v3d_s* dir_t );

/**********************************************************************************************************************/

static inline f3_t plane_ray_offset( v3d_s pos, v3d_s nor, const ray_s* ray )
{
    f3_t div = v3d_s_mlv( nor, ray->d );
    if( div == 0 ) return f3_inf; // plane and ray are parallel
    f3_t offs = v3d_s_sub_mlv( pos, ray->p, nor ) / div;
    return ( offs > 0 ) ? offs - f3_eps : f3_inf;
}

static inline bl_t plane_observer_outside( v3d_s pos, v3d_s nor, v3d_s observer )
{
    return v3d_s_sub_mlv( observer, pos, nor ) > 0;
}

static inline v3d_s plane_observer_normal( v3d_s pos, v3d_s nor, v3d_s observer )
{
    return plane_observer_outside( pos, nor, observer ) ? nor : v3d_s_neg( nor );
}

/**********************************************************************************************************************/

static inline f3_t sphere_ray_offset( v3d_s pos, f3_t r, const ray_s* ray )
{
    v3d_s p = v3d_s_sub( ray->p, pos );
    f3_t s = v3d_s_mlv( p, ray->d );
    f3_t q = v3d_s_sqr( p ) - ( r * r );

    f3_t s2 = s * s;
    if( s2 < q ) return f3_inf; // missing the object
    if( s < 0 && q > 0 ) // entry hit is positive
    {
        return -s - sqrt( s2 - q ) - f3_eps;
    }
    else if( s < 0 || q < 0 ) // exit hit is positive
    {
        return -s + sqrt( s2 - q ) - f3_eps;
    }
    else
    {
        return f3_inf; // all hits negative
    }
}

static inline bl_t sphere_observer_outside( v3d_s pos, f3_t r, v3d_s observer )
{
    v3d_s diff = v3d_s_sub( observer, pos );
    return ( v3d_s_sqr( diff ) > r * r ) ? true : false;
}

static inline v3d_s sphere_observer_normal( v3d_s pos, f3_t r, v3d_s observer )
{
    v3d_s diff = v3d_s_sub( observer, pos );
//    return v3d_s_of_length( diff, ( v3d_s_sqr( diff ) > r * r ) ? 1.0 : -1.0 );
    return v3d_s_of_length( diff, 1.0 );
}

/**********************************************************************************************************************/

static inline f3_t cylinder_ray_offset( v3d_s pos, v3d_s dir, f3_t r, const ray_s* ray )
{
    v3d_s px = v3d_s_sub( ray->p, pos );
    f3_t a = v3d_s_mlv( px,   dir );
    f3_t b = v3d_s_mlv( ray->d, dir );
    f3_t c = v3d_s_mlv( ray->d, px );
    f3_t e = v3d_s_sqr( px );

    f3_t div = 1.0 - b * b;
    if( div == 0 )  return f3_inf; // cylinder and ray are parallel

    f3_t inv = 1.0 / div;
    f3_t s = ( c - a * b ) * inv;
    f3_t q = ( e - a * a - r * r ) * inv;

    f3_t s2 = s * s;
    if( s2 < q ) return f3_inf; // missing the object
    if( s < 0 && q > 0 ) // entry hit is positive
    {
        return -s - sqrt( s2 - q ) - f3_eps;
    }
    else if( s < 0 || q < 0 ) // exit hit is positive
    {
        return -s + sqrt( s2 - q ) - f3_eps;
    }
    else
    {
        return f3_inf; // all hits negative
    }
}

static inline bl_t cylinder_observer_outside( v3d_s pos, v3d_s dir, f3_t r, v3d_s observer )
{
    v3d_s p = v3d_s_sub( observer, pos );
    v3d_s pr = v3d_s_orthogonal_projection( p, dir );
    return v3d_s_sqr( pr ) > r * r;
}

static inline v3d_s cylinder_observer_normal( v3d_s pos, v3d_s dir, f3_t r, v3d_s observer )
{
    v3d_s p = v3d_s_sub( observer, pos );
    v3d_s pr = v3d_s_orthogonal_projection( p, dir );
    return v3d_s_of_length( pr, 1.0 );
}

/**********************************************************************************************************************/

static inline f3_t cone_ray_offset( v3d_s pos, v3d_s dir, f3_t cosa, const ray_s* ray )
{
    v3d_s px = v3d_s_sub( ray->p, pos );
    f3_t a = v3d_s_mlv( px, dir );
    f3_t b = v3d_s_mlv( ray->d, dir );
    f3_t c = v3d_s_mlv( ray->d, px );
    f3_t e = v3d_s_sqr( px );
    f3_t f = cosa * cosa;

    f3_t div = f - b * b;
    if( div == 0 )  return f3_inf; // cone and ray are parallel

    f3_t inv = 1.0 / div;
    f3_t s = ( c * f - a * b ) * inv;
    f3_t q = ( e * f - a * a ) * inv;

    f3_t s2 = s * s;
    if( s2 < q ) return f3_inf; // missing the object

    if( s < 0 && q > 0 ) // entry hit is positive
    {
        f3_t offs = -s - sqrt( s2 - q );
        if( a + offs * b > 0 ) return f3_inf; // opposite cone direction
        return offs - f3_eps;
    }
    else if( s < 0 || q < 0 ) // exit hit is positive
    {
        f3_t offs = -s + sqrt( s2 - q );
        if( a + offs * b > 0 ) return f3_inf; // opposite cone direction
        return offs - f3_eps;
    }
    else
    {
        return f3_inf; // all hits negative
    }
}

static inline bl_t cone_observer_outside( v3d_s pos, v3d_s dir, f3_t cosa, v3d_s observer )
{
    v3d_s p = v3d_s_sub( observer, pos );
    f3_t p2 = v3d_s_sqr( p );
    if( p2 == 0 ) return true;
    f3_t coso = -v3d_s_mlv( p, dir ) / sqrt( p2 );
    return coso < cosa;
}

static inline v3d_s cone_observer_normal( v3d_s pos, v3d_s dir, f3_t cosa, v3d_s observer )
{
    v3d_s p = v3d_s_sub( observer, pos );
    f3_t p2 = v3d_s_sqr( p );
    if( p2 == 0 ) return dir;
    f3_t pd = v3d_s_mlv( p, dir );
    v3d_s n = v3d_s_sub( dir, v3d_s_mlf( p, pd / p2 ) );
    return v3d_s_of_length( n, 1.0 );
}

/**********************************************************************************************************************/

vd_t gmath_signal( tp_t target, tp_t signal, vd_t object );

#endif // GMATH_H


