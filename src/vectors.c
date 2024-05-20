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

#include "bcore_sinks.h"
#include "bcore_spect_inst.h"
#include "bcore_life.h"
#include "bcore_spect.h"
#include "bcore_spect_array.h"
#include "bcore_txt_ml.h"
#include "vectors.h"

/**********************************************************************************************************************/

BCORE_DEFINE_FUNCTIONS_SELF_OBJECT_FLAT( v2d_s,      "v2d_s = bcore_inst { f3_t x; f3_t y; }" )
BCORE_DEFINE_FUNCTIONS_SELF_OBJECT_FLAT( v3d_s,      "v3d_s = bcore_inst { f3_t x; f3_t y; f3_t z; }" )
BCORE_DEFINE_FUNCTIONS_SELF_OBJECT_FLAT( m3d_s,      "m3d_s = bcore_inst { v3d_s x; v3d_s y; v3d_s z; }" )
BCORE_DEFINE_FUNCTIONS_SELF_OBJECT_FLAT( ray_s,      "ray_s = bcore_inst { v3d_s p; v3d_s d; }" )
BCORE_DEFINE_FUNCTIONS_SELF_OBJECT_FLAT( ray_cone_s, "ray_cone_s = bcore_inst { ray_s ray; f3_t cos_rs; }" )

/**********************************************************************************************************************/
/// cl_s

typedef er_t (*bcore_fp_copy_typed   )( vd_t o, tp_t type, vc_t src ); // deep conversion & copy
er_t cl_s_copy_typed( cl_s* o, tp_t type, vc_t src )
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
        return bcore_error_push_fa( TYPEOF_conversion_error, "Cannot convert '%s' to 'cl_s'.", ifnameof( type ) );
    }
    return 0;
}

BCORE_DEFINE_FUNCTIONS_OBJ_FLAT( cl_s )
static bcore_self_s* cl_s_create_self( void )
{
    bcore_self_s* self = BCORE_SELF_S_BUILD_PARSE_SC( "cl_s = v3d_s", cl_s );
    bcore_self_s_push_ns_func( self, ( fp_t )cl_s_copy_typed, "bcore_fp_copy_typed", "copy_typed" );
    return self;
}

/**********************************************************************************************************************/
/// row_cl_s

BCORE_DEFINE_FUNCTIONS_SELF_OBJECT_INST( row_cl_s, "row_cl_s = bcore_inst { aware_t _; cl_s [] arr; }" )


void row_cl_s_set_size( row_cl_s* o, uz_t size, cl_s color )
{
    bcore_array_a_set_size( (bcore_array*)o, size );
    for( uz_t i = 0; i < size; i++ ) o->data[ i ] = color;
}

/**********************************************************************************************************************/
/// image_cl_s

BCORE_DEFINE_FUNCTIONS_SELF_OBJECT_INST( image_cl_s, "image_cl_s = bcore_inst { aware_t _; uz_t w; uz_t h; cl_s [] arr; }" )

void image_cl_s_set_size( image_cl_s* o, uz_t w, uz_t h, cl_s color )
{
    bcore_array_a_set_size( (bcore_array*)o, w * h );
    for( uz_t i = 0; i < o->size; i++ ) o->data[ i ] = color;
    o->w = w;
    o->h = h;
}

void image_cl_s_saturate( image_cl_s* o, f3_t gamma )
{
    for( uz_t i = 0; i < o->size; i++ ) o->data[ i ] = cl_s_sat( o->data[ i ], gamma );
}

/**********************************************************************************************************************/

vd_t vectors_signal_handler( const bcore_signal_s* o )
{
    switch( bcore_signal_s_handle_type( o, typeof( "vectors" ) ) )
    {
        case TYPEOF_init1:
        {
            BCORE_REGISTER_OBJECT( v2d_s );
            BCORE_REGISTER_OBJECT( v3d_s );
            BCORE_REGISTER_OBJECT( m3d_s );
            BCORE_REGISTER_OBJECT( ray_s );
            BCORE_REGISTER_OBJECT( ray_cone_s );
            BCORE_REGISTER_OBJECT( cl_s );
            BCORE_REGISTER_OBJECT( row_cl_s );
            BCORE_REGISTER_OBJECT( image_cl_s );
        }
        break;

        default: break;
    }
    return NULL;
}

/**********************************************************************************************************************/


