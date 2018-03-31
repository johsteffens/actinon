/** Texture Fields and Texture Mapping */

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

#include <math.h>
#include "bcore_threads.h"
#include "bcore_spect_inst.h"
#include "bcore_life.h"
#include "bcore_spect.h"
#include "bcore_trait.h"
#include "textures.h"
#include "quicktypes.h"

/**********************************************************************************************************************/
/// spect_txm_s  (texture-map)

typedef cl_s (*clr_fp )( vc_t o, vc_t obj, v3d_s pos ); // converts position into color
v2d_s obj_projection( vc_t o, v3d_s pos );

typedef struct spect_txm_s
{
    aware_t p_type;
    tp_t    o_type;
    clr_fp  fp_clr;
} spect_txm_s;
BCORE_DEFINE_FUNCTIONS_OBJ_INST( spect_txm_s )

/// common txm header
typedef struct txm_hdr_s
{
    aware_t _;
    const spect_txm_s* p;
} txm_hdr_s;

static const spect_txm_s* txm_get_spect( vc_t o ) { return ( ( txm_hdr_s* )o )->p; }

cl_s txm_clr( vc_t o, vc_t obj, v3d_s pos )
{
    return txm_get_spect( o )->fp_clr( o, obj, pos );
}

static spect_txm_s* spect_txm_s_create_from_self( const bcore_self_s* self )
{
    assert( self != NULL );
    spect_txm_s* o = spect_txm_s_create();
    o->o_type = self->type;
    o->fp_clr = ( clr_fp )bcore_self_s_get_external_fp( self, bcore_name_enroll( "clr_fp" ), 0 );
    return o;
}

static bcore_self_s* spect_txm_s_create_self( void )
{
    sc_t def = "spect_txm_s = spect { aware_t p_type; tp_t o_type; ... }";
    bcore_self_s* self = bcore_self_s_build_parse_sc( def, sizeof( spect_txm_s ) );
    bcore_self_s_push_ns_func( self, ( fp_t )spect_txm_s_create_from_self, "bcore_spect_fp_create_from_self", "create_from_self" );
    return self;
}

/**********************************************************************************************************************/
/// txm_plain_s  (plain color texture map)

typedef struct txm_plain_s
{
    aware_t _;
    const spect_txm_s* p;
    cl_s color;
} txm_plain_s;

static sc_t txm_plain_s_def =
"txm_plain_s = spect_txm"
"{"
    "aware_t _;"
    "spect spect_txm_s* p;"
    "cl_s color;"
"}";

BCORE_DEFINE_FUNCTIONS_OBJ_INST( txm_plain_s )

static void txm_plain_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; txm_plain_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
    nc_l->o->color = ( cl_s ) { 0.7, 0.7, 0.7 };
}

static cl_s txm_plain_s_clr( const txm_plain_s* o, vc_t obj, v3d_s pos )
{
    return o->color;
}

cl_s txm_plain_clr( vc_t o )
{
    assert( *(aware_t*)o == TYPEOF_txm_plain_s );
    return ( ( txm_plain_s* )o )->color;
}

static bcore_self_s* txm_plain_s_create_self( void )
{
    bcore_self_s* self = bcore_self_s_build_parse_sc( txm_plain_s_def, sizeof( txm_plain_s ) );
    bcore_self_s_push_ns_func( self, ( fp_t )txm_plain_s_init_a, "ap_t", "init" );
    bcore_self_s_push_ns_func( self, ( fp_t )txm_plain_s_clr, "clr_fp", "clr" );
    return self;
}

/**********************************************************************************************************************/
/// txm_chess_s  (chess color texture map)

typedef struct txm_chess_s
{
    aware_t _;
    const spect_txm_s* p;
    cl_s color1;
    cl_s color2;
    f3_t scale;
} txm_chess_s;

static sc_t txm_chess_s_def =
"txm_chess_s = spect_txm"
"{"
    "aware_t _;"
    "spect spect_txm_s* p;"
    "cl_s color1;"
    "cl_s color2;"
    "f3_t scale = 1.0;"
"}";

BCORE_DEFINE_FUNCTIONS_OBJ_INST( txm_chess_s )

static cl_s txm_chess_s_clr( const txm_chess_s* o, vc_t obj, v3d_s pos )
{
    v2d_s p = obj_projection( obj, pos );
    s3_t x = llrint( p.x * o->scale );
    s3_t y = llrint( p.y * o->scale );
    return ( ( x ^ y ) & 1 ) ? o->color1 : o->color2;
}

static bcore_self_s* txm_chess_s_create_self( void )
{
    bcore_self_s* self = bcore_self_s_build_parse_sc( txm_chess_s_def, sizeof( txm_chess_s ) );
    bcore_self_s_push_ns_func( self, ( fp_t )txm_chess_s_clr, "clr_fp", "clr" );
    return self;
}

/**********************************************************************************************************************/

vd_t textures_signal_handler( const bcore_signal_s* o )
{
    switch( bcore_signal_s_handle_type( o, typeof( "textures" ) ) )
    {
        case TYPEOF_init1:
        {
            bcore_trait_set( entypeof( "spect_txm" ), entypeof( "bcore_inst" ) );

            BCORE_REGISTER_FLECT( spect_txm_s );
            BCORE_REGISTER_FLECT( txm_plain_s );
            BCORE_REGISTER_FLECT( txm_chess_s );
        }
        break;

        default: break;
    }
    return NULL;
}

/**********************************************************************************************************************/


