/// Author & Copyright (C) 2017 Johannes Bernhard Steffens. All rights reserved.

#include "scene.h"
#include "bcore.h"
#include "bcore_signal.h"
#include "bcore_st.h"
#include "bcore_name_manager.h"
#include "bcore_life.h"
#include "bcore_txt_ml.h"

void selftest( const char* name )
{
    st_s* log = bcore_signal( typeof( name ), typeof( "selftest" ), NULL );
    st_s_print_d( log );
}

vd_t signal( tp_t target, tp_t signal, vd_t object )
{
    vd_t ret = NULL;
    if( ( ret = vectors_signal( target, signal, object ) ) ) return ret;
    if( ( ret = scene_signal(   target, signal, object ) ) ) return ret;
    return ret;
}

int main( int argc, const char** argv )
{
    bcore_library_init( signal );
    bcore_msg( "RAYS: Ray-tracing aided design.\n" );
    bcore_msg( "Copyright (C) 2017 Johannes B. Steffens.\n\n" );

    if( argc < 2 ) ERR( "Format: rays <config file>\n" );

    bcore_life_s* l = bcore_life_s_create();
    st_s* in_file  = bcore_life_s_push_aware( l, st_s_create_sc( argv[ 1 ] ) );
    st_s* out_name = bcore_life_s_push_aware( l, st_s_create_sc( in_file->sc ) );

    st_s* png_file = bcore_life_s_push_aware( l, st_s_create_fa( "#<st_s*>.pnm", out_name ) );
//    st_s* map_file = bcore_life_s_push_aware( l, st_s_create_fa( "#<st_s*>_photon.pnm", out_name ) );

    bcore_msg_fa( "reading '#<st_s*>'\n", in_file  );
    scene_s* scene = bcore_life_s_push_sr( l, bcore_txt_ml_from_file( in_file->sc ) ).o;

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
