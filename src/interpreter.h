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
    CL_SEMICOLON, // ';'
    CL_ROUND_BRACKET_OPEN,   // '('
    CL_ROUND_BRACKET_CLOSE,  // ')'
    CL_SQUARE_BRACKET_OPEN,  // '['
    CL_SQUARE_BRACKET_CLOSE, // ']'
    CL_FSIGNATURE,       // '<-' indicates a function signature
    CL_DYN_ARRAY,        // '[]' indicates a dynamic array
    CL_CL_END, // end of controls

    /// operators
    CL_OP_BEGIN, // begin of operators
    OP_DOT, // '.'
    OP_QUERY,         // '?'  entire data structure to stdout
    OP_DOUBLE_QUERY,  // '??' data content of leaf type to stdout
    OP_MUL, // '*'
    OP_DIV, // '/'
    OP_ADD, // '+'
    OP_SUB, // '-'

    CL_ASSIGN_OPS_BEGIN,
    OP_ASSIGN, // '='
    OP_MUL_ASSIGN, // '*='
    OP_ADD_ASSIGN, // '+='
    OP_SUB_ASSIGN, // '-='
    OP_DIV_ASSIGN, // '/='
    CL_ASSIGN_OPS_END,

    OP_EQUAL,   // '=='
    OP_SMALLER, // '<'
    OP_UNEQUAL, // '<>'
    OP_SMALLER_EQUAL, // '<='

    OP_LARGER, // '>'
    OP_LARGER_EQUAL, // '>='

    // logic operators
    OP_NOT,  // '!', 'NOT'
    OP_AND,  // '&', 'AND'
    OP_OR,   // '|', 'OR'
    OP_XOR,  // '^', 'XOR'

    // other operators
    OP_CAT,  // ':' - catenation

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
sr_s  meval_s_eval_texture_field( meval_s* o ); // error if expression is not of spect_txm
bl_t  meval_s_eval_bl(         meval_s* o ); // error if expression does not yield bl_t
m3d_s meval_s_eval_rot(        meval_s* o ); // error if expression does not yield a rotation matrix
sr_s  meval_s_eval(            meval_s* o, sr_s front_obj );

/**********************************************************************************************************************/

typedef struct mclosure_s
{
    aware_t _;
    mcode_s*           mcode;
    bclos_signature_s* signature;
    bclos_frame_s*     lexical_frame;  // private
} mclosure_s;

DECLARE_FUNCTIONS_OBJ( mclosure_s )
void mclosure_s_define( mclosure_s* o, bclos_frame_s* frame, bclos_signature_s* signature, mcode_s* mcode );

/**********************************************************************************************************************/

vd_t interpreter_signal( tp_t target, tp_t signal, vd_t object );

#endif // INTERPRETER_H

