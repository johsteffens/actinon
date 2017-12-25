/** Vectors and Related Objects */

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
DEFINE_CREATE_SELF( v2d_s, "v2d_s = bcore_inst { f3_t x; f3_t y; }" )

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

typedef void (*bcore_fp_copy_typed   )( vd_t o, tp_t type, vc_t src ); // deep conversion & copy
void cl_s_copy_typed( cl_s* o, tp_t type, vc_t src )
{
    if( type == TYPEOF_cl_s )
    {
        *o = *( cl_s* )src;
    }
    else if( type == TYPEOF_v3d_s )
    {
        *o = *( v3d_s* )src;
    }
    else
    {
        ERR( "Cannot convert '%s' to 'cl_s'.", ifnameof( type ) );
    }
}

DEFINE_FUNCTIONS_OBJ_FLAT( cl_s )
static bcore_flect_self_s* cl_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( "cl_s = v3d_s", sizeof( cl_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )cl_s_copy_typed, "bcore_fp_copy_typed", "copy_typed" );
    return self;
}

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

void image_cl_s_saturate( image_cl_s* o, f3_t gamma )
{
    for( sz_t i = 0; i < o->size; i++ ) o->data[ i ] = cl_s_sat( o->data[ i ], gamma );
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


