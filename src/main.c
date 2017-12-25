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

#include <time.h>

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
#include "gmath.h"
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
    if( ( ret = gmath_signal(       target, signal, object ) ) ) return ret;
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

    bcore_msg_fa( "Processing '#<st_s*>'\n", in_file  );
    sr_s obj = bcore_life_s_push_sr( l, bcore_interpret_auto_file( in_file->sc ) );
    if( sr_s_type( &obj ) == typeof( "scene_s" ) )
    {
        scene_s* scene = obj.o;
        image_cps_s* image = scene_s_create_image( scene );
        bcore_msg_fa( "writing '#<st_s*>'\n", png_file  );
        image_cps_s_write_pnm( image, png_file->sc );
        image_cps_s_discard( image );
    }

    bcore_life_s_discard( l );

    bcore_library_down( false );
    return 0;
}
