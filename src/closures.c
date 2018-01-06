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


#include "closures.h"
#include "vectors.h"

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

/// exp( x )
static sr_s exp_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 1 );
    return sr_f3( exp( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ) ) );
}

DEFINE_STD_CLOSURE( exp_s, "f3_t exp_s( num val )", exp_s_call )

/**********************************************************************************************************************/

/// pow( x, y )
static sr_s pow_s_call( vc_t o, bclos_frame_s* frm, const bclos_arguments_s* args )
{
    ASSERT( args->size == 2 );
    return sr_f3( pow( sr_f3_sr( bclos_arguments_s_get( args, 0, frm ) ), sr_f3_sr( bclos_arguments_s_get( args, 1, frm ) ) ) );
}

DEFINE_STD_CLOSURE( pow_s, "f3_t pow_s( num base, num exp )", pow_s_call )

/**********************************************************************************************************************/

vd_t closures_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "closures" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
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
        bcore_flect_define_creator( typeof( "create_string_fa_s" ), create_string_fa_s_create_self );
        bcore_flect_define_creator( typeof( "sqrt_s"             ), sqrt_s_create_self );
        bcore_flect_define_creator( typeof( "exp_s"              ), exp_s_create_self );
        bcore_flect_define_creator( typeof( "pow_s"              ), pow_s_create_self  );
    }

    return NULL;
}

/**********************************************************************************************************************/
