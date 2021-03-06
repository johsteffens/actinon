/** Compound of objects */

/** Copyright 2018 Johannes Bernhard Steffens
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

#include "bcore_spect_inst.h"
#include "bcore_life.h"
#include "bcore_spect.h"
#include "bcore_spect_array.h"
#include "bcore_trait.h"

#include "compound.h"
#include "container.h"

/**********************************************************************************************************************/
/// trans_data_s // ray transition data

BCORE_DEFINE_FUNCTIONS_OBJ_FLAT( trans_data_s )
BCORE_DEFINE_CREATE_SELF( trans_data_s, "trans_data_s = bcore_inst { v3d_s exit_nor; private vc_t exit_obj; private vc_t enter_obj; }" )

/**********************************************************************************************************************/
/// compound_s

typedef struct compound_s
{
    aware_t _;
    envelope_s* envelope;
    union
    {
        bcore_array_dyn_link_aware_s arr;
        struct
        {
            vd_t* data;
            uz_t  size;
            uz_t  space;
        };
    };
} compound_s;

static sc_t compound_s_def =
"compound_s = bcore_inst"
"{"
    "aware_t _;"
    "envelope_s => envelope;"
    "aware => [] object_arr;"
"}";

BCORE_DEFINE_FUNCTIONS_OBJ_INST( compound_s )

uz_t compound_s_get_size( const compound_s* o )
{
    return o ? o->size : 0;
}

void compound_s_set_envelope( compound_s* o, const envelope_s* envelope )
{
    if( o->envelope ) envelope_s_discard( o->envelope );
    o->envelope = envelope_s_clone( envelope );
}

void compound_s_set_auto_envelope( compound_s* o )
{
    if( o->envelope )
    {
        envelope_s_discard( o->envelope );
        o->envelope = NULL;
    }
    for( uz_t i = 0; i < o->size; i++ )
    {
        envelope_s env = envelope_create( v3d_s_zero(), 0 );
        vd_t obj = o->data[ i ];
        tp_t type = *( aware_t* )obj;
        if( type == TYPEOF_compound_s )
        {
            compound_s* cmp = obj;
            if( !cmp->envelope ) compound_s_set_auto_envelope( cmp );
            env = *cmp->envelope;
        }
        else if( bcore_trait_is_of( type, TYPEOF_spect_obj ) )
        {
            obj_hdr_s* hdr = obj;
            if( !hdr->prp.envelope ) obj_set_auto_envelope( obj );
            env = *hdr->prp.envelope;
        }

        if( o->envelope )
        {
            *o->envelope = envelope_of_pair( o->envelope, &env );
        }
        else
        {
            o->envelope = envelope_s_clone( &env );
        }
    }
}

const aware_t* compound_s_get_object( const compound_s* o, uz_t index )
{
    return o ? o->data[ index ] : NULL;
}

void compound_s_clear( compound_s* o )
{
    bcore_array_a_set_size( (bcore_array*)o, 0 );
}

vd_t compound_s_push_type( compound_s* o, tp_t type )
{
    if( type == TYPEOF_compound_s )
    {
        sr_s sr = sr_create( type );
        bcore_array_a_push( (bcore_array*)o, sr );
        return sr.o;
    }
    else if( bcore_trait_is_of( type, TYPEOF_spect_obj ) )
    {
        sr_s sr = sr_create( type );
        bcore_array_a_push( (bcore_array*)o, sr );
        return sr.o;
    }
    else
    {
        bcore_err_fa( "Cannot push '#<sc_t>' to compound_s.", ifnameof( type ) );
    }
    return NULL;
}

void compound_s_push_q( compound_s* o, const sr_s* object )
{
    if( !object ) return;
    tp_t type = sr_s_type( object );
    if( bcore_trait_is_of( type, TYPEOF_spect_obj ) )
    {
        vd_t dst = compound_s_push_type( o, type );
        bcore_inst_t_copy( type, dst, object->o );
        obj_hdr_s* hdr = dst;
        if( o->envelope )
        {
            if( hdr->prp.envelope )
            {
                *o->envelope = envelope_of_pair( o->envelope, hdr->prp.envelope );
            }
            else
            {
                envelope_s_discard( o->envelope );
                o->envelope = NULL;
            }
        }
        else if( o->size == 1 )
        {
            o->envelope = envelope_s_clone( hdr->prp.envelope );
        }
    }
    else if( type == TYPEOF_compound_s )
    {
        const compound_s* compound = ( const compound_s* )object->o;

        if( compound->envelope )
        {
            vd_t dst = compound_s_push_type( o, TYPEOF_compound_s );
            bcore_inst_t_copy( TYPEOF_compound_s, dst, compound );
        }
        else
        {
            for( uz_t i = 0; i < compound->size; i++ )
            {
                compound_s_push( o, sr_awc( compound->data[ i ] ) );
            }
        }
    }
    else if( type == TYPEOF_map_s )
    {
        map_s* map = object->o;
        uz_t size = bcore_hmap_tp_sr_s_size( &map->m );
        for( uz_t i = 0; i < size; i++ )
        {
            if( bcore_hmap_tp_sr_s_idx_key( &map->m, i ) )
            {
                compound_s_push_q( o, bcore_hmap_tp_sr_s_idx_val( &map->m, i ) );
            }
        }
    }
    else if( type == TYPEOF_arr_s )
    {
        const arr_s* arr = object->o;
        for( uz_t i = 0; i < arr->a.size; i++ )
        {
            compound_s_push_q( o, &arr->a.data[ i ] );
        }
    }
    else
    {
        bcore_err_fa( "Cannot push object #<sc_t> to compound_s.", ifnameof( type ) );
    }
}

void compound_s_push( compound_s* o, sr_s object )
{
    compound_s_push_q( o, &object );
    sr_down( object );
}

f3_t compound_s_ray_hit( const compound_s* o, const ray_s* ray, v3d_s* p_nor, vc_t* hit_obj )
{
    if( o->envelope && !envelope_s_ray_hits( o->envelope, ray ) ) return f3_inf;
    v3d_s nor;
    f3_t min_a = f3_inf;
    for( uz_t i = 0; i < o->size; i++ )
    {
        aware_t* element = o->data[ i ];
        vc_t hit_obj_l = NULL;
        f3_t a = f3_inf;
        if( *element == TYPEOF_compound_s )
        {
            compound_s* cmp = ( compound_s* )element;
            a = compound_s_ray_hit( cmp, ray, &nor, &hit_obj_l );
        }
        else
        {
            hit_obj_l = ( obj_hdr_s* )element;
            a = obj_ray_hit( hit_obj_l, ray, &nor );
        }

        if( a < min_a )
        {
            min_a = a;
            if( p_nor ) *p_nor = nor;
            if( hit_obj ) *hit_obj = hit_obj_l;
        }
    }
    return min_a;
}

f3_t compound_s_ray_trans_hit( const compound_s* o, const ray_s* ray, trans_data_s* trans )
{
    if( o->envelope && !envelope_s_ray_hits( o->envelope, ray ) ) return f3_inf;
    v3d_s nor;
    f3_t min_a = f3_inf;
    for( uz_t i = 0; i < o->size; i++ )
    {
        aware_t* element = o->data[ i ];
        obj_hdr_s* hit_obj = NULL;
        f3_t a = f3_inf;
        if( *element == TYPEOF_compound_s )
        {
            a = compound_s_ray_hit( ( compound_s* )element, ray, &nor, ( vc_t* )&hit_obj );
        }
        else
        {
            hit_obj = ( obj_hdr_s* )element;
            a = obj_ray_hit( hit_obj, ray, &nor );
        }

        if( a < f3_inf )
        {
            if( a < min_a - f3_eps )
            {
                min_a = a;
                if( v3d_s_mlv( nor, ray->d ) > 0 )
                {
                    trans->exit_nor = nor;
                    trans->exit_obj = hit_obj;
                    trans->enter_obj = NULL;
                }
                else
                {
                    trans->exit_nor = v3d_s_neg( nor );
                    trans->exit_obj = NULL;
                    trans->enter_obj = hit_obj;
                }
            }
            else if( f3_abs( a - min_a ) < f3_eps )
            {
                min_a = a < min_a ? a : min_a;
                if( v3d_s_mlv( nor, ray->d ) > 0 )
                {
                    trans->exit_obj = hit_obj;
                }
                else
                {
                    trans->enter_obj = hit_obj;
                }
            }
        }
    }
    return min_a;
}

uz_t compound_s_side_count( const compound_s* o, v3d_s pos, s2_t side )
{
    uz_t count = 0;
    for( uz_t i = 0; i < o->size; i++ )
    {
        count += ( obj_side( o->data[ i ], pos ) == side );
    }
    return count;
}

static bcore_self_s* compound_s_create_self( void )
{
    bcore_self_s* self = BCORE_SELF_S_BUILD_PARSE_SC( compound_s_def, compound_s );
    return self;
}

void compound_s_move( compound_s* o, const v3d_s* vec )
{
    if( o->envelope ) envelope_s_move( o->envelope, vec );
    for( uz_t i = 0; i < o->size; i++ )
    {
        vd_t obj = o->data[ i ];
        if( obj )
        {
            tp_t type = *( aware_t* )obj;
            if( type == TYPEOF_compound_s )
            {
                compound_s_move( obj, vec );
            }
            else if( bcore_trait_is_of( type, TYPEOF_spect_obj ) )
            {
                obj_move( obj, vec );
            }
        }
    }
}

void compound_s_rotate( compound_s* o, const m3d_s* mat )
{
    if( o->envelope ) envelope_s_rotate( o->envelope, mat );
    for( uz_t i = 0; i < o->size; i++ )
    {
        vd_t obj = o->data[ i ];
        if( obj )
        {
            tp_t type = *( aware_t* )obj;
            if( type == TYPEOF_compound_s )
            {
                compound_s_rotate( obj, mat );
            }
            else if( bcore_trait_is_of( type, TYPEOF_spect_obj ) )
            {
                obj_rotate( obj, mat );
            }
        }
    }
}

void compound_s_scale( compound_s* o, f3_t fac )
{
    if( o->envelope ) envelope_s_scale( o->envelope, fac );
    for( uz_t i = 0; i < o->size; i++ )
    {
        vd_t obj = o->data[ i ];
        if( obj )
        {
            tp_t type = *( aware_t* )obj;
            if( type == TYPEOF_compound_s )
            {
                compound_s_scale( obj, fac );
            }
            else if( bcore_trait_is_of( type, TYPEOF_spect_obj ) )
            {
                obj_scale( obj, fac );
            }
        }
    }
}

sr_s compound_s_meval_key( sr_s* sr_o, meval_s* ev, tp_t key )
{
    if( !sr_o ) return sr_null();
    assert( sr_s_type( sr_o ) == TYPEOF_compound_s );
    compound_s* o = sr_o->o;

    sr_s ret = sr_null();

    if( key == TYPEOF_push )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        sr_s obj = meval_s_eval( ev, sr_null() );
        tp_t type = sr_s_type( &obj );
        if( type != TYPEOF_compound_s && !bcore_trait_is_of( type, TYPEOF_spect_obj ) )
        {
            meval_s_err_fa( ev, "Cannot push '#<sc_t>' to compound_s.", ifnameof( type ) );
        }
        compound_s_push( o, obj );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_move )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        v3d_s v = meval_s_eval_v3d( ev );
        compound_s_move( o, &v );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_rotate )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        m3d_s rot = meval_s_eval_rot( ev );
        compound_s_rotate( o, &rot );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_scale )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        compound_s_scale( o, meval_s_eval_f3( ev ) );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == typeof( "set_envelope" ) )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        sr_s v = meval_s_eval( ev, sr_null() );
        if( sr_s_type( &v ) == TYPEOF_envelope_s )
        {
            compound_s_set_envelope( sr_o->o, ( const envelope_s* )v.o );
        }
        else if( sr_s_type( &v ) == TYPEOF_obj_sphere_s )
        {
            envelope_s env;
            env.pos = ( ( obj_hdr_s* )v.o )->prp.pos;
            env.radius = obj_sphere_s_get_radius( v.o );
            compound_s_set_envelope( sr_o->o, &env );
        }
        else
        {
            meval_s_err_fa( ev, "Object '#<sc_t>' cannot be used as envelope (use a sphere).", ifnameof( sr_s_type( &v ) ) );
        }

        sr_down( v );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == typeof( "set_auto_envelope" ) )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
        compound_s_set_auto_envelope( sr_o->o );
    }
    else
    {
        meval_s_err_fa( ev, "Compound has no element of name #<sc_t>.", meval_s_get_name( ev, key ) );
    }

    return sr_fork( ret );
}

/**********************************************************************************************************************/

vd_t compound_signal_handler( const bcore_signal_s* o )
{
    switch( bcore_signal_s_handle_type( o, typeof( "compound" ) ) )
    {
        case TYPEOF_init1:
        {
            BCORE_REGISTER_OBJECT( trans_data_s );
            BCORE_REGISTER_OBJECT( compound_s );
        }
        break;

        default: break;
    }
    return NULL;
}

/**********************************************************************************************************************/

