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

#define TYPEOF_interpreter_s typeof( "interpreter_s" )
typedef struct interpreter_s
{
    aware_t _;
    sr_s source;
    bcore_name_map_s names;
    bcore_hmap_tp_sr_s variables;
    bcore_hmap_tp_sr_s constants;
    bcore_hmap_tpfp_s  f_eval; // function handlers (evaluates stream for arguments)
    bcore_hmap_tpfp_s  o_eval; // object   handlers (evaluates stream for elements or function_name with arguments)
    const struct interpreter_s* parent;
} interpreter_s;

DECLARE_FUNCTIONS_OBJ( interpreter_s )

/// object or function specific evaluation
typedef sr_s (*fp_eval)( sr_s* o, interpreter_s* ip );

sr_s interpreter_s_eval( interpreter_s* o, sr_s front_obj );

/**********************************************************************************************************************/

vd_t interpreter_signal( tp_t target, tp_t signal, vd_t object );

#endif // INTERPRETER_H

