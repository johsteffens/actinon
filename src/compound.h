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

#ifndef COMPOUND_H
#define COMPOUND_H

#include "quicktypes.h"
#include "bcore_arr.h"
#include "vectors.h"
#include "objects.h"

/**********************************************************************************************************************/
/// trans_data_s // ray transition data

typedef struct trans_data_s
{
    v3d_s exit_nor;
    obj_hdr_s* exit_obj;
    obj_hdr_s* enter_obj;
} trans_data_s;

BCORE_DECLARE_FUNCTIONS_OBJ( trans_data_s )

/**********************************************************************************************************************/
/// compound_s (array of objects)

typedef struct compound_s compound_s;

BCORE_DECLARE_FUNCTIONS_OBJ( compound_s )

sz_t           compound_s_get_size(   const compound_s* o );
const aware_t* compound_s_get_object( const compound_s* o, sz_t index );

/// envelopes
void compound_s_set_envelope( compound_s* o, const envelope_s* envelope );
void compound_s_set_auto_envelope( compound_s* o );

/// empties compound
void compound_s_clear( compound_s* o );

/// pushes an object to compound (copies object)
void compound_s_push_q( compound_s* o, const sr_s* object );
void compound_s_push(   compound_s* o, sr_s object );

/// computes an object hit by given ray; returns f3_inf in case of no hit
f3_t compound_s_ray_hit( const compound_s* o, const ray_s* r, v3d_s* p_nor, vc_t* hit_obj );
f3_t compound_s_ray_trans_hit( const compound_s* o, const ray_s* r, trans_data_s* trans );

/// counts number of objects where pos is on the side 'side'
sz_t compound_s_side_count( const compound_s* o, v3d_s pos, s2_t side );

void compound_s_move(   compound_s* o, const v3d_s* vec );
void compound_s_rotate( compound_s* o, const m3d_s* mat );
void compound_s_scale(  compound_s* o, f3_t fac );

/// executes a function given by key
sr_s compound_s_meval_key( sr_s* sr_o, meval_s* ev, tp_t key );

/**********************************************************************************************************************/

vd_t compound_signal( tp_t target, tp_t signal, vd_t object );

#endif // COMPOUND_H
