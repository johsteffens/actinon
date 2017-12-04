/// Author & Copyright (C) 2017 Johannes Bernhard Steffens. All rights reserved.

#include "scene.h"
#include "bcore_sinks.h"
#include "bcore_spect_inst.h"
#include "bcore_life.h"
#include "bcore_spect.h"
#include "bcore_spect_array.h"
#include "bcore_txt_ml.h"
#include "vectors.h"

/**********************************************************************************************************************/

/// v2d_s
DEFINE_FUNCTIONS_OBJ_FLAT( v2d_s )
DEFINE_CREATE_SELF( v2d_s, "v2d_s = bcore_inst { f3_t x; f3_t y }" )

/**********************************************************************************************************************/

/// v3d_s
DEFINE_FUNCTIONS_OBJ_FLAT( v3d_s )
DEFINE_CREATE_SELF( v3d_s, "v3d_s = bcore_inst { f3_t x; f3_t y; f3_t z; }" )

/**********************************************************************************************************************/

/// m3d_s  (3x3 matrix)
DEFINE_FUNCTIONS_OBJ_FLAT( m3d_s )
DEFINE_CREATE_SELF( m3d_s, "m3d_s = bcore_inst { v3d_s x; v3d_s y; v3d_s z; }" )

/**********************************************************************************************************************/
/// ray_s

DEFINE_FUNCTIONS_OBJ_FLAT( ray_s )
DEFINE_CREATE_SELF( ray_s, "ray_s = bcore_inst { v3d_s p; v3d_s d; }" )

/**********************************************************************************************************************/
/// ray_cone_s

DEFINE_FUNCTIONS_OBJ_FLAT( ray_cone_s )
DEFINE_CREATE_SELF( ray_cone_s, "ray_cone_s = bcore_inst { ray_s ray; f3_t cos_rs; }" )

/**********************************************************************************************************************/
/// cl_s

DEFINE_FUNCTIONS_OBJ_FLAT( cl_s )
DEFINE_CREATE_SELF( cl_s, "cl_s = v3d_s" )

/**********************************************************************************************************************/
/// row_cl_s

DEFINE_FUNCTIONS_OBJ_INST( row_cl_s )
DEFINE_CREATE_SELF( row_cl_s, "row_cl_s = bcore_inst { aware_t _; cl_s [] arr; }" )

void row_cl_s_set_size( row_cl_s* o, sz_t size, cl_s color )
{
    bcore_array_aware_set_size( o, size );
    for( sz_t i = 0; i < size; i++ ) o->data[ i ] = color;
}

/**********************************************************************************************************************/
/// image_cl_s

DEFINE_FUNCTIONS_OBJ_INST( image_cl_s )
DEFINE_CREATE_SELF( image_cl_s, "image_cl_s = bcore_inst { aware_t _; sz_t w; sz_t h; cl_s [] arr; }" )

void image_cl_s_set_size( image_cl_s* o, sz_t w, sz_t h, cl_s color )
{
    bcore_array_aware_set_size( o, w * h );
    for( sz_t i = 0; i < o->size; i++ ) o->data[ i ] = color;
    o->w = w;
    o->h = h;
}

/**********************************************************************************************************************/

vd_t vectors_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "vectors" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        bcore_flect_define_creator( typeof( "v2d_s"       ), v2d_s_create_self );
        bcore_flect_define_creator( typeof( "v3d_s"       ), v3d_s_create_self );
        bcore_flect_define_creator( typeof( "m3d_s"       ), m3d_s_create_self );
        bcore_flect_define_creator( typeof( "ray_s"       ), ray_s_create_self );
        bcore_flect_define_creator( typeof( "ray_cone_s"  ), ray_cone_s_create_self );
        bcore_flect_define_creator( typeof( "cl_s"        ), cl_s_create_self  );
        bcore_flect_define_creator( typeof( "row_cl_s"    ), row_cl_s_create_self   );
        bcore_flect_define_creator( typeof( "image_cl_s"  ), image_cl_s_create_self );
    }

    return NULL;
}

/**********************************************************************************************************************/


