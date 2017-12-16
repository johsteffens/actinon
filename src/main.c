/** main.c
 *  Author: Johannes Bernhard Steffens
 *
 *  Copyright (c) 2017 Johannes Bernhard Steffens. All rights reserved.
 */

#include "bcore.h"
#include "bcore_signal.h"
#include "bcore_st.h"
#include "bcore_name_manager.h"
#include "bcore_life.h"
#include "bcore_txt_ml.h"
#include "bcore_spect_interpreter.h"

#include "vectors.h"
#include "objects.h"
#include "textures.h"
#include "scene.h"
#include "interpreter.h"
#include "container.h"
#include "closures.h"
#include "quicktypes.h"

void selftest( const char* name )
{
    st_s* log = bcore_signal( typeof( name ), typeof( "selftest" ), NULL );
    st_s_print_d( log );
}

vd_t signal( tp_t target, tp_t signal, vd_t object )
{
    vd_t ret = NULL;
    if( ( ret = vectors_signal(     target, signal, object ) ) ) return ret;
    if( ( ret = textures_signal(    target, signal, object ) ) ) return ret;
    if( ( ret = objects_signal(     target, signal, object ) ) ) return ret;
    if( ( ret = scene_signal(       target, signal, object ) ) ) return ret;
    if( ( ret = interpreter_signal( target, signal, object ) ) ) return ret;
    if( ( ret = container_signal(   target, signal, object ) ) ) return ret;
    if( ( ret = closures_signal(    target, signal, object ) ) ) return ret;
    return ret;
}

int main( int argc, const char** argv )
{
    bcore_library_init( signal );
    {
//        st_s_print_d( signal( typeof( "all" ), typeof( "selftest" ), NULL ) );
//        quicktypes_to_stdout( NULL );
//        bcore_library_down( false );
//        return 0;
    }

    bcore_msg( "RAYFLUX: Ray-tracer.\n" );
    bcore_msg( "Copyright (C) 2017 Johannes B. Steffens.\n\n" );

    if( argc < 2 )
    {
        bcore_msg( "Format: rayflux <config file>\n" );
        return 1;
    }

    bcore_life_s* l = bcore_life_s_create();
    st_s* in_file  = bcore_life_s_push_aware( l, st_s_create_sc( argv[ 1 ] ) );
    st_s* out_name = bcore_life_s_push_aware( l, st_s_create_sc( in_file->sc ) );

    st_s* png_file = bcore_life_s_push_aware( l, st_s_create_fa( "#<st_s*>.pnm", out_name ) );
//    st_s* map_file = bcore_life_s_push_aware( l, st_s_create_fa( "#<st_s*>_photon.pnm", out_name ) );

    bcore_msg_fa( "reading '#<st_s*>'\n", in_file  );
    sr_s obj = bcore_life_s_push_sr( l, bcore_interpret_auto_file( in_file->sc ) );
    if( !obj.o ) ERR( "File '%s' did not yield an object.", in_file->sc );
    if( sr_s_type( &obj ) != typeof( "scene_s" ) )
    {
        ERR( "File '%s' yielded object '%s' (scene_s was expected).", in_file->sc, ifnameof( sr_s_type( &obj ) ) );
    }

    scene_s* scene = obj.o;

//    bcore_txt_ml_to_stdout( sr_awc( scene ) );
//    return 0;

    bcore_msg_fa( "creating photon map:" );
    scene_s_create_photon_map( scene );
//    image_cps_s* photon_image = scene_s_show_photon_map( scene );
//    bcore_msg_fa( "writing '#<st_s*>'\n", map_file  );
//    image_cps_s_write_pnm( photon_image, map_file->sc );
//    image_cps_s_discard( photon_image );

    image_cps_s* image = scene_s_create_image( scene );
    bcore_msg_fa( "writing '#<st_s*>'\n", png_file  );
    image_cps_s_write_pnm( image, png_file->sc );
    image_cps_s_discard( image );

    bcore_life_s_discard( l );

    bcore_library_down( false );
    return 0;
}
