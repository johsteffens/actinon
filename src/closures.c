/** Closures
 *  Author: Johannes Bernhard Steffens
 *
 *  Copyright (c) 2017 Johannes Bernhard Steffens. All rights reserved.
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

vd_t closures_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "closures" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        bcore_flect_define_creator( typeof( "create_vec_s"   ), create_vec_s_create_self );
        bcore_flect_define_creator( typeof( "create_color_s" ), create_color_s_create_self );
    }

    return NULL;
}

/**********************************************************************************************************************/
