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

DEFINE_FUNCTIONS_OBJ_INST( distance_sphere_s )

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

static bcore_flect_self_s* distance_sphere_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( distance_sphere_s_def, sizeof( distance_sphere_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )distance_sphere_s_init_a, "ap_t", "init" );
    return self;
}

/**********************************************************************************************************************/

vd_t distance_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "distance" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        bcore_trait_set( entypeof( "distance" ), entypeof( "bcore_inst" ) );
        bcore_flect_define_creator( typeof( "distance_sphere_s" ), distance_sphere_s_create_self );
    }

    return NULL;
}

/**********************************************************************************************************************/



