/// Author & Copyright (C) 2017 Johannes Bernhard Steffens. All rights reserved.

// Quickly obtainable types.

#ifndef QUICKTYPES_H
#define QUICKTYPES_H

#include "bcore_name_manager.h"
#include "bcore_quicktypes.h"

/// This function computes and formats defines below to stdout
void quicktypes_to_stdout( tp_t (*hash)( sc_t name ) );

/** List of predefined types for quick access.
 *  Created via quicktypes_to_stdout( NULL );
 */

#define TYPEOF_v3d_s        2345158304
#define TYPEOF_spect_txm_s  3271085024
#define TYPEOF_txm_plain_s  2242851901
#define TYPEOF_txm_chess_s  3472243821
#define TYPEOF_scene_s      3915967093
#define TYPEOF_spect_obj_s   309901730
#define TYPEOF_spect_obj     295833720
#define TYPEOF_properties_s 2952202878
#define TYPEOF_compound_s   2450623358
#define TYPEOF_meval_s       468175712
#define TYPEOF_mclosure_s   2748702287
#define TYPEOF_arr_s        2279435138
#define TYPEOF_map_s        3272089243
#define TYPEOF_clear        1550717474
#define TYPEOF_push         2272264157
#define TYPEOF_move          407568404
#define TYPEOF_rotate       2784296202
#define TYPEOF_scale        2190941297
#define TYPEOF_true         1303515621
#define TYPEOF_false         184981848
#define TYPEOF_NOT          2884519850
#define TYPEOF_AND          2439409094
#define TYPEOF_OR           2095360516
#define TYPEOF_XOR          1330009950
#define TYPEOF_if            959999494
#define TYPEOF_while         231090382
#define TYPEOF_else         3183434736
#define TYPEOF_vec          1932546277
#define TYPEOF_color        1031692888

#endif // QUICKTYPES_H
