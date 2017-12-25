/** Interpreter for Scene and Object Construction */

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
#include "bcore_spect_interpreter.h"
#include "bcore_spect_via.h"
#include "bcore_trait.h"
#include "bcore_sources.h"

#include "vectors.h"
#include "objects.h"
#include "interpreter.h"
#include "container.h"
#include "scene.h"
#include "closures.h"

/**********************************************************************************************************************/
// code_s: metacode elements

sc_t code_symbol( code_s o )
{
    switch( o )
    {
        case CL_COMMA:     return ","; break;
        case CL_COLON:     return ":"; break;
        case CL_SEMICOLON: return ";"; break;
        case CL_ROUND_BRACKET_OPEN:   return "("; break;
        case CL_ROUND_BRACKET_CLOSE:  return ")"; break;
        case CL_SQUARE_BRACKET_OPEN:  return "["; break;
        case CL_SQUARE_BRACKET_CLOSE: return "]"; break;
        case OP_DOT:           return "."; break;
        case OP_QUERY:         return "?"; break;
        case OP_MUL:           return "*"; break;
        case OP_MUL_ASSIGN:    return "*="; break;
        case OP_DIV:           return "/"; break;
        case OP_DIV_ASSIGN:    return "/="; break;
        case OP_ADD:           return "+"; break;
        case OP_ADD_ASSIGN:    return "+="; break;
        case OP_SUB:           return "-"; break;
        case OP_SUB_ASSIGN:    return "-="; break;
        case OP_ASSIGN:        return "="; break;
        case OP_EQUAL:         return "=="; break;
        case OP_SMALLER:       return "<"; break;
        case OP_UNEQUAL:       return "!="; break;
        case OP_SMALLER_EQUAL: return "<="; break;
        case OP_LARGER:        return ">"; break;
        case OP_LARGER_EQUAL:  return ">="; break;
        case OP_NOT:           return "NOT"; break;
        case OP_AND:           return "AND"; break;
        case OP_OR:            return "OR"; break;
        case OP_XOR:           return "XOR"; break;
        default:               return ""; break;
    }
    return "";
}

/**********************************************************************************************************************/
// mcode_s: metacode



#define TYPEOF_mcode_s typeof( "mcode_s" )
typedef struct mcode_s
{
    aware_t _;
    st_s              file;
    bcore_arr_sr_s    data;
    bcore_arr_tp_s    code;
    bcore_name_map_s  names;
    bcore_arr_sz_s    src_map; // map to source code: alternating code and source index
} mcode_s;

static sc_t mcode_s_def =
"mcode_s = bcore_inst"
"{"
    "aware_t _;"
    "st_s             file;"
    "bcore_arr_sr_s   data;"
    "bcore_arr_tp_s   code;"
    "bcore_name_map_s names;"
    "bcore_arr_sz_s   src_map;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( mcode_s )
DEFINE_CREATE_SELF( mcode_s, mcode_s_def )

static void mcode_s_reset( mcode_s* o )
{
    st_s_clear( &o->file );
    bcore_arr_sr_s_clear( &o->data );
    bcore_arr_tp_s_clear( &o->code );
    bcore_name_map_s_clear( &o->names );
}

static void mcode_s_err_fv( const mcode_s* o, sz_t* index, sc_t format, va_list args )
{
    if( o->file.size > 0 )
    {
        s3_t src_idx = 0;
        for( sz_t i = 0; i < o->src_map.size; i += 2 )
        {
            if( o->src_map.data[ i ] > *index ) break;
            src_idx = o->src_map.data[ i + 1 ];
        }
        bcore_source_chain_s* chain = bcore_source_open_file( o->file.sc );
        bcore_source_aware_set_index( chain, src_idx );
        bcore_source_aware_parse_err_fv( chain, format, args );
        bcore_source_chain_s_discard( chain );
    }
    else
    {
        bcore_err_fv( format, args );
    }
}

void mcode_s_push_code( mcode_s* o, tp_t code )
{
    bcore_arr_tp_s_push( &o->code, code );
}

void mcode_s_push_data( mcode_s* o, sr_s v )
{
    mcode_s_push_code( o, CL_DATA );
    mcode_s_push_code( o, o->data.size );
    bcore_arr_sr_s_push_sr( &o->data, v );
}

void mcode_s_push_name( mcode_s* o, sc_t name )
{
    bcore_name_s n = bcore_name_sc( name );
    mcode_s_push_code( o, CL_NAME );
    mcode_s_push_code( o, n.key );
    bcore_name_map_s_set( &o->names, n );
}

void mcode_s_push_src_index( mcode_s* o, sr_s* src )
{
    bcore_arr_sz_s_push( &o->src_map, o->code.size );
    bcore_arr_sz_s_push( &o->src_map, bcore_source_q_get_index( src ) );
}

void mcode_s_parse( mcode_s* o, sr_s* src )
{
    bcore_life_s* l = bcore_life_s_create();
    mcode_s_reset( o );
    st_s_copy_sc( &o->file, bcore_source_q_get_file( src ) );
    bcore_source_q_parse_fa( src, " " );

    bcore_arr_sz_s* jmp_buf = bcore_life_s_push_aware( l, bcore_arr_sz_s_create() ); // jump address buffer

    while( !bcore_source_q_eos( src ) )
    {
        mcode_s_push_src_index( o, src );

        // number literal
        if( bcore_source_q_parse_bl_fa( src, "#?([0]>='0'&&[0]<='9')" ) )
        {
            u3_t vi = 0; // integer part
            f3_t vf = 0; // fraction part (behind decimal point)
            s3_t vx = 0; // exponent
            bcore_source_q_parse_fa( src, "#<u3_t*>", &vi );
            bl_t is_int = true;
            if( bcore_source_q_parse_bl_fa( src, "#?'.'" ) )
            {
                is_int = false;
                f3_t f = 0.1;
                while( bcore_source_q_parse_bl_fa( src, "#?([0]>='0'&&[0]<='9')" ) )
                {
                    char c = bcore_source_q_get_u0( src );
                    vf += f * ( c - '0' );
                    f *= 0.1;
                }
            }
            if( bcore_source_q_parse_bl_fa( src, "#?([0]=='e'||[0]=='E')" ) )
            {
                is_int = false;
                bcore_source_q_parse_fa( src, "#-<char*>" );
                bcore_source_q_parse_fa( src, "#<s3_t*>", &vx );
            }
            if( is_int )
            {
                mcode_s_push_data( o, sr_s3( vi ) );
            }
            else
            {
                f3_t v = vi + vf;
                v *= pow( 10.0, ( f3_t )vx );
                mcode_s_push_data( o, sr_f3( v ) );
            }
        }
        else if( bcore_source_q_parse_bl_fa( src, "#?'\"'" ) ) // string literal
        {
            st_s* s = st_s_create();
            while( !bcore_source_q_parse_bl_fa( src, "#?'\"'" ) )
            {
                if( bcore_source_q_eos( src ) ) bcore_source_q_parse_err_fa( src, "Stream ends in string literal" );
                char c = bcore_source_q_get_u0( src );
                if( c == '\\' )
                {
                    if     ( bcore_source_q_parse_bl_fa( src, "#?'\"'" ) ) st_s_push_char( s, '\"' );
                    else if( bcore_source_q_parse_bl_fa( src, "#?'n'"  ) ) st_s_push_char( s, '\n' );
                    else if( bcore_source_q_parse_bl_fa( src, "#?'r'"  ) ) st_s_push_char( s, '\r' );
                    else if( bcore_source_q_parse_bl_fa( src, "#?'t'"  ) ) st_s_push_char( s, '\t' );
                    else if( bcore_source_q_parse_bl_fa( src, "#?'0'"  ) ) st_s_push_char( s, '\0' );
                    else if( bcore_source_q_parse_bl_fa( src, "#?'\\'" ) ) st_s_push_char( s, '\\' );
                    else st_s_push_char( s, '\\' );
                }
                else
                {
                    st_s_push_char( s, c );
                }
            }
            mcode_s_push_data( o, sr_asd( s ) );
        }
        else if( bcore_source_q_parse_bl_fa( src, "#?(([0]>='A'&&[0]<='Z')||([0]>='a'&&[0]<='z')||([0]=='_'))" ) ) // name of variable, constant, type, function or flow-control
        {
            st_s* name = st_s_create();
            bcore_source_q_parse_fa( src, "#name ", name );
            tp_t key = typeof( name->sc );

            switch( key )
            {
                // predefined names
                case TYPEOF_true:  mcode_s_push_data( o, sr_bl( true  ) ); break;
                case TYPEOF_false: mcode_s_push_data( o, sr_bl( false ) ); break;
                case TYPEOF_AND:   mcode_s_push_code( o, OP_AND         ); break;
                case TYPEOF_OR:    mcode_s_push_code( o, OP_OR          ); break;
                case TYPEOF_XOR:   mcode_s_push_code( o, OP_XOR         ); break;
                case TYPEOF_NOT:   mcode_s_push_code( o, OP_NOT         ); break;
                case TYPEOF_if:
                {
                    mcode_s_push_code( o, FL_IF );
                    bcore_arr_sz_s_push( jmp_buf, o->code.size );
                    mcode_s_push_code( o, 0 ); // end of if-block
                }
                break;

                case TYPEOF_while:
                {
                    mcode_s_push_code( o, FL_WHILE );
                    bcore_arr_sz_s_push( jmp_buf, o->code.size );
                    mcode_s_push_code( o, 0 ); // end of while-block
                }
                break;

                case TYPEOF_else:
                {
                    if( jmp_buf->size == 0 ) bcore_source_q_parse_err_fa( src, "'else' without 'if'" );
                    sz_t idx = bcore_arr_sz_s_pop( jmp_buf );
                    o->code.data[ idx ] = o->code.size;
                    mcode_s_push_code( o, FL_ELSE );
                    bcore_arr_sz_s_push( jmp_buf, o->code.size );
                    mcode_s_push_code( o, 0 ); // end of else-block
                }
                break;

                default:
                {
                    if( bcore_flect_exists( key ) )
                    {
                        mcode_s_push_data( o, sr_create( key ) );
                    }
                    else
                    {
                        mcode_s_push_name( o, name->sc );
                    }
                }
                break;
            }
            st_s_discard( name );
        }
        // operators
        else if( bcore_source_q_parse_bl_fa( src, "#?([0]=='!'||[0]=='?'||[0]=='.'||[0]=='='||[0]=='+'||[0]=='-'||[0]=='*'||[0]=='/'||[0]=='>'||[0]=='<'||[0]=='&'||[0]=='|')" ) ) // operator
        {
            char c = bcore_source_q_get_u0( src );
            switch( c )
            {
                case '!': mcode_s_push_code( o, OP_NOT   ); break;
                case '?': mcode_s_push_code( o, OP_QUERY ); break;
                case '.': mcode_s_push_code( o, OP_DOT   ); break;
                case '=': mcode_s_push_code( o, bcore_source_q_parse_bl_fa( src, "#?'='" ) ? OP_EQUAL         : OP_ASSIGN ); break;
                case '+': mcode_s_push_code( o, bcore_source_q_parse_bl_fa( src, "#?'='" ) ? OP_ADD_ASSIGN    : OP_ADD ); break;
                case '-': mcode_s_push_code( o, bcore_source_q_parse_bl_fa( src, "#?'='" ) ? OP_SUB_ASSIGN    : OP_SUB ); break;
                case '*': mcode_s_push_code( o, bcore_source_q_parse_bl_fa( src, "#?'='" ) ? OP_MUL_ASSIGN    : OP_MUL ); break;
                case '/': mcode_s_push_code( o, bcore_source_q_parse_bl_fa( src, "#?'='" ) ? OP_DIV_ASSIGN    : OP_DIV ); break;
                case '<': mcode_s_push_code( o, bcore_source_q_parse_bl_fa( src, "#?'='" ) ? OP_SMALLER_EQUAL :
                                                bcore_source_q_parse_bl_fa( src, "#?'>'" ) ? OP_UNEQUAL       : OP_SMALLER ); break;
                case '>': mcode_s_push_code( o, bcore_source_q_parse_bl_fa( src, "#?'='" ) ? OP_LARGER_EQUAL  : OP_LARGER ); break;
                case '&': mcode_s_push_code( o , OP_AND ); break;
                case '|': mcode_s_push_code( o , OP_OR  ); break;
                case '^': mcode_s_push_code( o , OP_XOR ); break;
                default : break;
            }
        }
        // controls
        else if( bcore_source_q_parse_bl_fa( src, "#?([0]==':'||[0]==';'||[0]==','||[0]=='('||[0]==')'||[0]=='['||[0]==']')" ) ) // controls
        {
            char c = bcore_source_q_get_u0( src );
            switch( c )
            {
                case ':': mcode_s_push_code( o, CL_COLON ); break;

                case ';':
                {
                    if( jmp_buf->size > 0 )
                    {
                        sz_t idx = bcore_arr_sz_s_pop( jmp_buf );
                        o->code.data[ idx ] = o->code.size;
                    }
                    if( jmp_buf->size > 0 ) bcore_source_q_parse_err_fa( src, "Trailing jump address at end of statement." );
                    mcode_s_push_code( o, CL_SEMICOLON );
                }
                break;

                case ',': mcode_s_push_code( o, CL_COMMA                ); break;
                case '(': mcode_s_push_code( o, CL_ROUND_BRACKET_OPEN   ); break;
                case ')': mcode_s_push_code( o, CL_ROUND_BRACKET_CLOSE  ); break;
                case '[': mcode_s_push_code( o, CL_SQUARE_BRACKET_OPEN  ); break;
                case ']': mcode_s_push_code( o, CL_SQUARE_BRACKET_CLOSE ); break;
                default : break;
            }
        }
        else if( bcore_source_q_parse_bl_fa( src, "#?'{' " ) ) // recurse into block
        {
            mcode_s* code = mcode_s_create();
            mcode_s_parse( code, src );
            mcode_s_push_data( o, sr_asd( code ) );
            bcore_source_q_parse_fa( src, "} " );
        }
        else if( bcore_source_q_parse_bl_fa( src, "#=?'}'" ) ) // end of block (no consumption)
        {
            break;
        }
        else if( bcore_source_q_parse_bl_fa( src, "#?'#include '" ) ) // include file (c-style syntax) (interpreted as block)
        {
            st_s* file = st_s_create();
            bcore_source_q_parse_fa( src, "#string ", file );
            if( file->size == 0 ) bcore_source_q_parse_err_fa( src, "File name expected." );
            if( file->sc[ 0 ] != '/' ) // make path relative to current file path
            {
                st_s* cur_file = st_s_create_sc( bcore_source_q_get_file( src ) );
                sz_t idx = st_s_find_char( cur_file, cur_file->size, 0, '/' );
                if( idx < cur_file->size )
                {
                    cur_file->data[ idx ] = 0;
                    st_s* new_file = st_s_create_fa( "#<sc_t>/#<sc_t>", cur_file->sc, file->sc );
                    st_s_discard( file );
                    file = new_file;
                }
            }

            mcode_s* code = mcode_s_create();

            sr_s chain = sr_asd( bcore_source_chain_s_create() );
            bcore_source_chain_s_push_d( chain.o, bcore_source_file_s_create_name( file->sc ) );
            bcore_source_chain_s_push_d( chain.o, bcore_inst_typed_create( typeof( "bcore_source_string_s" ) ) );
            mcode_s_parse( code, &chain );
            sr_down( chain );
            st_s_discard( file );

            mcode_s_push_code( o, CL_DATA );
            mcode_s_push_code( o, o->data.size );

            bcore_arr_sr_s_push_sr( &o->data, sr_asd( code ) );
        }
        else
        {
            bcore_source_q_parse_err_fa( src, "Syntax error." );
        }

        bcore_source_q_parse_fa( src, " " );
    }
    bcore_life_s_discard( l );
}

static tp_t mcode_s_peek_code( const mcode_s* o, const sz_t* index )
{
    if( *index < o->code.size )
    {
        return o->code.data[ *index ];
    }
    return 0;
}

static tp_t mcode_s_get_code( const mcode_s* o, sz_t* index )
{
    tp_t code = ( *index < o->code.size ) ? o->code.data[ ( *index )++ ] : 0;
    return code;
}

/// if matching: consumes code and returns true; else returns false;
static bl_t mcode_s_try_code( const mcode_s* o, sz_t* index, tp_t code )
{
    if( mcode_s_peek_code( o, index ) == code )
    {
        mcode_s_get_code( o, index );
        return true;
    }
    return false;
}

/// if matching: consumes code and returns true; else returns false;
bl_t mcode_s_try_name( const mcode_s* o, sz_t* index, tp_t key )
{
    if
    (
        *index < o->code.size - 1 &&
        o->code.data[ *index     ] == CL_NAME &&
        o->code.data[ *index + 1 ] == key
    )
    {
        ( *index ) += 2;
        return true;
    }
    return false;
}

static bl_t mcode_s_end_code( const mcode_s* o, sz_t* index )
{
    return ( *index == o->code.size ) ? true : false;
}

/**********************************************************************************************************************/

typedef struct meval_s
{
    aware_t _;
    const mcode_s* mcode; // not owned only referenced
    bclos_frame_s* frame; // not owned only referenced
    sz_t           index; // code index
} meval_s;

static sc_t meval_s_def =
"meval_s = bcore_inst"
"{"
    "aware_t _;"
    "private mcode_s*       mcode;"
    "private bclos_frame_s* frame;"
    "sz_t                   index;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( meval_s )

static bcore_flect_self_s* meval_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( meval_s_def, sizeof( meval_s ) );
    return self;
}

void meval_s_err_fv( meval_s* o, sc_t format, va_list args )
{
    mcode_s_err_fv( o->mcode, &o->index, format, args );
}

void meval_s_err_fa( meval_s* o, sc_t format, ... )
{
    va_list args;
    va_start( args, format );
    meval_s_err_fv( o, format, args );
    va_end( args );
}

sr_s* meval_s_get_obj( const meval_s* o, tp_t key )
{
    return bclos_frame_s_get( o->frame, key );
}

sr_s* meval_s_set_obj( meval_s* o, tp_t key, sr_s obj )
{
    return bclos_frame_s_set( o->frame, key, obj );
}

sc_t meval_s_get_name( const meval_s* o, tp_t key )
{
    bcore_name_s* name = bcore_name_map_s_get( &o->mcode->names, key );
    return name ? name->name : NULL;
}

static sr_s meval_s_mul( meval_s* o, sr_s v1, sr_s v2 )
{
    tp_t t1 = sr_s_type( &v1 );
    tp_t t2 = sr_s_type( &v2 );
    sr_s r = sr_null();

    switch( t1 )
    {
        case TYPEOF_s3_t:
        switch( t2 )
        {
            case TYPEOF_s3_t:  r = sr_s3( *( s3_t* )v1.o * *( s3_t* )v2.o ); break;
            case TYPEOF_f3_t:  r = sr_f3( *( s3_t* )v1.o * *( f3_t* )v2.o ); break;
            case TYPEOF_bl_t:  r = sr_s3( *( s3_t* )v1.o * *( bl_t* )v2.o ); break;
            case TYPEOF_v3d_s: r = sr_create( TYPEOF_v3d_s ); *( v3d_s* )r.o = v3d_s_mlf( *( v3d_s* )v2.o, *( s3_t* )v1.o ); break;
        }
        break;

        case TYPEOF_f3_t:
        switch( t2 )
        {
            case TYPEOF_s3_t:  r = sr_f3( *( f3_t* )v1.o * *( s3_t* )v2.o ); break;
            case TYPEOF_f3_t:  r = sr_f3( *( f3_t* )v1.o * *( f3_t* )v2.o ); break;
            case TYPEOF_bl_t:  r = sr_f3( *( f3_t* )v1.o * *( bl_t* )v2.o ); break;
            case TYPEOF_v3d_s: r = sr_create( TYPEOF_v3d_s ); *( v3d_s* )r.o = v3d_s_mlf( *( v3d_s* )v2.o, *( f3_t* )v1.o ); break;
        }
        break;

        case TYPEOF_bl_t:
        switch( t2 )
        {
            case TYPEOF_s3_t:  r = sr_s3( *( bl_t* )v1.o * *( s3_t* )v2.o ); break;
            case TYPEOF_f3_t:  r = sr_f3( *( bl_t* )v1.o * *( f3_t* )v2.o ); break;
            case TYPEOF_bl_t:  r = sr_bl( *( bl_t* )v1.o * *( bl_t* )v2.o ); break;
            case TYPEOF_v3d_s: r = sr_create( TYPEOF_v3d_s ); *( v3d_s* )r.o = v3d_s_mlf( *( v3d_s* )v2.o, *( bl_t* )v1.o ); break;
        }
        break;

        case TYPEOF_v3d_s:
        switch( t2 )
        {
            case TYPEOF_s3_t:  r = sr_create( TYPEOF_v3d_s ); *( v3d_s* )r.o = v3d_s_mlf( *( v3d_s* )v1.o, *( s3_t* )v2.o ); break;
            case TYPEOF_f3_t:  r = sr_create( TYPEOF_v3d_s ); *( v3d_s* )r.o = v3d_s_mlf( *( v3d_s* )v1.o, *( f3_t* )v2.o ); break;
            case TYPEOF_bl_t:  r = sr_create( TYPEOF_v3d_s ); *( v3d_s* )r.o = v3d_s_mlf( *( v3d_s* )v1.o, *( bl_t* )v2.o ); break;
            case TYPEOF_v3d_s: r = sr_f3( v3d_s_mlv( *( v3d_s* )v1.o, *( v3d_s* )v2.o ) ); break;
        }
        break;

        case TYPEOF_m3d_s:
        switch( t2 )
        {
            case TYPEOF_s3_t:  r = sr_create( TYPEOF_m3d_s ); *( m3d_s* )r.o = m3d_s_mlf( v1.o, *( s3_t*  )v2.o ); break;
            case TYPEOF_f3_t:  r = sr_create( TYPEOF_m3d_s ); *( m3d_s* )r.o = m3d_s_mlf( v1.o, *( f3_t*  )v2.o ); break;
            case TYPEOF_v3d_s: r = sr_create( TYPEOF_v3d_s ); *( v3d_s* )r.o = m3d_s_mlv( v1.o, *( v3d_s* )v2.o ); break;
            case TYPEOF_m3d_s: r = sr_create( TYPEOF_m3d_s ); *( m3d_s* )r.o = m3d_s_mlm( v1.o, v2.o ); break;
        }
        break;
    }

    if( !r.o )
    {
        meval_s_err_fa( o, "Cannot evaluate '#<sc_t>' * '#<sc_t>'\n", ifnameof( t1 ), ifnameof( t2 ) );
    }

    sr_down( v1 );
    sr_down( v2 );

    return r;
}

static sr_s meval_s_add( meval_s* o, sr_s v1, sr_s v2 )
{
    tp_t t1 = sr_s_type( &v1 );
    tp_t t2 = sr_s_type( &v2 );
    sr_s r = sr_null();

    switch( t1 )
    {
        case TYPEOF_s3_t:
        switch( t2 )
        {
            case TYPEOF_s3_t:  r = sr_s3( *( s3_t* )v1.o + *( s3_t* )v2.o ); break;
            case TYPEOF_f3_t:  r = sr_f3( *( s3_t* )v1.o + *( f3_t* )v2.o ); break;
            case TYPEOF_bl_t:  r = sr_s3( *( s3_t* )v1.o + *( bl_t* )v2.o ); break;
        }
        break;

        case TYPEOF_f3_t:
        switch( t2 )
        {
            case TYPEOF_s3_t:  r = sr_f3( *( f3_t* )v1.o + *( s3_t* )v2.o ); break;
            case TYPEOF_f3_t:  r = sr_f3( *( f3_t* )v1.o + *( f3_t* )v2.o ); break;
            case TYPEOF_bl_t:  r = sr_f3( *( f3_t* )v1.o + *( bl_t* )v2.o ); break;
        }
        break;

        case TYPEOF_bl_t:
        switch( t2 )
        {
            case TYPEOF_s3_t:  r = sr_s3( *( bl_t* )v1.o + *( s3_t* )v2.o ); break;
            case TYPEOF_f3_t:  r = sr_f3( *( bl_t* )v1.o + *( f3_t* )v2.o ); break;
            case TYPEOF_bl_t:  r = sr_s3( *( bl_t* )v1.o + *( bl_t* )v2.o ); break;
        }
        break;

        case TYPEOF_v3d_s:
        switch( t2 )
        {
            case TYPEOF_v3d_s: r = sr_create( TYPEOF_v3d_s ); *( v3d_s* )r.o = v3d_s_add( *( v3d_s* )v1.o, *( v3d_s* )v2.o ); break;
        }
        break;

        case TYPEOF_st_s:
        {
            r = sr_clone( v1 );
            v1 = sr_null();
            switch( t2 )
            {
                case TYPEOF_st_s: st_s_push_st( r.o, v2.o ); break;
                case TYPEOF_s3_t: st_s_push_fa( r.o, "#<s3_t>", *( s3_t* )v2.o ); break;
            }
        }
        break;
    }

    if( !r.o )
    {
        meval_s_err_fa( o, "Cannot evaluate '#<sc_t>' + '#<sc_t>'\n", ifnameof( t1 ), ifnameof( t2 ) );
    }

    sr_down( v1 );
    sr_down( v2 );

    return r;
}

/// comparison
static s2_t meval_s_cmp( meval_s* o, sr_s v1, sr_s v2 )
{
    tp_t t1 = sr_s_type( &v1 );
    tp_t t2 = sr_s_type( &v2 );
    s2_t r = 2;

    switch( t1 )
    {
        case TYPEOF_s3_t:
        {
            s3_t v = *( s3_t* )v1.o;
            switch( t2 )
            {
                case TYPEOF_s3_t:  r = ( v < *( s3_t* )v2.o ) ? 1 : ( v > *( s3_t* )v2.o ) ? -1 : 0; break;
                case TYPEOF_f3_t:  r = ( v < *( f3_t* )v2.o ) ? 1 : ( v > *( f3_t* )v2.o ) ? -1 : 0; break;
                case TYPEOF_bl_t:  r = ( v < *( bl_t* )v2.o ) ? 1 : ( v > *( bl_t* )v2.o ) ? -1 : 0; break;
            }
        }
        break;

        case TYPEOF_f3_t:
        {
            f3_t v = *( f3_t* )v1.o;
            switch( t2 )
            {
                case TYPEOF_s3_t:  r = ( v < *( s3_t* )v2.o ) ? 1 : ( v > *( s3_t* )v2.o ) ? -1 : 0; break;
                case TYPEOF_f3_t:  r = ( v < *( f3_t* )v2.o ) ? 1 : ( v > *( f3_t* )v2.o ) ? -1 : 0; break;
                case TYPEOF_bl_t:  r = ( v < *( bl_t* )v2.o ) ? 1 : ( v > *( bl_t* )v2.o ) ? -1 : 0; break;
            }
        }
        break;

        case TYPEOF_bl_t:
        {
            bl_t v = *( bl_t* )v1.o;
            switch( t2 )
            {
                case TYPEOF_s3_t:  r = ( v < *( s3_t* )v2.o ) ? 1 : ( v > *( s3_t* )v2.o ) ? -1 : 0; break;
                case TYPEOF_f3_t:  r = ( v < *( f3_t* )v2.o ) ? 1 : ( v > *( f3_t* )v2.o ) ? -1 : 0; break;
                case TYPEOF_bl_t:  r = ( v < *( bl_t* )v2.o ) ? 1 : ( v > *( bl_t* )v2.o ) ? -1 : 0; break;
            }
        }
        break;
    }

    if( r == 2 )
    {
        meval_s_err_fa( o, "Cannot compare '#<sc_t>' with '#<sc_t>'\n", ifnameof( t1 ), ifnameof( t2 ) );
    }

    sr_down( v1 );
    sr_down( v2 );

    return r;
}

static sr_s meval_s_inverse( meval_s* o, sr_s v1 )
{
    tp_t t1 = sr_s_type( &v1 );
    sr_s r = sr_null();

    switch( t1 )
    {
        case TYPEOF_s3_t: r = sr_f3( *( s3_t* )v1.o != 0 ? 1.0 / *( s3_t* )v1.o : f3_inf ); break;
        case TYPEOF_f3_t: r = sr_f3( *( f3_t* )v1.o != 0 ? 1.0 / *( f3_t* )v1.o : f3_inf ); break;
    }
    if( !r.o )
    {
        meval_s_err_fa( o, "Cannot invert '#<sc_t>'\n", ifnameof( t1 ) );
    }

    sr_down( v1 );

    return r;
}

static bl_t meval_s_logic_and( meval_s* o, sr_s v1, sr_s v2 )
{
    bl_t r = false;

    if( sr_s_type( &v1 ) == TYPEOF_bl_t && sr_s_type( &v2 ) == TYPEOF_bl_t )
    {
        r = *( bl_t* )v1.o && *( bl_t* )v2.o;
    }
    else
    {
        meval_s_err_fa( o, "Cannot evaluate '#<sc_t>' AND '#<sc_t>'\n", sr_s_type( &v1 ), sr_s_type( &v2 ) );
    }

    sr_down( v1 );
    sr_down( v2 );

    return r;
}

static bl_t meval_s_logic_or( meval_s* o, sr_s v1, sr_s v2 )
{
    bl_t r = false;

    if( sr_s_type( &v1 ) == TYPEOF_bl_t && sr_s_type( &v2 ) == TYPEOF_bl_t )
    {
        r = *( bl_t* )v1.o || *( bl_t* )v2.o;
    }
    else
    {
        meval_s_err_fa( o, "Cannot evaluate '#<sc_t>' OR '#<sc_t>'\n", sr_s_type( &v1 ), sr_s_type( &v2 ) );
    }

    sr_down( v1 );
    sr_down( v2 );

    return r;
}

static bl_t meval_s_logic_xor( meval_s* o, sr_s v1, sr_s v2 )
{
    bl_t r = false;

    if( sr_s_type( &v1 ) == TYPEOF_bl_t && sr_s_type( &v2 ) == TYPEOF_bl_t )
    {
        r = ( *( bl_t* )v1.o && !( *( bl_t* )v2.o ) ) || ( !( *( bl_t* )v1.o ) && *( bl_t* )v2.o );
    }
    else
    {
        meval_s_err_fa( o, "Cannot evaluate '#<sc_t>' XOR '#<sc_t>'\n", sr_s_type( &v1 ), sr_s_type( &v2 ) );
    }

    sr_down( v1 );
    sr_down( v2 );

    return r;
}

static sr_s meval_s_logic_not( meval_s* o, sr_s v1 )
{
    bl_t r = false;

    if( sr_s_type( &v1 ) == TYPEOF_bl_t )
    {
        r = !( *( bl_t* )v1.o );
    }
    else
    {
        meval_s_err_fa( o, "Cannot evaluate NOT '#<sc_t>'\n", sr_s_type( &v1 ) );
    }

    sr_down( v1 );

    return sr_bl( r );
}

tp_t meval_s_peek_code( const meval_s* o )
{
    return mcode_s_peek_code( o->mcode, &o->index );
}

tp_t meval_s_get_code( meval_s* o )
{
    return mcode_s_get_code( o->mcode, &o->index );
}

sr_s meval_s_get_data( meval_s* o )
{
    if( !meval_s_try_code( o, CL_DATA ) ) meval_s_err_fa( o, "Data entry expected." );
    sz_t idx = meval_s_get_code( o );
    if( idx > o->mcode->data.size ) meval_s_err_fa( o, "Data index '#<sz_t>' out of range.", idx );
    return sr_cw( o->mcode->data.data[ idx ] );
}

bl_t  meval_s_try_code( meval_s* o, tp_t code )
{
    return mcode_s_try_code( o->mcode, &o->index, code );
}

bl_t  meval_s_try_name( meval_s* o, tp_t key )
{
    return mcode_s_try_name( o->mcode, &o->index, key );
}

bl_t  meval_s_end_code( meval_s* o )
{
    return mcode_s_end_code( o->mcode, &o->index );
}

void meval_s_expect_code( meval_s* o, tp_t code )
{
    if( !mcode_s_try_code( o->mcode, &o->index, code ) )
    {
        meval_s_err_fa( o, "'#sc_t' expected.", code_symbol( code ) );
    }
}

sz_t meval_s_get_index( meval_s* o )
{
    return o->index;
}

void meval_s_jmp_to( meval_s* o, sz_t address )
{
    if( address >= o->mcode->code.size ) meval_s_err_fa( o, "Target address out of range." );
    o->index = address;
}

v3d_s meval_s_eval_v3d( meval_s* o )
{
    sr_s vec = meval_s_eval( o, sr_null() );
    v3d_s ret;
    if     ( sr_s_type( &vec ) == TYPEOF_v3d_s ) ret = *( v3d_s* )vec.o;
    else if( sr_s_type( &vec ) == TYPEOF_cl_s  ) ret = *( cl_s* )vec.o;
    else meval_s_err_fa( o, "Vector expected." );
    sr_down( vec );
    return ret;
}

f3_t meval_s_eval_f3( meval_s* o )
{
    sr_s val = meval_s_eval( o, sr_null() );
    f3_t ret = 0;
    if     ( sr_s_type( &val ) == TYPEOF_f3_t ) ret = *( f3_t* )val.o;
    else if( sr_s_type( &val ) == TYPEOF_f2_t ) ret = *( f2_t* )val.o;
    else if( sr_s_type( &val ) == TYPEOF_u3_t ) ret = *( u3_t* )val.o;
    else if( sr_s_type( &val ) == TYPEOF_u2_t ) ret = *( u2_t* )val.o;
    else if( sr_s_type( &val ) == TYPEOF_s3_t ) ret = *( s3_t* )val.o;
    else if( sr_s_type( &val ) == TYPEOF_s2_t ) ret = *( s2_t* )val.o;
    else
    {
        meval_s_err_fa( o, "Scalar expected." );
    }
    sr_down( val );
    return ret;
}

sr_s meval_s_eval_texture_field( meval_s* o )
{
    sr_s val = meval_s_eval( o, sr_null() );
    if( !bcore_trait_is( sr_s_type( &val ), typeof( "spect_txm" ) ) ) meval_s_err_fa( o, "Texture map expected." );
    return val;
}

bl_t meval_s_eval_bl( meval_s* o )
{
    sr_s val = meval_s_eval( o, sr_null() );
    bl_t ret = 0;
    if( sr_s_type( &val ) == TYPEOF_bl_t ) ret = *( bl_t* )val.o;
    else
    {
        meval_s_err_fa( o, "Boolean expected." );
    }
    sr_down( val );
    return ret;
}

m3d_s meval_s_eval_rot( meval_s* o )
{
    sr_s mat = meval_s_eval( o, sr_null() );
    if( sr_s_type( &mat ) != TYPEOF_m3d_s ) meval_s_err_fa( o, "Rotation expected." );
    m3d_s ret = *( m3d_s* )mat.o;
    sr_down( mat );
    return ret;
}

/// calls a closure
sr_s meval_s_eval_call( meval_s* o, const sr_s* closure )
{
    if( !bcore_trait_is( sr_s_type( closure ), TYPEOF_bclos_closure ) )
    {
        meval_s_err_fa( o, "'#<sc_t>' is no function.", ifnameof( sr_s_type( closure ) ) );
    }
    meval_s_expect_code( o, CL_ROUND_BRACKET_OPEN  );
    bclos_arguments_s* args = bclos_arguments_s_create();
    sr_s sig_obj = bclos_closure_q_sig( closure );
    if( !sig_obj.o ) meval_s_err_fa( o, "Function '#<sc_t>' has no signature.", ifnameof( sr_s_type( closure ) ) );
    const bclos_signature_s* sig = sig_obj.o;
    for( sz_t i = 0; i < sig->size; i++ )
    {
        if( i > 0 ) meval_s_expect_code( o, CL_COMMA );
        sr_s arg  = meval_s_eval( o, sr_null() );
        tp_t sig_type = sig->data[ i ].type;
        tp_t arg_type = sr_s_type( &arg );
        if( arg_type != sig_type && !bcore_trait_is( arg_type, sig_type ) )
        {
            meval_s_err_fa( o, "Function '#<sc_t>': Argument #<sz_t> is '#<sc_t>' and not of '#<sc_t>'.",
                                ifnameof( sr_s_type( closure ) ),
                                i + 1,
                                ifnameof( arg_type ),
                                ifnameof( sig_type )
                           );
        }
        bclos_arguments_s_push( args, arg );
    }
    sr_s obj = sr_fork( bclos_closure_q_call( closure, o->frame, args ) );
    bclos_arguments_s_discard( args );
    meval_s_expect_code( o, CL_ROUND_BRACKET_CLOSE  );
    sr_down( sig_obj );
    return obj;
}

/// evaluates an expression
sr_s meval_s_eval( meval_s* o, sr_s front_obj )
{
    tp_t opr = CL_NULL;

    if( front_obj.p )
    {
        tp_t code = meval_s_peek_code( o );
        if( code > CL_OP_BEGIN && code < CL_OP_END )
        {
            opr = meval_s_get_code( o );
        }
        else
        {
            return sr_fork( front_obj );
        }

        // operators to be immediately processed
        if( opr > CL_ASSIGN_OPS_BEGIN && opr < CL_ASSIGN_OPS_END )
        {
            if( sr_s_is_const( &front_obj ) ) meval_s_err_fa( o, "Attempt to modify a const object.\n" );
            sr_s obj = meval_s_eval( o, sr_null() );
            if( !obj.p ) meval_s_err_fa( o, "Assignment from empty object.\n" );
            switch( opr )
            {
                case OP_ADD_ASSIGN: obj = meval_s_add( o, sr_cw( front_obj ), obj ); break;
                case OP_SUB_ASSIGN: obj = meval_s_add( o, sr_cw( front_obj ), meval_s_mul( o, sr_f3( -1 ), obj ) ); break;
                case OP_MUL_ASSIGN: obj = meval_s_mul( o, sr_cw( front_obj ), obj ); break;
                case OP_DIV_ASSIGN: obj = meval_s_mul( o, sr_cw( front_obj ), meval_s_inverse( o, obj ) ); break;
                default: break;
            }
            bcore_inst_typed_copy_typed( sr_s_type( &front_obj ), front_obj.o, sr_s_type( &obj ), obj.o );
            sr_down( obj );
            return sr_fork( front_obj );
        }

    }
    else // specific unary operators processed immediately
    {
        if( meval_s_try_code( o, OP_QUERY ) )
        {
            bcore_txt_ml_to_stdout( meval_s_eval( o, sr_null() ) );
            return sr_null();
        }
    }

    // unary operators
    tp_t unary_op = 0;
    switch( meval_s_peek_code( o ) )
    {
        case OP_ADD:
        case OP_SUB:
        case OP_NOT:
            unary_op = meval_s_get_code( o );
            break;

        default:
            break;
    }

    sr_s obj = sr_null();

    if( meval_s_peek_code( o ) == CL_DATA )
    {
        obj = sr_fork( meval_s_get_data( o ) );

        if( sr_s_type( &obj ) == TYPEOF_mcode_s ) // make a closure out of mcode_s
        {
            mclosure_s* mclosure = mclosure_s_create();
            mclosure_s_define( mclosure, o->frame, NULL, obj.o );
            sr_down( obj );
            obj = sr_tsd( TYPEOF_mclosure_s, mclosure );
        }

        if( meval_s_peek_code( o ) == CL_ROUND_BRACKET_OPEN ) // calling a closure
        {
            sr_s closure = obj;
            obj = meval_s_eval_call( o, &closure );
            sr_down( closure );
        }
    }
    else if( meval_s_try_code( o, CL_NAME ) ) // name of variable, constant or function
    {
        tp_t key = meval_s_get_code( o );
        if( opr == OP_DOT ) // member to front_obj
        {
            if( bcore_via_q_nexists( &front_obj, key ) )
            {
                obj = bcore_via_q_nget( &front_obj, key );
            }
            else
            {
                tp_t type = sr_s_type( &front_obj );
                switch( type )
                {
                    case TYPEOF_scene_s: obj = scene_s_meval_key( &front_obj, o, key ); break;
                    case TYPEOF_map_s:   obj =   map_s_meval_key( &front_obj, o, key ); break;
                    case TYPEOF_arr_s:   obj =   arr_s_meval_key( &front_obj, o, key ); break;
                    default:
                    {
                        if( bcore_trait_is( type, TYPEOF_spect_obj ) )
                        {
                            obj = obj_meval_key( &front_obj, o, key );
                        }
                        else
                        {
                            meval_s_err_fa( o, "Object '#<sc_t>' has no element named '#<sc_t>'.", ifnameof( sr_s_type( &front_obj ) ), meval_s_get_name( o, key ) );
                        }
                    }
                    break;
                }
            }
            opr = CL_NULL;
        }
        else // name is variable
        {
            sr_s* p_obj = meval_s_get_obj( o, key );

            tp_t peek_code = meval_s_peek_code( o );
            if( peek_code > CL_ASSIGN_OPS_BEGIN && peek_code < CL_ASSIGN_OPS_END ) // assignment (not consumed here)
            {
                if( !p_obj )
                {
                    meval_s_expect_code( o, OP_ASSIGN );
                    obj = sr_s_fork( meval_s_set_obj( o, key, sr_clone( meval_s_eval( o, sr_null() ) ) ) );
                }
                else
                {
                    if( sr_s_is_const( &obj ) ) meval_s_err_fa( o, "'#<sc_t>' is a constant.", meval_s_get_name( o, key ) );
                    obj = meval_s_eval( o, sr_s_fork( p_obj ) ); // consume assignment in nested cycle
                }
            }
            else if( !p_obj )
            {
                meval_s_err_fa( o, "Unknown name '#<sc_t>'\n", meval_s_get_name( o, key ) );
            }
            else
            {
                if( meval_s_peek_code( o ) == CL_ROUND_BRACKET_OPEN ) // calling a closure
                {
                    obj = meval_s_eval_call( o, p_obj );
                }
                else
                {
                    obj = sr_s_fork( p_obj );
                }
            }
        }
    }
    else if( meval_s_try_code( o, CL_ROUND_BRACKET_OPEN ) ) // nested expression
    {
        obj = meval_s_eval( o, sr_null() );
        meval_s_expect_code( o, CL_ROUND_BRACKET_CLOSE );
    }

    if( obj.p )
    {
        switch( unary_op )
        {
            case OP_SUB: obj = meval_s_mul( o, sr_s3( -1 ), obj ); break;
            case OP_NOT: obj = meval_s_logic_not( o, obj ); break;
            default: break;
        }

        if( opr )
        {
            switch( opr )
            {

                case OP_MUL: return meval_s_eval( o, meval_s_mul( o, front_obj, obj ) );
                case OP_DIV: return meval_s_eval( o, meval_s_mul( o, front_obj, meval_s_inverse( o, obj ) ) );

                case OP_EQUAL:         return meval_s_eval( o, sr_bl( meval_s_cmp( o, front_obj, obj ) == 0 ) );
                case OP_SMALLER:       return meval_s_eval( o, sr_bl( meval_s_cmp( o, front_obj, obj ) >  0 ) );
                case OP_SMALLER_EQUAL: return meval_s_eval( o, sr_bl( meval_s_cmp( o, front_obj, obj ) >= 0 ) );
                case OP_LARGER:        return meval_s_eval( o, sr_bl( meval_s_cmp( o, front_obj, obj ) <  0 ) );
                case OP_LARGER_EQUAL:  return meval_s_eval( o, sr_bl( meval_s_cmp( o, front_obj, obj ) <= 0 ) );

                case OP_ADD: return sr_fork( meval_s_add( o, front_obj, meval_s_eval( o, obj ) ) );
                case OP_SUB: return sr_fork( meval_s_add( o, front_obj, meval_s_eval( o, meval_s_mul( o, sr_s3( -1 ), obj ) ) ) );

                case OP_AND: return sr_bl( meval_s_logic_and( o, front_obj, meval_s_eval( o, obj ) ) );
                case OP_OR:  return sr_bl( meval_s_logic_or(  o, front_obj, meval_s_eval( o, obj ) ) );
                case OP_XOR: return sr_bl( meval_s_logic_xor( o, front_obj, meval_s_eval( o, obj ) ) );
                default : meval_s_err_fa( o, "Invalid operator '#<sc_t>'.\n", code_symbol( opr ) );
            }
        }
        else
        {
            sr_down( front_obj );
            return meval_s_eval( o, obj );
        }
    }
    else
    {
        if( opr ) meval_s_err_fa( o, "Expression does not yield an operand for operator '#<sc_t>'.\n", code_symbol( opr ) );
    }
    sr_down( front_obj );
    return sr_fork( obj );
}

sr_s meval_s_execute( meval_s* o )
{
    sr_s return_obj = sr_null();
    while( !meval_s_end_code( o ) )
    {
        sr_s obj = sr_null();

        // control structures
        tp_t code = meval_s_peek_code( o );
        if( code > CL_FL_BEGIN && code < CL_FL_END )
        {
            meval_s_get_code( o );
            if( code == FL_IF )
            {
                sz_t jmp_target = meval_s_get_code( o );
                meval_s_expect_code( o, CL_ROUND_BRACKET_OPEN );
                sr_s cond = meval_s_eval( o, sr_null() );
                meval_s_expect_code( o, CL_ROUND_BRACKET_CLOSE );
                if( sr_s_type( &cond ) != TYPEOF_bl_t ) meval_s_err_fa( o, "Expression does not evaluate to boolean." );
                bl_t flag = *( bl_t* )cond.o;
                sr_down( cond );
                if( flag )
                {
                    obj = meval_s_eval( o, sr_null() );
                }
                else
                {
                    meval_s_jmp_to( o, jmp_target );
                }

                if( meval_s_peek_code( o ) == FL_ELSE )
                {
                    meval_s_get_code( o );
                    sz_t jmp_target = meval_s_get_code( o );
                    if( flag )
                    {
                        meval_s_jmp_to( o, jmp_target );
                    }
                    else
                    {
                        obj = meval_s_eval( o, sr_null() );
                    }
                }
            }
            else if( code == FL_WHILE )
            {
                sz_t end_while = meval_s_get_code( o );
                sz_t begin_while = meval_s_get_index( o );
                for( ;; )
                {
                    meval_s_expect_code( o, CL_ROUND_BRACKET_OPEN );
                    sr_s cond = meval_s_eval( o, sr_null() );
                    meval_s_expect_code( o, CL_ROUND_BRACKET_CLOSE );
                    if( sr_s_type( &cond ) != TYPEOF_bl_t ) meval_s_err_fa( o, "Expression does not evaluate to boolean." );
                    bl_t flag = *( bl_t* )cond.o;
                    sr_down( cond );
                    if( flag )
                    {
                        sr_down( obj );
                        obj = meval_s_eval( o, sr_null() );
                        meval_s_jmp_to( o, begin_while );

                    }
                    else
                    {
                        meval_s_jmp_to( o, end_while );
                        break;
                    }
                }
            }
        }
        else
        {
            obj = meval_s_eval( o, sr_null() );
        }

        meval_s_expect_code( o, CL_SEMICOLON );
        sr_down( return_obj );
        return_obj = obj;
    }
    return return_obj;
}

/**********************************************************************************************************************/

typedef struct mclosure_s
{
    aware_t _;
    mcode_s*           mcode;
    bclos_signature_s* signature;
    bclos_frame_s*     lexical_frame;  // private
} mclosure_s;

static sc_t mclosure_s_def =
"mclosure_s = bclos_closure"
"{"
    "aware_t _;"
    "mcode_s*           mcode;"
    "bclos_signature_s* signature;"
    "private bclos_frame_s* lexical_frame;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( mclosure_s )

void mclosure_s_define( mclosure_s* o, bclos_frame_s* frame, bclos_signature_s* signature, mcode_s* mcode )
{
    mcode_s_discard(           o->mcode );
    bclos_frame_s_discard(     o->lexical_frame );
    bclos_signature_s_discard( o->signature );
    o->mcode         = bcore_fork( mcode );
    o->lexical_frame = frame;
    o->signature     = bcore_fork( signature );
    if( !o->signature )
    {
        o->signature = bclos_signature_s_parse_from_sc( "root inlined()" );
    }
}

static sr_s mclosure_s_call( mclosure_s* o, bclos_frame_s* frame, const bclos_arguments_s* args )
{
    bcore_life_s* l = bcore_life_s_create();

    bclos_frame_s* local_frame = bcore_life_s_push_aware( l, bclos_frame_s_create() ); // local frame
    meval_s*       meval       = bcore_life_s_push_aware( l, meval_s_create() );

    local_frame->external = o->lexical_frame ? o->lexical_frame : frame;

    meval->mcode = o->mcode;
    meval->frame = local_frame;

    if( o->signature )
    {
        for( sz_t i = 0; i < o->signature->size; i++ )
        {
            bclos_signature_arg_s sig_arg = o->signature->data[ i ];
            sr_s arg_obj = bclos_arguments_s_get( args, i, frame );
            bclos_frame_s_set( local_frame, sig_arg.name, arg_obj );
        }
    }

    sr_s return_obj = meval_s_execute( meval );
    bcore_life_s_discard( l );

    return return_obj;
}

sr_s mclosure_s_signature( const mclosure_s* o )
{
    return sr_twc( TYPEOF_bclos_signature_s, o->signature );
}

sr_s mclosure_s_interpret( const mclosure_s* const_o, sr_s source )
{
    bcore_life_s* l = bcore_life_s_create();
    sr_s src = sr_cp( bcore_life_s_push_sr( l, source ), TYPEOF_bcore_source_s );
    mclosure_s* o = bcore_life_s_push_aware( l, mclosure_s_clone( const_o ) );
    mcode_s* mcode = bcore_life_s_push_aware( l, mcode_s_create() );
    mcode_s_parse( mcode, &src );
    bclos_frame_s* frame = bcore_life_s_push_aware( l, bclos_frame_s_create() );
    bclos_frame_s_set( frame, typeof( "vec"       ), sr_create( typeof( "create_vec_s"       ) ) );
    bclos_frame_s_set( frame, typeof( "vecx"      ), sr_create( typeof( "vecx_s"             ) ) );
    bclos_frame_s_set( frame, typeof( "vecy"      ), sr_create( typeof( "vecy_s"             ) ) );
    bclos_frame_s_set( frame, typeof( "vecz"      ), sr_create( typeof( "vecz_s"             ) ) );
    bclos_frame_s_set( frame, typeof( "rotx"      ), sr_create( typeof( "rotx_s"             ) ) );
    bclos_frame_s_set( frame, typeof( "roty"      ), sr_create( typeof( "roty_s"             ) ) );
    bclos_frame_s_set( frame, typeof( "rotz"      ), sr_create( typeof( "rotz_s"             ) ) );
    bclos_frame_s_set( frame, typeof( "color"     ), sr_create( typeof( "create_color_s"     ) ) );
    bclos_frame_s_set( frame, typeof( "colr"      ), sr_create( typeof( "colr_s"             ) ) );
    bclos_frame_s_set( frame, typeof( "colg"      ), sr_create( typeof( "colg_s"             ) ) );
    bclos_frame_s_set( frame, typeof( "colb"      ), sr_create( typeof( "colb_s"             ) ) );

    bclos_frame_s_set( frame, typeof( "string_fa" ), sr_create( typeof( "create_string_fa_s" ) ) );
    mclosure_s_define( o, frame, NULL, mcode );
    sr_s return_obj = mclosure_s_call( o, NULL, NULL );
    bcore_life_s_discard( l );
    return return_obj;
}

static bcore_flect_self_s* mclosure_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( mclosure_s_def, sizeof( mclosure_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )mclosure_s_call,      "bclos_closure_fp_call", "call" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )mclosure_s_signature, "bclos_closure_fp_sig",  "sig"  );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )mclosure_s_interpret, "bcore_fp_interpret", "interpret" );
    return self;
}

st_s* mclosure_selftest()
{
    st_s* log = st_s_create();
    sr_s obj = bcore_interpret_auto_file( "../../rayflux/data/test.txt" );
    bcore_txt_ml_to_string( obj, log );
    return log;
}

/**********************************************************************************************************************/

vd_t interpreter_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "interpreter" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        bcore_flect_define_creator( typeof( "mcode_s"    ), mcode_s_create_self );
        bcore_flect_define_creator( typeof( "meval_s"    ), meval_s_create_self );
        bcore_flect_define_creator( typeof( "mclosure_s" ), mclosure_s_create_self );
    }
    else if( signal == typeof( "selftest" ) )
    {
        return mclosure_selftest();
    }

    return NULL;
}

/**********************************************************************************************************************/
