/** Rayflux Scene */

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

#ifndef SCENE_H
#define SCENE_H

#include "vectors.h"
#include "interpreter.h"
#include "quicktypes.h"

/**********************************************************************************************************************/

extern bl_t scene_s_overwrite_output_files_g;

typedef struct image_cps_s image_cps_s;
DECLARE_FUNCTIONS_OBJ( image_cps_s )

void image_cps_s_set_size( image_cps_s* o, sz_t w, sz_t h, u2_t v );
tp_t image_cps_s_hash( const image_cps_s* o );
void image_cps_s_write_pnm( const image_cps_s* o, sc_t file );

typedef struct scene_s scene_s;
DECLARE_FUNCTIONS_OBJ( scene_s )

/// clears light & matter
void scene_s_clear( scene_s* o );

/// appends light & matter from object; returns number of atomic objects pushed
sz_t scene_s_push( scene_s* o, const sr_s* object );
sz_t scene_s_objects( const scene_s* o ); // number of objects

sr_s scene_s_meval_key( sr_s* o, meval_s* ev, tp_t key );

void scene_s_create_image_file( scene_s* o, sc_t file );

/**********************************************************************************************************************/

vd_t scene_signal( tp_t target, tp_t signal, vd_t object );

#endif // SCENE_H
