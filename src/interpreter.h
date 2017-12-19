/** Interpreter for Scene and Object Construction
 *  Author: Johannes Bernhard Steffens
 *
 *  Copyright (c) 2017 Johannes Bernhard Steffens. All rights reserved.
 */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "bcore_features.h"
#include "bcore_hmap.h"
#include "bcore_hmap_tp_sr.h"
#include "bclos_spect_closure.h"

#include "quicktypes.h"

/**********************************************************************************************************************/

/// operators and controls
typedef enum
{
    CL_NULL = 0,

    /// controls
    CL_CL_BEGIN, // begin of controls
    CL_DATA, // data followed by index
    CL_NAME, // name followed by name hash
    CL_COMMA, // ','
    CL_COLON, // ':'
    CL_SEMICOLON, // ';'
    CL_ROUND_BRACKET_OPEN,   // '('
    CL_ROUND_BRACKET_CLOSE,  // ')'
    CL_SQUARE_BRACKET_OPEN,  // '['
    CL_SQUARE_BRACKET_CLOSE, // ']'
    CL_CL_END, // end of controls

    /// operators
    CL_OP_BEGIN, // begin of operators
    OP_DOT, // '.'
    OP_QUERY,  // '?'

    OP_MUL, // '*'
    OP_MUL_ASSIGN, // '*='

    OP_DIV, // '/'
    OP_DIV_ASSIGN, // '/='

    OP_ADD, // '+'
    OP_ADD_ASSIGN, // '+='

    OP_SUB, // '-'
    OP_SUB_ASSIGN, // '-='

    OP_ASSIGN, // '='
    OP_EQUAL,  // '=='

    OP_SMALLER, // '<'
    OP_UNEQUAL, // '<>'
    OP_SMALLER_EQUAL, // '<='

    OP_LARGER, // '>'
    OP_LARGER_EQUAL, // '>='

    // logic operators
    OP_NOT,
    OP_AND,
    OP_OR,
    OP_XOR,
    CL_OP_END, // end of operators

    /// flow controls
    CL_FL_BEGIN, // begin of flow-controls
    FL_IF,       //
    FL_WHILE,    //
    FL_ELSE,     //
    CL_FL_END,   // end of flow-controls
} code_s;

sc_t code_symbol( code_s o );

/**********************************************************************************************************************/

typedef struct mcode_s mcode_s;
DECLARE_FUNCTIONS_OBJ( mcode_s )

/**********************************************************************************************************************/

typedef struct meval_s meval_s;
DECLARE_FUNCTIONS_OBJ( meval_s )

void  meval_s_err_fv(          meval_s* o, sc_t format, va_list args );
void  meval_s_err_fa(          meval_s* o, sc_t format, ... );
sr_s* meval_s_get_obj(   const meval_s* o, tp_t key );
sr_s* meval_s_set_obj(         meval_s* o, tp_t key, sr_s obj );
sc_t  meval_s_get_name(  const meval_s* o, tp_t key );
tp_t  meval_s_peek_code( const meval_s* o );
tp_t  meval_s_get_code(        meval_s* o );
bl_t  meval_s_try_code(        meval_s* o, tp_t code );
void  meval_s_expect_code(     meval_s* o, tp_t code );
bl_t  meval_s_try_name(        meval_s* o, tp_t key );
bl_t  meval_s_end_code(        meval_s* o );
v3d_s meval_s_eval_v3d(        meval_s* o ); // error if expression does not yield a vector
f3_t  meval_s_eval_f3 (        meval_s* o ); // error if expression does not yield f3_t
m3d_s meval_s_eval_rot(        meval_s* o ); // error if expression does not yield a rotation matrix
sr_s  meval_s_eval(            meval_s* o, sr_s front_obj );

/**********************************************************************************************************************/

typedef struct mclosure_s mclosure_s;
DECLARE_FUNCTIONS_OBJ( mclosure_s )
void mclosure_s_define( mclosure_s* o, bclos_frame_s* frame, bclos_signature_s* signature, mcode_s* mcode );

/**********************************************************************************************************************/

vd_t interpreter_signal( tp_t target, tp_t signal, vd_t object );

#endif // INTERPRETER_H

