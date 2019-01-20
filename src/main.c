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

#include "bcore_std.h"

#include "vectors.h"
#include "objects.h"
#include "compound.h"
#include "textures.h"
#include "scene.h"
#include "interpreter.h"
#include "container.h"
#include "closures.h"
#include "gmath.h"
#include "quicktypes.h"
#include "distance.h"

vd_t main_signal_handler( const bcore_signal_s* o )
{
    bcore_fp_signal_handler arr[] =
    {
        vectors_signal_handler,
        textures_signal_handler,
        objects_signal_handler,
        compound_signal_handler,
        scene_signal_handler,
        interpreter_signal_handler,
        container_signal_handler,
        closures_signal_handler,
        gmath_signal_handler,
        distance_signal_handler,
    };
    return bcore_signal_s_broadcast( o, arr, sizeof( arr ) / sizeof( bcore_fp_signal_handler ) );
}

void run_selftest()
{
    bcore_register_signal_handler( bclos_signal_handler );
    bcore_register_signal_handler( main_signal_handler );
    st_s_print_d( bcore_run_signal_selftest( typeof( "interpreter" ), NULL ) );
    bcore_down( false );
    exit( 0 );
}

void run_quicktypes()
{
    bcore_register_signal_handler( bclos_signal_handler );
    bcore_register_signal_handler( main_signal_handler );
    quicktypes_to_stdout( NULL );
    bcore_down( false );
    exit( 0 );
}

int main( int argc, const char** argv )
{
//    run_selftest();
//    run_quicktypes();

    bcore_register_signal_handler( bclos_signal_handler );
    bcore_register_signal_handler( main_signal_handler );

    bcore_msg( "ACTINON: Ray-tracer.\n" );
    bcore_msg( "Copyright (C) 2017 ... 2019 Johannes B. Steffens.\n\n" );

    if( argc < 2 )
    {
        bcore_msg( "Usage: actinon <script file> [-f]\n" );
        return 1;
    }

    bcore_life_s* l = bcore_life_s_create();
    st_s* in_file  = bcore_life_s_push_aware( l, st_s_create_sc( argv[ 1 ] ) );

    for( uz_t i = 0; i < 2; i++ ) bcore_arr_st_s_push_sc( interpreter_args_g, argv[ i ] );

    for( uz_t i = 2; i < argc; i++ )
    {
        st_s* arg = st_s_create_sc( argv[ i ] );
        if( st_s_equal_sc( arg, "-f" ) )
        {
            scene_s_overwrite_output_files_g = true;
        }
        else
        {
            bcore_arr_st_s_push_sc( interpreter_args_g, argv[ i ] );
        }
        st_s_discard( arg );
    }

    bcore_msg_fa( "Processing '#<st_s*>'\n", in_file  );
    bcore_life_s_push_sr( l, bcore_interpret_auto_file( in_file->sc ) );
    bcore_life_s_discard( l );

    bcore_down( false );
    return 0;
}
