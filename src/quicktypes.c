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

#include "bcore_quicktypes.h"
#include "bcore_st.h"
#include "bcore_spect_array.h"
#include "bcore_spect_inst.h"

/**********************************************************************************************************************/

typedef tp_t (*hf)( sc_t );

static st_s* get_def_quicktype( hf hash, sr_s string, uz_t align )
{
    sc_t name = ( ( st_s* )string.o )->sc;
    st_s* s = st_s_createf( "#define TYPEOF_%s", name );
    uz_t pad = s->size < align ? align - s->size : 1;
    st_s_push_char_n( s, ' ', pad );
    st_s_pushf( s, "% 10"PRIu32, hash( name ) );
    st_s_push_char( s, '\n' );
    sr_down( string );
    return s;
}

static sr_s typelist()
{
    sr_s list = bcore_inst_t_create_sr( bcore_flect_type_parse_fa( "{ st_s * [] arr; }" ) );
    bcore_array_r_push_sc( &list, "v3d_s" );
    bcore_array_r_push_sc( &list, "m3d_s" );
    bcore_array_r_push_sc( &list, "spect_txm_s" );
    bcore_array_r_push_sc( &list, "txm_plain_s" );
    bcore_array_r_push_sc( &list, "txm_chess_s" );
    bcore_array_r_push_sc( &list, "scene_s" );
    bcore_array_r_push_sc( &list, "spect_obj_s" );
    bcore_array_r_push_sc( &list, "spect_obj" );
    bcore_array_r_push_sc( &list, "obj_plane_s" );
    bcore_array_r_push_sc( &list, "obj_sphere_s" );
    bcore_array_r_push_sc( &list, "obj_squaroid_s" );
    bcore_array_r_push_sc( &list, "obj_distance_s" );
    bcore_array_r_push_sc( &list, "obj_pair_inside_s" );
    bcore_array_r_push_sc( &list, "obj_pair_outside_s" );
    bcore_array_r_push_sc( &list, "obj_neg_s" );
    bcore_array_r_push_sc( &list, "obj_scale_s" );

    bcore_array_r_push_sc( &list, "properties_s" );
    bcore_array_r_push_sc( &list, "compound_s" );
    bcore_array_r_push_sc( &list, "meval_s" );
    bcore_array_r_push_sc( &list, "mclosure_s" );
    bcore_array_r_push_sc( &list, "arr_s" );
    bcore_array_r_push_sc( &list, "map_s" );
    bcore_array_r_push_sc( &list, "lum_s" );
    bcore_array_r_push_sc( &list, "lum_arr_s" );
    bcore_array_r_push_sc( &list, "lum_map_s" );

    bcore_array_r_push_sc( &list, "clear" );
    bcore_array_r_push_sc( &list, "push" );
    bcore_array_r_push_sc( &list, "move" );
    bcore_array_r_push_sc( &list, "rotate" );
    bcore_array_r_push_sc( &list, "scale" );
    bcore_array_r_push_sc( &list, "true" );
    bcore_array_r_push_sc( &list, "false" );

    bcore_array_r_push_sc( &list, "NOT" );
    bcore_array_r_push_sc( &list, "AND" );
    bcore_array_r_push_sc( &list, "OR" );
    bcore_array_r_push_sc( &list, "XOR" );
    bcore_array_r_push_sc( &list, "CAT" );

    bcore_array_r_push_sc( &list, "def" );
    bcore_array_r_push_sc( &list, "if" );
    bcore_array_r_push_sc( &list, "while" );
    bcore_array_r_push_sc( &list, "else" );
    bcore_array_r_push_sc( &list, "for" );
    bcore_array_r_push_sc( &list, "in" );
    bcore_array_r_push_sc( &list, "of" );

    bcore_array_r_push_sc( &list, "vec" );
    bcore_array_r_push_sc( &list, "set_color" );
    bcore_array_r_push_sc( &list, "set_refractive_index" );
    bcore_array_r_push_sc( &list, "set_radiance" );
    bcore_array_r_push_sc( &list, "set_transparency" );
    bcore_array_r_push_sc( &list, "set_texture_field" );

    bcore_array_r_push_sc( &list, "create_image" );

//    bcore_array_r_sort( &list, 0, -1, 1 );
    return list;
}

static uz_t max_len( const sr_s* list )
{
    uz_t len = 0;
    for( uz_t i = 0; i < bcore_array_r_get_size( list ); i++ )
    {
        uz_t size = ( ( st_s* )bcore_array_r_get( list, i ).o )->size;
        len = size > len ? size : len;
    }
    return len;
}

void quicktypes_to_stdout( tp_t (*hash)( sc_t name ) )
{
    hf hash_l = ( hash ) ? hash : typeof;
    sr_s list = typelist();
    for( uz_t i = 0; i < bcore_array_r_get_size( &list ); i++ ) st_s_print_d( get_def_quicktype( hash_l, bcore_array_r_get( &list, i ), 16 + max_len( &list ) ) );
    sr_down( list );
}

/**********************************************************************************************************************/


