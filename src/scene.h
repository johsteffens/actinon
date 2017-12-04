/// Author & Copyright (C) 2017 Johannes Bernhard Steffens. All rights reserved.

#ifndef SCENE_H
#define SCENE_H

#include "vectors.h"

/**********************************************************************************************************************/

typedef struct image_cps_s image_cps_s;
DECLARE_FUNCTIONS_OBJ( image_cps_s )

void image_cps_s_set_size( image_cps_s* o, sz_t w, sz_t h, u2_t v );
tp_t image_cps_s_hash( const image_cps_s* o );
void image_cps_s_write_pnm( const image_cps_s* o, sc_t file );

typedef struct scene_s scene_s;
DECLARE_FUNCTIONS_OBJ( scene_s )

void scene_s_create_photon_map( scene_s* scene );
image_cps_s* scene_s_show_photon_map( const scene_s* scene );

image_cps_s* scene_s_create_image( const scene_s* scene );

/**********************************************************************************************************************/

vd_t scene_signal( tp_t target, tp_t signal, vd_t object );

#endif // SCENE_H
