/** Geometry Mathematics (Ray-Intersections, Projections, etc)
 *  Author: Johannes Bernhard Steffens
 *
 *  Copyright (c) 2017 Johannes Bernhard Steffens. All rights reserved.
 */

#include "gmath.h"

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



