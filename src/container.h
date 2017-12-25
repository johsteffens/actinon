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

#ifndef CONTAINER_H
#define CONTAINER_H

#include "bcore_features.h"
#include "bcore_hmap_tp_sr.h"
#include "bcore_arr.h"

#include "interpreter.h"
#include "quicktypes.h"

/**********************************************************************************************************************/

typedef struct map_s
{
    aware_t _;
    bcore_hmap_tp_sr_s m;
} map_s;

DECLARE_FUNCTIONS_OBJ( map_s )

bl_t  map_s_exists( const map_s* o, tp_t key );
sr_s* map_s_get(          map_s* o, tp_t key );
void  map_s_set(          map_s* o, tp_t key, sr_s obj );

void map_s_move(   map_s* o, const v3d_s* vec );
void map_s_rotate( map_s* o, const m3d_s* mat );
void map_s_scale(  map_s* o, f3_t fac );

sr_s map_s_meval_key( sr_s* o, meval_s* ev, tp_t key );

/**********************************************************************************************************************/

typedef struct arr_s
{
    aware_t _;
    bcore_arr_sr_s a;
} arr_s;

DECLARE_FUNCTIONS_OBJ( arr_s )

sz_t arr_s_get_size( const arr_s* o );
sr_s* arr_s_get( arr_s* o, sz_t idx );
void arr_s_set( arr_s* o, sz_t idx, sr_s obj );
void arr_s_push( arr_s* o, sr_s obj );

void arr_s_move(   arr_s* o, const v3d_s* vec );
void arr_s_rotate( arr_s* o, const m3d_s* mat );
void arr_s_scale(  arr_s* o, f3_t fac );

sr_s arr_s_meval_key( sr_s* o, meval_s* ev, tp_t key );

/**********************************************************************************************************************/

vd_t container_signal( tp_t target, tp_t signal, vd_t object );

#endif // CONTAINER_H

