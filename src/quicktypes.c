/// Author & Copyright (C) 2017 Johannes Bernhard Steffens. All rights reserved.

#include "bcore_quicktypes.h"
#include "bcore_st.h"
#include "bcore_spect_array.h"
#include "bcore_spect_inst.h"

/**********************************************************************************************************************/

typedef tp_t (*hf)( sc_t );

static st_s* get_def_quicktype( hf hash, sr_s string, sz_t align )
{
    sc_t name = ( ( st_s* )string.o )->sc;
    st_s* s = st_s_createf( "#define TYPEOF_%s", name );
    sz_t pad = s->size < align ? align - s->size : 1;
    st_s_push_char_n( s, ' ', pad );
    st_s_pushf( s, "% 10"PRIu32, hash( name ) );
    st_s_push_char( s, '\n' );
    sr_down( string );
    return s;
}

static sr_s typelist()
{
    sr_s list = bcore_inst_typed_create_sr( bcore_flect_type_parse_fa( "{ st_s * [] arr; }" ) );
    bcore_array_q_push_sc( &list, "v3d_s" );
    bcore_array_q_push_sc( &list, "spect_txm_s" );
    bcore_array_q_push_sc( &list, "txm_plain_s" );
    bcore_array_q_push_sc( &list, "txm_chess_s" );
    bcore_array_q_push_sc( &list, "scene_s" );
    bcore_array_q_push_sc( &list, "spect_obj_s" );
    bcore_array_q_push_sc( &list, "spect_obj" );
    bcore_array_q_push_sc( &list, "properties_s" );
    bcore_array_q_push_sc( &list, "compound_s" );
    bcore_array_q_push_sc( &list, "meval_s" );
    bcore_array_q_push_sc( &list, "mclosure_s" );
    bcore_array_q_push_sc( &list, "arr_s" );
    bcore_array_q_push_sc( &list, "map_s" );

    bcore_array_q_push_sc( &list, "clear" );
    bcore_array_q_push_sc( &list, "push" );
    bcore_array_q_push_sc( &list, "move" );
    bcore_array_q_push_sc( &list, "rotate" );
    bcore_array_q_push_sc( &list, "scale" );
    bcore_array_q_push_sc( &list, "true" );
    bcore_array_q_push_sc( &list, "false" );

    bcore_array_q_push_sc( &list, "NOT" );
    bcore_array_q_push_sc( &list, "AND" );
    bcore_array_q_push_sc( &list, "OR" );
    bcore_array_q_push_sc( &list, "XOR" );

    bcore_array_q_push_sc( &list, "if" );
    bcore_array_q_push_sc( &list, "while" );
    bcore_array_q_push_sc( &list, "else" );

    bcore_array_q_push_sc( &list, "vec" );
    bcore_array_q_push_sc( &list, "color" );

//    bcore_array_q_sort( &list, 0, -1, 1 );
    return list;
}

static sz_t max_len( const sr_s* list )
{
    sz_t len = 0;
    for( sz_t i = 0; i < bcore_array_q_get_size( list ); i++ )
    {
        sz_t size = ( ( st_s* )bcore_array_q_get( list, i ).o )->size;
        len = size > len ? size : len;
    }
    return len;
}

void quicktypes_to_stdout( tp_t (*hash)( sc_t name ) )
{
    hf hash_l = ( hash ) ? hash : typeof;
    sr_s list = typelist();
    for( sz_t i = 0; i < bcore_array_q_get_size( &list ); i++ ) st_s_print_d( get_def_quicktype( hash_l, bcore_array_q_get( &list, i ), 16 + max_len( &list ) ) );
    sr_down( list );
}

/**********************************************************************************************************************/


