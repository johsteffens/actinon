/** Texture Fields and Texture Mapping
 *  Author: Johannes Bernhard Steffens
 *
 *  Copyright (c) 2017 Johannes Bernhard Steffens. All rights reserved.
 */

#ifndef TEXTURES_H
#define TEXTURES_H

#include "vectors.h"
#include "quicktypes.h"

typedef struct txm_plain_s txm_plain_s;
DECLARE_FUNCTIONS_OBJ( txm_plain_s )

typedef struct txm_chess_s txm_chess_s;
DECLARE_FUNCTIONS_OBJ( txm_chess_s )

/**********************************************************************************************************************/

/// returns object texture color at surface position projected from pos
cl_s txm_clr( vc_t o, vc_t obj, v3d_s pos );

/// returns color of txm_plain_s; error if o is not txm_plain_s
cl_s txm_plain_clr( vc_t o );

/**********************************************************************************************************************/

vd_t textures_signal( tp_t target, tp_t signal, vd_t object );

#endif // TEXTURES_H
