/** Geometry Mathematics (Ray-Intersections, Projections, etc)
 *  Author: Johannes Bernhard Steffens
 *
 *  Copyright (c) 2017 Johannes Bernhard Steffens. All rights reserved.
 */

#include "gmath.h"

void compute_refraction( v3d_s dir_i, v3d_s nor, f3_t rix, f3_t* intensity_r, v3d_s* dir_r, f3_t* intensity_t, v3d_s* dir_t )
{
    f3_t c = v3d_s_mlv( dir_i, nor );
    f3_t f = c > 0 ? rix : 1.0 / rix;

    f3_t cos_ai = fabs( c );
    cos_ai = cos_ai > 1.0 ? 1.0 : cos_ai; // to prevent rounding errors
    f3_t sin_ai = sqrt( 1.0 - cos_ai * cos_ai );
    f3_t sin_at = sin_ai * f;

    f3_t reflectance   = 1.0;
    f3_t transmittance = 0.0;

    if( sin_at < 1 ) // else: total reflection
    {
        f3_t cos_at = sqrt( 1.0 - sin_at * sin_at );
        f3_t rs = f3_sqr( ( f * cos_ai - cos_at ) / ( f * cos_ai + cos_at ) ); // perpendicular polarization
        f3_t rp = f3_sqr( ( f * cos_at - cos_ai ) / ( f * cos_at + cos_ai ) ); // parallel polarization
        reflectance   = ( rs + rp ) * 0.5;
        transmittance = 1.0 - reflectance;
    }

    if( dir_r )
    {
        *intensity_r = reflectance;
        *dir_r = v3d_s_of_length( v3d_s_sub( dir_i, v3d_s_mlf( nor, 2.0 * v3d_s_mlv( dir_i, nor ) ) ), 1.0 );
    }

    if( dir_t )
    {
        *intensity_t = transmittance;
        f3_t a = f;
        f3_t q = f * f * ( 1.0 - c * c );
        if( q < 1.0 && transmittance > 0 )
        {
            f3_t b = - f * c + ( c > 0 ? sqrt( 1.0 - q ) : -sqrt( 1.0 - q ) );
            *dir_t = v3d_s_add( v3d_s_mlf( dir_i, a ), v3d_s_mlf( nor, b ) );
        }
        else
        {
            *dir_t = dir_i; // straight incidence or no transmittance
        }
    }
}

/**********************************************************************************************************************/

vd_t gmath_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "gmath" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
//        bcore_flect_define_creator( typeof( "image_cl_s"  ), image_cl_s_create_self );
    }

    return NULL;
}

/**********************************************************************************************************************/



