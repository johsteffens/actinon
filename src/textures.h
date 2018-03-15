/** Texture Fields and Texture Mapping */

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

#ifndef TEXTURES_H
#define TEXTURES_H

#include "bcore_std.h"
#include "vectors.h"
#include "quicktypes.h"

typedef struct txm_plain_s txm_plain_s;
BCORE_DECLARE_FUNCTIONS_OBJ( txm_plain_s )

typedef struct txm_chess_s txm_chess_s;
BCORE_DECLARE_FUNCTIONS_OBJ( txm_chess_s )

/**********************************************************************************************************************/

/// returns object texture color at surface position projected from pos
cl_s txm_clr( vc_t o, vc_t obj, v3d_s pos );

/// returns color of txm_plain_s; error if o is not txm_plain_s
cl_s txm_plain_clr( vc_t o );

/**********************************************************************************************************************/

vd_t textures_signal_handler( const bcore_signal_s* o );

#endif // TEXTURES_H
