/** Distance Functions */

/** Copyright 2018 Johannes Bernhard Steffens
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

#include "bcore_trait.h"
#include "distance.h"

/**********************************************************************************************************************/

#define TYPEOF_distance_sphere_s typeof( "distance_sphere_s" )
typedef struct distance_sphere_s
{
    aware_t _;
    distance_fp fp_distance;
} distance_sphere_s;

static sc_t distance_sphere_s_def =
"distance_sphere_s = distance"
"{"
    "aware_t _;"
    "fp_t fp_distance;"
"}";

BCORE_DEFINE_FUNCTIONS_OBJ_INST( distance_sphere_s )

f3_t distance_sphere_s_call( const distance_sphere_s* o, const v3d_s* pos )
{
    return sqrt( f3_sqr( pos->x ) + f3_sqr( pos->y ) + f3_sqr( pos->z ) ) - 1.0;
}

static void distance_sphere_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; distance_sphere_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
    nc_l->o->fp_distance = ( distance_fp )distance_sphere_s_call;
}

static bcore_self_s* distance_sphere_s_create_self( void )
{
    bcore_self_s* self = bcore_self_s_build_parse_sc( distance_sphere_s_def, sizeof( distance_sphere_s ) );
    bcore_self_s_push_ns_func( self, ( fp_t )distance_sphere_s_init_a, "ap_t", "init" );
    return self;
}

/**********************************************************************************************************************/

#define TYPEOF_distance_torus_s typeof( "distance_torus_s" )
typedef struct distance_torus_s
{
    aware_t _;
    distance_fp fp_distance;
    f3_t ex_radius; // ex-planar radius
} distance_torus_s;

static sc_t distance_torus_s_def =
"distance_torus_s = distance"
"{"
    "aware_t _;"
    "fp_t fp_distance;"
    "f3_t ex_radius = 0.5;" // ex-planar radius
"}";

BCORE_DEFINE_FUNCTIONS_OBJ_INST( distance_torus_s )

void distance_torus_s_set_ex_radius( distance_torus_s* o, f3_t radius )
{
    o->ex_radius = radius;
}

f3_t distance_torus_s_call( const distance_torus_s* o, const v3d_s* pos )
{
    f3_t x = pos->x;
    f3_t y = pos->y;
    f3_t f = sqrt( x * x + y * y );
    f3_t f_inv = ( f > 0 ) ? ( 1.0 / f ) : 1.0;
    x *= f_inv;
    y *= f_inv;
    return sqrt( f3_sqr( x - pos->x ) + f3_sqr( y - pos->y ) + f3_sqr( pos->z ) ) - o->ex_radius;
}

static void distance_torus_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; distance_torus_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
    nc_l->o->fp_distance = ( distance_fp )distance_torus_s_call;
}

static bcore_self_s* distance_torus_s_create_self( void )
{
    bcore_self_s* self = bcore_self_s_build_parse_sc( distance_torus_s_def, sizeof( distance_torus_s ) );
    bcore_self_s_push_ns_func( self, ( fp_t )distance_torus_s_init_a, "ap_t", "init" );
    return self;
}

/**********************************************************************************************************************/

vd_t distance_signal_handler( const bcore_signal_s* o )
{
    switch( bcore_signal_s_handle_type( o, typeof( "distance" ) ) )
    {
        case TYPEOF_init1:
        {
            bcore_trait_set( entypeof( "distance" ), entypeof( "bcore_inst" ) );
            BCORE_REGISTER_FLECT( distance_sphere_s );
            BCORE_REGISTER_FLECT( distance_torus_s );
        }
        break;

        default: break;
    }
    return NULL;
}

/**********************************************************************************************************************/



