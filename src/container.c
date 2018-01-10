/** Container of objects */

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


#include "bcore_tp.h"
#include "bcore_name_manager.h"
#include "bcore_spect_source.h"
#include "bcore_spect_inst.h"
#include "bcore_arr.h"
#include "bcore_quicktypes.h"
#include "bcore_life.h"
#include "bcore_txt_ml.h"
#include "bcore_spect_via.h"
#include "bcore_trait.h"

#include "vectors.h"
#include "objects.h"
#include "container.h"

/**********************************************************************************************************************/
// map of objects

static sc_t map_s_def =
"map_s = bcore_inst"
"{"
    "aware_t _;"
    "bcore_hmap_tp_sr_s m;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( map_s )
DEFINE_CREATE_SELF( map_s, map_s_def )

bl_t map_s_exists( const map_s* o, tp_t key )
{
    return bcore_hmap_tp_sr_s_exists( &o->m, key );
}

sr_s* map_s_get( map_s* o, tp_t key )
{
    return bcore_hmap_tp_sr_s_get( &o->m, key );
}

void map_s_set( map_s* o, tp_t key, sr_s obj )
{
    bcore_hmap_tp_sr_s_set( &o->m, key, obj );
}

void map_s_move( map_s* o, const v3d_s* vec )
{
    sz_t size = bcore_hmap_tp_sr_s_size( &o->m );
    for( sz_t i = 0; i < size; i++ )
    {
        const sr_s* sr = bcore_hmap_tp_sr_s_idx_val( &o->m, i );
        if( sr )
        {
            tp_t type = sr_s_type( sr );
            if( type == TYPEOF_map_s )
            {
                map_s_move( sr->o, vec );
            }
            else if( type == TYPEOF_arr_s )
            {
                arr_s_move( sr->o, vec );
            }
            else if( bcore_trait_is_of( sr_s_type( sr->o ), TYPEOF_spect_obj ) )
            {
                obj_move( sr->o, vec );
            }
        }
    }
}

void map_s_rotate( map_s* o, const m3d_s* mat )
{
    sz_t size = bcore_hmap_tp_sr_s_size( &o->m );
    for( sz_t i = 0; i < size; i++ )
    {
        const sr_s* sr = bcore_hmap_tp_sr_s_idx_val( &o->m, i );
        if( sr )
        {
            tp_t type = sr_s_type( sr );
            if( type == TYPEOF_map_s )
            {
                map_s_rotate( sr->o, mat );
            }
            else if( type == TYPEOF_arr_s )
            {
                arr_s_rotate( sr->o, mat );
            }
            else if( bcore_trait_is_of( sr_s_type( sr->o ), TYPEOF_spect_obj ) )
            {
                obj_rotate( sr->o, mat );
            }
        }
    }
}

void map_s_scale( map_s* o, f3_t fac )
{
    sz_t size = bcore_hmap_tp_sr_s_size( &o->m );
    for( sz_t i = 0; i < size; i++ )
    {
        const sr_s* sr = bcore_hmap_tp_sr_s_idx_val( &o->m, i );
        if( sr )
        {
            tp_t type = sr_s_type( sr );
            if( type == TYPEOF_map_s )
            {
                map_s_scale( sr->o, fac );
            }
            else if( type == TYPEOF_arr_s )
            {
                arr_s_scale( sr->o, fac );
            }
            else if( bcore_trait_is_of( sr_s_type( sr->o ), TYPEOF_spect_obj ) )
            {
                obj_scale( sr->o, fac );
            }
        }
    }
}

sr_s map_s_meval_key( sr_s* sr_o, meval_s* ev, tp_t key )
{
    if( !sr_o ) return sr_null();
    assert( sr_s_type( sr_o ) == TYPEOF_map_s );
    map_s* o = sr_o->o;

    sr_s obj = sr_null();

    if( map_s_exists( o, key ) )
    {
        obj = sr_s_fork( map_s_get( o, key ) );
    }
    else if( meval_s_try_code( ev, OP_ASSIGN ) )
    {
        map_s_set( o, key, sr_clone( meval_s_eval( ev, sr_null() ) ) );
        obj = sr_s_fork( map_s_get( o, key ) );
    }
    else if( key == TYPEOF_move )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        v3d_s v = meval_s_eval_v3d( ev );
        map_s_move( o, &v );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_rotate )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        m3d_s rot = meval_s_eval_rot( ev );
        map_s_rotate( o, &rot );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_scale )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
        map_s_scale( o, meval_s_eval_f3( ev ) );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else
    {
        meval_s_err_fa( ev, "Map has no element of name #<sc_t>.", meval_s_get_name( ev, key ) );
    }

    return sr_fork( obj );
}

/**********************************************************************************************************************/
// array of objects

static sc_t arr_s_def =
"arr_s = bcore_inst"
"{"
    "aware_t _;"
    "bcore_arr_sr_s a;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( arr_s )
DEFINE_CREATE_SELF( arr_s, arr_s_def )

sz_t arr_s_get_size( const arr_s* o )
{
    return o->a.size;
}

sr_s* arr_s_get( arr_s* o, sz_t idx )
{
    if( idx < o->a.size ) return &o->a.data[ idx ];
    return NULL;
}

void arr_s_set( arr_s* o, sz_t idx, sr_s obj )
{
    if( idx < o->a.size )
    {
        sr_down( o->a.data[ idx ] );
        o->a.data[ idx ] = sr_clone( obj );
    }
}

void arr_s_push( arr_s* o, sr_s obj )
{
    bcore_arr_sr_s_push_sr( &o->a, obj );
}

void arr_s_cat( arr_s* o, const arr_s* arr )
{
    for( sz_t i = 0; i < arr->a.size; i++ )
    {
        arr_s_push( o, sr_cw( arr->a.data[ i ] ) );
    }
}

void arr_s_clear( arr_s* o )
{
    bcore_arr_sr_s_clear( &o->a );
}

void arr_s_move( arr_s* o, const v3d_s* vec )
{
    sz_t size = arr_s_get_size( o );
    for( sz_t i = 0; i < size; i++ )
    {
        sr_s* sr = arr_s_get( o, i );
        if( sr )
        {
            tp_t type = sr_s_type( sr );
            if( type == TYPEOF_arr_s )
            {
                arr_s_move( sr->o, vec );
            }
            else if( type == TYPEOF_map_s )
            {
                map_s_move( sr->o, vec );
            }
            else if( bcore_trait_is_of( sr_s_type( sr ), TYPEOF_spect_obj ) )
            {
                obj_move( sr->o, vec );
            }
        }
    }
}

void arr_s_rotate( arr_s* o, const m3d_s* mat )
{
    sz_t size = arr_s_get_size( o );
    for( sz_t i = 0; i < size; i++ )
    {
        sr_s* sr = arr_s_get( o, i );
        if( sr )
        {
            tp_t type = sr_s_type( sr );
            if( type == TYPEOF_arr_s )
            {
                arr_s_rotate( sr->o, mat );
            }
            else if( type == TYPEOF_map_s )
            {
                map_s_rotate( sr->o, mat );
            }
            else if( bcore_trait_is_of( sr_s_type( sr->o ), TYPEOF_spect_obj ) )
            {
                obj_rotate( sr->o, mat );
            }
        }
    }
}

void arr_s_scale( arr_s* o, f3_t fac )
{
    sz_t size = arr_s_get_size( o );
    for( sz_t i = 0; i < size; i++ )
    {
        sr_s* sr = arr_s_get( o, i );
        if( sr )
        {
            tp_t type = sr_s_type( sr );
            if( type == TYPEOF_arr_s )
            {
                arr_s_scale( sr->o, fac );
            }
            else if( type == TYPEOF_map_s )
            {
                map_s_scale( sr->o, fac );
            }
            else if( bcore_trait_is_of( sr_s_type( sr->o ), TYPEOF_spect_obj ) )
            {
                obj_scale( sr->o, fac );
            }
        }
    }
}

sr_s arr_s_meval_key( sr_s* sr_o, meval_s* ev, tp_t key )
{
    if( !sr_o ) return sr_null();
    assert( sr_s_type( sr_o ) == TYPEOF_arr_s );
    arr_s* o = sr_o->o;

    sr_s obj = sr_null();

    if( key == TYPEOF_push )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj = sr_clone( meval_s_eval( ev, sr_null() ) );
        arr_s_push( o, sr_s_fork( &obj ) );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_move )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        v3d_s v = meval_s_eval_v3d( ev );
        arr_s_move( o, &v );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_rotate )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        m3d_s rot = meval_s_eval_rot( ev );
        arr_s_rotate( o, &rot );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_scale )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        arr_s_scale( o, meval_s_eval_f3( ev ) );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == typeof( "size" ) )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj = sr_s3( arr_s_get_size( o ) );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_clear )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        arr_s_clear( o );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else
    {
        meval_s_err_fa( ev, "Map has no element of name #<sc_t>.", meval_s_get_name( ev, key ) );
    }

    return sr_fork( obj );
}

/**********************************************************************************************************************/

vd_t container_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "container" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        bcore_flect_define_creator( typeof( "map_s" ), map_s_create_self );
        bcore_flect_define_creator( typeof( "arr_s" ), arr_s_create_self );
    }

    return NULL;
}

/**********************************************************************************************************************/
