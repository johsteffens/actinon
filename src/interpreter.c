/** Interpreter for Scene and Object Construction
 *  Author: Johannes Bernhard Steffens
 *
 *  Copyright (c) 2017 Johannes Bernhard Steffens. All rights reserved.
 */

#include "bcore_tp.h"
#include "bcore_name_manager.h"
#include "bcore_spect_source.h"
#include "bcore_spect_inst.h"
#include "bcore_arr.h"
#include "bcore_quicktypes.h"
#include "bcore_life.h"
#include "bcore_txt_ml.h"
#include "bcore_spect_interpreter.h"
#include "bcore_spect_via.h"

#include "vectors.h"
#include "objects.h"
#include "interpreter.h"

/**********************************************************************************************************************/

/// specific handlers
static sr_s eval_to_stdout( sr_s* o, interpreter_s* ip )
{
    bcore_source_q_parse_fa( &ip->source, " ( ) " );
    bcore_txt_ml_to_stdout( sr_cw( *o ) );
    return sr_null();
}

/**********************************************************************************************************************/

static sc_t interpreter_s_def =
"interpreter_s = bcore_interpreter"
"{"
    "aware_t _;"
    "sr_s source;"
    "bcore_name_map_s names;"
    "bcore_hmap_tp_sr_s variables;"
    "bcore_hmap_tp_sr_s constants;"
    "bcore_hmap_tpfp_s  f_eval;"
    "bcore_hmap_tpfp_s  o_eval;"
    "private interpreter_s* parent;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( interpreter_s )

static sr_s* interpreter_s_get_obj( const interpreter_s* o, tp_t key )
{
    sr_s* p = bcore_hmap_tp_sr_s_get( &o->variables, key );
    if( !p )
    {
        p = bcore_hmap_tp_sr_s_get( &o->constants, key );
        if( p ) sr_s_set_const( p, true );
    }
    if( !p && o->parent )
    {
        p = interpreter_s_get_obj( o->parent, key );
    }
    return p;
}

static sr_s* interpreter_s_set_obj( interpreter_s* o, tp_t key, sr_s obj )
{
    return bcore_hmap_tp_sr_s_set( &o->variables, key, obj );
}

static void interpreter_s_set_name( interpreter_s* o, sc_t name )
{
    bcore_name_map_s_set( &o->names, bcore_name_sc( name ) );
}

/*
static sc_t interpreter_s_get_name( const interpreter_s* o, tp_t key )
{
    bcore_name_s* name = bcore_name_map_s_get( &o->names, key );
    return name ? name->name : NULL;
}
*/

static void interpreter_s_declare_eval_handlers( interpreter_s* o )
{
    bcore_hmap_tpfp_s_set( &o->f_eval, typeof( "to_stdout" ), ( fp_t )eval_to_stdout );
}

static sr_s interpreter_s_mul( const interpreter_s* o, sr_s v1, sr_s v2 )
{
    tp_t t1 = sr_s_type( &v1 );
    tp_t t2 = sr_s_type( &v2 );
    sr_s ret = sr_null();
    if( t1 == TYPEOF_f3_t )
    {
        if( t2 == TYPEOF_f3_t )
        {
            ret = sr_f3( *( f3_t* )v1.o * *( f3_t* )v2.o );
        }
        else if( t2 == TYPEOF_v3d_s )
        {
            ret = sr_create( TYPEOF_v3d_s );
            *( v3d_s* )ret.o = v3d_s_mlf(  *( v3d_s* )v2.o, *( f3_t* )v1.o );
        }
    }
    else if( t1 == TYPEOF_v3d_s )
    {
        if( t2 == TYPEOF_f3_t  )
        {
            ret = sr_create( TYPEOF_v3d_s );
            *( v3d_s* )ret.o = v3d_s_mlf(  *( v3d_s* )v1.o, *( f3_t* )v2.o );
        }
        else if( t2 == TYPEOF_v3d_s )
        {
            ret = sr_f3( v3d_s_mlv( *( v3d_s* )v1.o, *( v3d_s* )v2.o ) );
        }
    }
    if( !ret.o )
    {
        bcore_source_q_parse_err_fa( &o->source, "Cannot evaluate '#<sc_t>' * '#<sc_t>'\n", ifnameof( t1 ), ifnameof( t2 ) );
    }

    sr_down( v1 );
    sr_down( v2 );

    return ret;
}

static sr_s interpreter_s_add( const interpreter_s* o, sr_s v1, sr_s v2 )
{
    tp_t t1 = sr_s_type( &v1 );
    tp_t t2 = sr_s_type( &v2 );
    sr_s ret = sr_null();
    if( t1 == TYPEOF_f3_t && t2 == TYPEOF_f3_t )
    {
        ret = sr_f3( *( f3_t* )v1.o + *( f3_t* )v2.o );
    }
    else if( t1 == TYPEOF_v3d_s && t2 == TYPEOF_v3d_s )
    {
        ret = sr_create( TYPEOF_v3d_s );
        *( v3d_s* )ret.o = v3d_s_add( *( v3d_s* )v1.o, *( v3d_s* )v2.o );
    }

    if( !ret.o )
    {
        bcore_source_q_parse_err_fa( &o->source, "Cannot evaluate '#<sc_t>' + '#<sc_t>'\n", ifnameof( t1 ), ifnameof( t2 ) );
    }

    sr_down( v1 );
    sr_down( v2 );

    return ret;
}

static sr_s interpreter_s_inverse( const interpreter_s* o, sr_s v1 )
{
    tp_t t1 = sr_s_type( &v1 );
    sr_s ret = sr_null();
    if( t1 == TYPEOF_f3_t )
    {
        f3_t v = *( f3_t* )v1.o;
        ret = sr_f3( v != 0 ? 1.0 / v : f3_inf );
    }

    if( !ret.o )
    {
        bcore_source_q_parse_err_fa( &o->source, "Cannot invert '#<sc_t>'\n", ifnameof( t1 ) );
    }

    sr_down( v1 );

    return ret;
}

/// evaluates an expression
sr_s interpreter_s_eval( interpreter_s* o, sr_s front_obj )
{
    char op = 0;
    if( front_obj.o )
    {
        if( bcore_source_q_parse_bl_fa( &o->source, "#?([0]=='.'||[0]=='='||[0]=='+'||[0]=='-'||[0]=='*'||[0]=='/')" ) )
        {
            bcore_source_q_get_data( &o->source, &op, 1 );
            bcore_source_q_parse_fa( &o->source, " " );
        }
        else
        {
            return sr_fork( front_obj );
        }

        // operators to be immediately processed
        if( op == '=' )
        {
            if( sr_s_is_const( &front_obj ) ) bcore_source_q_parse_err_fa( &o->source, "Attempt to modify a const object.\n" );
            sr_s obj = interpreter_s_eval( o, sr_null() );
            if( !obj.o ) bcore_source_q_parse_err_fa( &o->source, "Assignment from empty object.\n" );
            bcore_inst_typed_copy_typed( sr_s_type( &front_obj ), front_obj.o, sr_s_type( &obj ), obj.o );
            sr_down( obj );
            return sr_fork( front_obj );
        }

    }

    // unary operators
    bcore_source_q_parse_fa( &o->source, "#-?'+' " );                 // identity
    bl_t negate = bcore_source_q_parse_bl_fa( &o->source, "#?'-' " ); // negation

    sr_s obj = sr_null();

    if( bcore_source_q_parse_bl_fa( &o->source, "#?([0]>='0'&&[0]<='9')" ) ) // number
    {
        f3_t num = 0;
        bcore_source_q_parse_fa( &o->source, "#<f3_t*> ", &num );
        obj = sr_f3( num );
    }
    else if( bcore_source_q_parse_bl_fa( &o->source, "#?(([0]>='A'&&[0]<='Z')||([0]>='a'&&[0]<='z')||([0]=='_'))" ) ) // name of variable, constant or function
    {
        st_s* name = st_s_create();
        bcore_source_q_parse_fa( &o->source, "#name ", name );
        tp_t key = typeof( name->sc );

        if( op == '.' ) // member to front_obj
        {
            if( bcore_hmap_tpfp_s_exists( &o->f_eval, key ) )
            {
                obj = ( ( fp_eval )*bcore_hmap_tpfp_s_get( &o->f_eval, key ) )( &front_obj, o );
            }
            else if( bcore_via_q_nexists( &front_obj, key ) )
            {
                obj = bcore_via_q_nget( &front_obj, key );
            }
            else
            {
                bcore_source_q_parse_err_fa( &o->source, "Object '#<sc_t>' has no element named '#<sc_t>'.", ifnameof( sr_s_type( &front_obj ) ), name->sc );
            }
            op = 0;
        }
        else if( bcore_flect_exists( key ) )
        {
            obj = sr_create( key );
        }
        else
        {
            sr_s* p_obj = interpreter_s_get_obj( o, key );
            if( bcore_source_q_parse_bl_fa( &o->source, "#=?'='" ) ) // assignment (not consumed here)
            {
                if( !p_obj )
                {
                    bcore_source_q_parse_fa( &o->source, "= " ); // consume assignment
                    interpreter_s_set_name( o, name->sc );
                    obj = sr_s_fork( interpreter_s_set_obj( o, key, interpreter_s_eval( o, sr_null() ) ) );
                }
                else
                {
                    if( sr_s_is_const( &obj ) ) bcore_source_q_parse_err_fa( &o->source, "'#<st_s*>' is a constant.", name );
                    obj = interpreter_s_eval( o, sr_s_fork( p_obj ) ); // consume assignment in nested cycle
                }
            }
            else if( !p_obj )
            {
                bcore_source_q_parse_err_fa( &o->source, "Unknown name '#<st_s*>'\n", name );
            }
            else
            {
                obj = sr_s_fork( p_obj );
            }
        }
        st_s_discard( name );
    }
    else if( bcore_source_q_parse_bl_fa( &o->source, "#?'(' " ) ) // nested expression
    {
        obj = interpreter_s_eval( o, sr_null() );
        bcore_source_q_parse_fa( &o->source, ") " );
    }
    else if( bcore_source_q_parse_bl_fa( &o->source, "#?'<' " ) ) // vector
    {
        v3d_s vec;
        for( sz_t i = 0; i < 3; i++ )
        {
            sr_s num = interpreter_s_eval( o, sr_null() );
            if( sr_s_type( &num ) != TYPEOF_f3_t ) bcore_source_q_parse_err_fa( &o->source, "Vector component #<sz_t> does not evaluate to a number.\n", i + 1 );
            ( &vec.x )[ i ] = *( f3_t* )num.o;
            sr_down( num );
            if( i < 2 ) bcore_source_q_parse_fa( &o->source, ", " );
        }
        obj = sr_tsd( TYPEOF_v3d_s, v3d_s_clone( &vec ) );
        bcore_source_q_parse_fa( &o->source, "> " );
    }

    if( obj.o )
    {
        if( negate ) obj = interpreter_s_mul( o, sr_f3( -1 ), obj );
        if( op )
        {
            switch( op )
            {
                case '*': return interpreter_s_eval( o, interpreter_s_mul( o, front_obj, obj ) );
                case '/': return interpreter_s_eval( o, interpreter_s_mul( o, front_obj, interpreter_s_inverse( o, obj ) ) );
                case '+': return sr_fork( interpreter_s_add(  o, front_obj, interpreter_s_eval( o, obj ) ) );
                case '-': return sr_fork( interpreter_s_add(  o, front_obj, interpreter_s_eval( o, interpreter_s_mul( o, sr_f3( -1 ), obj ) ) ) );
                default : bcore_source_q_parse_err_fa( &o->source, "Invalid operator '#<char>'.\n", op );
            }
        }
        else
        {
            sr_down( front_obj );
            return interpreter_s_eval( o, obj );
        }
    }
    else
    {
        if( op ) bcore_source_q_parse_err_fa( &o->source, "Expression does not yield an operand for operation '#<char>'.\n", op );
    }
    sr_down( front_obj );
    return sr_fork( obj );
}

sr_s interpreter_s_interpret( const interpreter_s* const_o, sr_s source )
{
    bcore_life_s* l = bcore_life_s_create();
    sr_s src = sr_cp( bcore_life_s_push_sr( l, source ), TYPEOF_bcore_source_s );
    interpreter_s* o = bcore_life_s_push_aware( l, interpreter_s_clone( const_o ) );
    interpreter_s_declare_eval_handlers( o );
    o->source = src;

    sr_s return_obj = sr_null();

    while( !bcore_source_q_eos( &src ) )
    {
        sr_s obj = interpreter_s_eval( o, sr_null() );
        bcore_source_parse_fa( src, "; " );
        sr_down( return_obj );
        return_obj = obj;
    }
    bcore_life_s_discard( l );
    return return_obj;
}

static bcore_flect_self_s* interpreter_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( interpreter_s_def, sizeof( interpreter_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )interpreter_s_interpret, "bcore_fp_interpret", "interpret" );
    return self;
}

/**********************************************************************************************************************/

st_s* interpreter_selftest( void )
{
    st_s* log = st_s_create();
    sc_t program =
    "<interpreter_s></> "
    "b = ( a = <1,2,3> ) * ( 2 - 5 ) + a;"
    "v = b * 10;"
    "v.to_stdout();"
    "v.y = 2 + 3;"
    "v.to_stdout();"
    ";";

    sr_s out = bcore_interpret_auto( sr_asd( st_s_create_weak_sc( program ) ) );
    bcore_txt_ml_to_string( out, log );

    return log;
}

/**********************************************************************************************************************/

vd_t interpreter_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "interpreter" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        bcore_flect_define_creator( typeof( "interpreter_s" ), interpreter_s_create_self );
    }
    else if( signal == typeof( "selftest" ) )
    {
        return interpreter_selftest();
    }

    return NULL;
}

/**********************************************************************************************************************/
