/** Texture Mapping
 *  Author: Johannes Bernhard Steffens
 *
 *  Copyright (c) 2017 Johannes Bernhard Steffens. All rights reserved.
 */

#ifndef TEXTURES_H
#define TEXTURES_H

#include "vectors.h"

/**********************************************************************************************************************/

/// returns object texture color at surface position projected from pos
cl_s txm_clr( vc_t o, vc_t obj, v3d_s pos );

/// returns color of txm_plain_s; error if o is not txm_plain_s
cl_s txm_plain_clr( vc_t o );

/**********************************************************************************************************************/
// quicktypes
#define TYPEOF_spect_txm_s typeof( "spect_txm_s" )
#define TYPEOF_txm_plain_s typeof( "txm_plain_s" )
#define TYPEOF_txm_chess_s typeof( "txm_chess_s" )

vd_t textures_signal( tp_t target, tp_t signal, vd_t object );

#endif // TEXTURES_H
