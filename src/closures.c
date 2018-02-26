/** Closures */

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

#include "bcore_sources.h"

#include "closures.h"
#include "vectors.h"
#include "objects.h"
#include "distance.h"

/**********************************************************************************************************************/

/// creates v3d_s
static sr_s create_vec_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 3 );
    sr_s ret = sr_create( TYPEOF_v3d_s );
    ( ( v3d_s* )( ret.o ) )->x = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    ( ( v3d_s* )( ret.o ) )->y = sr_f3_sr( bclos_arguments_s_get( args, 1, frm ) );
    ( ( v3d_s* )( ret.o ) )->z = sr_f3_sr( bclos_arguments_s_get( args, 2, frm ) );
    return ret;
}
DEFINE_STD_CLOSURE( create_vec_s, "v3d_s create_vec_s( num x, num y, num z )", create_vec_s_call )

/**********************************************************************************************************************/

/// vectors from vector components
static sr_s vecx_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s ret = sr_create( TYPEOF_v3d_s );
    ( ( v3d_s* )ret.o )->x = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    return ret;
}
static sr_s vecy_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s ret = sr_create( TYPEOF_v3d_s );
    ( ( v3d_s* )ret.o )->y = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    return ret;
}
static sr_s vecz_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s ret = sr_create( TYPEOF_v3d_s );
    ( ( v3d_s* )ret.o )->z = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    return ret;
}

DEFINE_STD_CLOSURE( vecx_s, "v3d_s vecx_s( num v )", vecx_s_call )
DEFINE_STD_CLOSURE( vecy_s, "v3d_s vecy_s( num v )", vecy_s_call )
DEFINE_STD_CLOSURE( vecz_s, "v3d_s vecz_s( num v )", vecz_s_call )

/**********************************************************************************************************************/

/// colors from color components
static sr_s colr_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s ret = sr_create( TYPEOF_cl_s );
    ( ( cl_s* )ret.o )->x = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    return ret;
}
static sr_s colg_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s ret = sr_create( TYPEOF_cl_s );
    ( ( cl_s* )ret.o )->y = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    return ret;
}
static sr_s colb_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s ret = sr_create( TYPEOF_cl_s );
    ( ( cl_s* )ret.o )->z = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    return ret;
}

DEFINE_STD_CLOSURE( colr_s, "cl_s colr_s( num v )", colr_s_call )
DEFINE_STD_CLOSURE( colg_s, "cl_s colg_s( num v )", colg_s_call )
DEFINE_STD_CLOSURE( colb_s, "cl_s colb_s( num v )", colb_s_call )

/**********************************************************************************************************************/

/// creates a cl_s
static sr_s create_color_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 3 );
    sr_s ret = sr_create( TYPEOF_cl_s );
    ( ( cl_s* )( ret.o ) )->x = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    ( ( cl_s* )( ret.o ) )->y = sr_f3_sr( bclos_arguments_s_get( args, 1, frm ) );
    ( ( cl_s* )( ret.o ) )->z = sr_f3_sr( bclos_arguments_s_get( args, 2, frm ) );
    return ret;
}
DEFINE_STD_CLOSURE( create_color_s, "v3d_s create_color_s( num x, num y, num z )", create_color_s_call )

/**********************************************************************************************************************/

/// rotations around x,y,z
static sr_s rotx_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s ret = sr_create( TYPEOF_m3d_s );
    *( m3d_s* )ret.o = m3d_s_rot_x( ( M_PI / 180.0 ) * sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) );
    return ret;
}
static sr_s roty_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s ret = sr_create( TYPEOF_m3d_s );
    *( m3d_s* )ret.o = m3d_s_rot_y( ( M_PI / 180.0 ) * sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) );
    return ret;
}
static sr_s rotz_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s ret = sr_create( TYPEOF_m3d_s );
    *( m3d_s* )ret.o = m3d_s_rot_z( ( M_PI / 180.0 ) * sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) );
    return ret;
}

DEFINE_STD_CLOSURE( rotx_s, "m3d_s rotx_s( num v )", rotx_s_call )
DEFINE_STD_CLOSURE( roty_s, "m3d_s roty_s( num v )", roty_s_call )
DEFINE_STD_CLOSURE( rotz_s, "m3d_s rotz_s( num v )", rotz_s_call )

/**********************************************************************************************************************/

/// creates formatted st_s with format string and format argument
static sr_s create_string_fa_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 2 );
    sr_s arg0 = bclos_arguments_s_get( args, 0, frm );
    sr_s arg1 = bclos_arguments_s_get( args, 1, frm );
    st_s* string = st_s_create_fa( ( ( st_s* )arg0.o )->sc, arg1.o );
    sr_down( arg0 );
    sr_down( arg1 );
    return sr_tsd( TYPEOF_st_s, string );
}

DEFINE_STD_CLOSURE( create_string_fa_s, "v3d_s create_string_fa_s( st_s format, root arg )", create_string_fa_s_call )

/**********************************************************************************************************************/

/// sqrt( x )
static sr_s sqrt_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( sqrt( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( sqrt_s, "f3_t sqrt_s( num val )", sqrt_s_call )

/**********************************************************************************************************************/

/// sqr( x ) = x * x
static sr_s sqr_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( f3_sqr( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( sqr_s, "f3_t sqr_s( num val )", sqr_s_call )

/**********************************************************************************************************************/

/// exp( x )
static sr_s exp_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( exp( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( exp_s, "f3_t exp_s( num val )", exp_s_call )

/**********************************************************************************************************************/

/// log( x )
static sr_s log_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( log( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( log_s, "f3_t log_s( num val )", log_s_call )

/**********************************************************************************************************************/

/// to_deg( v_rad )  (converts radiant -> degree)
static sr_s to_deg_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) * 180.0 / M_PI );
}

DEFINE_STD_CLOSURE( to_deg_s, "f3_t to_deg_s( num val )", to_deg_s_call )

/**********************************************************************************************************************/

/// to_rad( v_deg )  (converts degree -> radiant)
static sr_s to_rad_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) * M_PI / 180.0 );
}

DEFINE_STD_CLOSURE( to_rad_s, "f3_t to_rad_s( num val )", to_rad_s_call )

/**********************************************************************************************************************/

/// sin( x )
static sr_s sin_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( sin( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( sin_s, "f3_t sin_s( num val )", sin_s_call )

/**********************************************************************************************************************/

/// cos( x )
static sr_s cos_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( cos( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( cos_s, "f3_t cos_s( num val )", cos_s_call )

/**********************************************************************************************************************/

/// tan( x )
static sr_s tan_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( tan( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( tan_s, "f3_t tan_s( num val )", tan_s_call )

/**********************************************************************************************************************/

/// sin_d( x )
static sr_s sin_d_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( sin( M_PI * sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) / 180.0 ) );
}

DEFINE_STD_CLOSURE( sin_d_s, "f3_t sin_d_s( num val )", sin_d_s_call )

/**********************************************************************************************************************/

/// cos_d( x )
static sr_s cos_d_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( cos( M_PI * sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) / 180.0 ) );
}

DEFINE_STD_CLOSURE( cos_d_s, "f3_t cos_d_s( num val )", cos_d_s_call )

/**********************************************************************************************************************/

/// tan_d( x )
static sr_s tan_d_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( tan( M_PI * sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) / 180.0 ) );
}

DEFINE_STD_CLOSURE( tan_d_s, "f3_t tan_d_s( num val )", tan_d_s_call )

/**********************************************************************************************************************/

/// asin( x )
static sr_s asin_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( asin( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( asin_s, "f3_t asin_s( num val )", asin_s_call )

/**********************************************************************************************************************/

/// acos( x )
static sr_s acos_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( acos( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( acos_s, "f3_t acos_s( num val )", acos_s_call )

/**********************************************************************************************************************/

/// atan( x )
static sr_s atan_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( atan( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( atan_s, "f3_t atan_s( num val )", atan_s_call )

/**********************************************************************************************************************/

/// pow( x, y )
static sr_s pow_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 2 );
    return sr_f3( pow( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ), sr_f3_sr( bclos_arguments_s_get( args, 1, frm ) ) ) );
}

DEFINE_STD_CLOSURE( pow_s, "f3_t pow_s( num base, num exp )", pow_s_call )

/**********************************************************************************************************************/

/// ceiling( x )
static sr_s ceiling_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( ceil( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( ceiling_s, "f3_t ceiling_s( num val )", ceiling_s_call )

/**********************************************************************************************************************/

/// floor( x )
static sr_s floor_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( floor( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( floor_s, "f3_t floor_s( num val )", floor_s_call )

/**********************************************************************************************************************/

/// file_exists( f )
static bl_t file_exists_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s arg0 = bclos_arguments_s_get( args, 0, frm );
    bl_t ret = bcore_source_file_s_exists( ( ( st_s* )arg0.o )->sc );
    sr_down( arg0 );
    return ret;
}

DEFINE_STD_CLOSURE( file_exists_s, "bl_t file_exists_s( st_s file )", file_exists_s_call )

/**********************************************************************************************************************/

/// file_delete( f )
static void file_delete_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s arg0 = bclos_arguments_s_get( args, 0, frm );
    bcore_source_file_s_delete( ( ( st_s* )arg0.o )->sc );
    sr_down( arg0 );
}

DEFINE_STD_CLOSURE( file_delete_s, "file_delete_s( st_s file )", file_delete_s_call )

/**********************************************************************************************************************/

/// beth_object( name-string ): creates a beth-object example: beth_object( "distance_torus_s" );
static sr_s create_beth_object_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    sr_s arg = bclos_arguments_s_get( args, 0, frm );
    sr_s r = sr_create( typeof( ( ( st_s* )arg.o )->sc ) );
    sr_down( arg );
    return r;
}

DEFINE_STD_CLOSURE( create_beth_object_s, "bcore_inst create_beth_object_s( st_s name )", create_beth_object_s_call )

/**********************************************************************************************************************/

static sr_s create_plane_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 0 );
    sr_s r = sr_create( typeof( "obj_plane_s" ) );
    return r;
}

DEFINE_STD_CLOSURE( create_plane_s, "spect_obj create_plane_s()", create_plane_s_call )

/**********************************************************************************************************************/

static sr_s create_sphere_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    f3_t radius = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    sr_s r = sr_create( typeof( "obj_sphere_s" ) );
    obj_sphere_s_set_radius( r.o, radius );
    return r;
}

DEFINE_STD_CLOSURE( create_sphere_s, "spect_obj create_sphere_s( num radius )", create_sphere_s_call )

/**********************************************************************************************************************/

static sr_s create_squaroid_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 4 );
    f3_t a = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    f3_t b = sr_f3_sr( bclos_arguments_s_get( args, 1, frm ) );
    f3_t c = sr_f3_sr( bclos_arguments_s_get( args, 2, frm ) );
    f3_t r = sr_f3_sr( bclos_arguments_s_get( args, 3, frm ) );
    sr_s ret = sr_asd( obj_squaroid_s_create_squaroid( a, b, c, r ) );
    return ret;
}

DEFINE_STD_CLOSURE( create_squaroid_s, "spect_obj create_squaroid_s( num a, num b, num c, num r )", create_squaroid_s_call )

/**********************************************************************************************************************/

static sr_s create_cylinder_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 2 );
    f3_t rx = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    f3_t ry = sr_f3_sr( bclos_arguments_s_get( args, 1, frm ) );
    sr_s r = sr_asd( obj_squaroid_s_create_cylinder( rx, ry ) );
    return r;
}

DEFINE_STD_CLOSURE( create_cylinder_s, "spect_obj create_cylinder_s( num radius_x, num radius_y )", create_cylinder_s_call )

/**********************************************************************************************************************/

static sr_s create_hyperboloid1_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 3 );
    f3_t rx = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    f3_t ry = sr_f3_sr( bclos_arguments_s_get( args, 1, frm ) );
    f3_t rz = sr_f3_sr( bclos_arguments_s_get( args, 2, frm ) );
    sr_s r = sr_asd( obj_squaroid_s_create_hyperboloid1( rx, ry, rz ) );
    return r;
}

DEFINE_STD_CLOSURE( create_hyperboloid1_s, "spect_obj create_hyperboloid1_s( num radius_x, num radius_y, num radius_z )", create_hyperboloid1_s_call )

/**********************************************************************************************************************/

static sr_s create_hyperboloid2_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 3 );
    f3_t rx = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    f3_t ry = sr_f3_sr( bclos_arguments_s_get( args, 1, frm ) );
    f3_t rz = sr_f3_sr( bclos_arguments_s_get( args, 2, frm ) );
    sr_s r = sr_asd( obj_squaroid_s_create_hyperboloid2( rx, ry, rz ) );
    return r;
}

DEFINE_STD_CLOSURE( create_hyperboloid2_s, "spect_obj create_hyperboloid2_s( num radius_x, num radius_y, num radius_z )", create_hyperboloid2_s_call )

/**********************************************************************************************************************/

static sr_s create_cone_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 3 );
    f3_t rx = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    f3_t ry = sr_f3_sr( bclos_arguments_s_get( args, 1, frm ) );
    f3_t rz = sr_f3_sr( bclos_arguments_s_get( args, 2, frm ) );
    sr_s r = sr_asd( obj_squaroid_s_create_cone( rx, ry, rz ) );
    return r;
}

DEFINE_STD_CLOSURE( create_cone_s, "spect_obj create_cone_s( num radius_x, num radius_y, num radius_z )", create_cone_s_call )

/**********************************************************************************************************************/

static sr_s create_ellipsoid_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 3 );
    f3_t rx = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    f3_t ry = sr_f3_sr( bclos_arguments_s_get( args, 1, frm ) );
    f3_t rz = sr_f3_sr( bclos_arguments_s_get( args, 2, frm ) );
    sr_s r = sr_asd( obj_squaroid_s_create_ellipsoid( rx, ry, rz ) );
    return r;
}

DEFINE_STD_CLOSURE( create_ellipsoid_s, "spect_obj create_ellipsoid_s( num radius_x, num radius_y, num radius_z )", create_ellipsoid_s_call )

/**********************************************************************************************************************/

static sr_s create_torus_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 2 );
    f3_t radius1 = sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) );
    f3_t radius2 = sr_f3_sr( bclos_arguments_s_get( args, 1, frm ) );
    sr_s r = sr_create( typeof( "obj_distance_s" ) );
    {
        distance_torus_s* distance_torus = distance_torus_s_create();
        distance_torus_s_set_ex_radius( distance_torus, radius2 / radius1 );
        obj_distance_s_set_distance( r.o, distance_torus );
        distance_torus_s_discard( distance_torus );
    }

    obj_scale( r.o, radius1 );

    {
        envelope_s* env = envelope_s_create();
        env->radius = ( radius1 + radius2 ) * 1.01;
        obj_set_envelope( r.o, env );
        envelope_s_discard( env );
    }

    return r;
}

DEFINE_STD_CLOSURE( create_torus_s, "spect_obj create_torus_s( num radius1, num radius2 )", create_torus_s_call )

/**********************************************************************************************************************/

vd_t closures_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "closures" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        // vectors, tensors
        bcore_flect_define_creator( typeof( "create_vec_s"       ), create_vec_s_create_self );
        bcore_flect_define_creator( typeof( "vecx_s"             ), vecx_s_create_self );
        bcore_flect_define_creator( typeof( "vecy_s"             ), vecy_s_create_self );
        bcore_flect_define_creator( typeof( "vecz_s"             ), vecz_s_create_self );
        bcore_flect_define_creator( typeof( "create_color_s"     ), create_color_s_create_self );
        bcore_flect_define_creator( typeof( "colr_s"             ), colr_s_create_self );
        bcore_flect_define_creator( typeof( "colg_s"             ), colg_s_create_self );
        bcore_flect_define_creator( typeof( "colb_s"             ), colb_s_create_self );
        bcore_flect_define_creator( typeof( "rotx_s"             ), rotx_s_create_self );
        bcore_flect_define_creator( typeof( "roty_s"             ), roty_s_create_self );
        bcore_flect_define_creator( typeof( "rotz_s"             ), rotz_s_create_self );

        // string
        bcore_flect_define_creator( typeof( "create_string_fa_s" ), create_string_fa_s_create_self );

        // math
        bcore_flect_define_creator( typeof( "sqrt_s"             ), sqrt_s_create_self );
        bcore_flect_define_creator( typeof( "sqr_s"              ), sqr_s_create_self  );
        bcore_flect_define_creator( typeof( "exp_s"              ), exp_s_create_self  );
        bcore_flect_define_creator( typeof( "log_s"              ), log_s_create_self  );
        bcore_flect_define_creator( typeof( "to_deg_s"           ), to_deg_s_create_self );
        bcore_flect_define_creator( typeof( "to_rad_s"           ), to_rad_s_create_self );
        bcore_flect_define_creator( typeof( "sin_s"              ), sin_s_create_self  );
        bcore_flect_define_creator( typeof( "cos_s"              ), cos_s_create_self  );
        bcore_flect_define_creator( typeof( "tan_s"              ), tan_s_create_self  );
        bcore_flect_define_creator( typeof( "sin_d_s"            ), sin_d_s_create_self );
        bcore_flect_define_creator( typeof( "cos_d_s"            ), cos_d_s_create_self );
        bcore_flect_define_creator( typeof( "tan_d_s"            ), tan_d_s_create_self );
        bcore_flect_define_creator( typeof( "asin_s"             ), asin_s_create_self );
        bcore_flect_define_creator( typeof( "acos_s"             ), acos_s_create_self );
        bcore_flect_define_creator( typeof( "atan_s"             ), atan_s_create_self );
        bcore_flect_define_creator( typeof( "pow_s"              ), pow_s_create_self  );
        bcore_flect_define_creator( typeof( "ceiling_s"          ), ceiling_s_create_self  );
        bcore_flect_define_creator( typeof( "floor_s"            ), floor_s_create_self  );

        // files
        bcore_flect_define_creator( typeof( "file_exists_s"      ), file_exists_s_create_self );
        bcore_flect_define_creator( typeof( "file_delete_s"      ), file_delete_s_create_self );

        // objects
        bcore_flect_define_creator( typeof( "create_beth_object_s" ), create_beth_object_s_create_self  );
        bcore_flect_define_creator( typeof( "create_plane_s"        ), create_plane_s_create_self        );
        bcore_flect_define_creator( typeof( "create_sphere_s"       ), create_sphere_s_create_self       );
        bcore_flect_define_creator( typeof( "create_squaroid_s"     ), create_squaroid_s_create_self     );
        bcore_flect_define_creator( typeof( "create_cylinder_s"     ), create_cylinder_s_create_self     );
        bcore_flect_define_creator( typeof( "create_torus_s"        ), create_torus_s_create_self        );
        bcore_flect_define_creator( typeof( "create_hyperboloid1_s" ), create_hyperboloid1_s_create_self );
        bcore_flect_define_creator( typeof( "create_hyperboloid2_s" ), create_hyperboloid2_s_create_self );
        bcore_flect_define_creator( typeof( "create_ellipsoid_s"    ), create_ellipsoid_s_create_self    );
        bcore_flect_define_creator( typeof( "create_cone_s"         ), create_cone_s_create_self         );
    }

    return NULL;
}

/**********************************************************************************************************************/
