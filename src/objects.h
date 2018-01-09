/** 3D Objects */

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

#ifndef OBJECTS_H
#define OBJECTS_H

#include "bcore_arr.h"

#include "vectors.h"
#include "interpreter.h"
#include "quicktypes.h"

/**********************************************************************************************************************/
/// properties_s  (object's properties)

typedef struct properties_s
{
    v3d_s pos;              // reference position of object
    m3d_s rax;              // object's local orthonormal system (reference-axes)
    vd_t  texture_field;    // 3D texture field
    f3_t  radiance;         // radiance (>0: object is active light source)
    f3_t  refractive_index;
    bl_t  transparent;

    /** Color of the object in absence of a texture field.
     *  For transparent material the color components are interpreted as absorption-distance:
     *  Distance at which the corresponding color intensity is halved.
     */
    cl_s color;
} properties_s;

/**********************************************************************************************************************/
// spect_obj_s (object perspective)

typedef struct spect_obj_s spect_obj_s;

/**********************************************************************************************************************/
// obj_hdr_s

/// common object header
typedef struct obj_hdr_s
{
    aware_t _;
    const spect_obj_s* p;
    properties_s prp;
} obj_hdr_s;

/**********************************************************************************************************************/

/// color on object's surface
cl_s obj_color( vc_t o, v3d_s pos );

/// projects on object's surface
v2d_s obj_projection( vc_t o, v3d_s pos );

/// returns a (minimal) ray-cone with entire object in field of view
ray_cone_s obj_fov( vc_t o, v3d_s pos );

/// tests if object is (at least partially) in field of view
bl_t obj_is_in_fov( vc_t o, const ray_cone_s* fov );

/// returns object's hit position (offset) or f3_inf if not hit.
f3_t obj_ray_hit( vc_t o, const ray_s* ray, v3d_s* p_nor );

/// return 1 when pos is outside object, -1 otherwise
s2_t obj_side( vc_t o, v3d_s pos );

f3_t obj_radiance( vc_t o );

sr_s obj_meval_key( sr_s* o, meval_s* ev, tp_t key );

void obj_move(   vd_t o, const v3d_s* vec );
void obj_rotate( vd_t o, const m3d_s* mat );
void obj_scale(  vd_t o, f3_t fac );
void obj_set_color           ( vd_t o, cl_s color );
void obj_set_refractive_index( vd_t o, f3_t val );
void obj_set_radiance        ( vd_t o, f3_t val );
void obj_set_transparent     ( vd_t o, bl_t flag );
void obj_set_texture_field   ( vd_t o, vc_t texture_field );

/**********************************************************************************************************************/
/// obj_pair_inside_s  (combination of two objects)

typedef struct obj_pair_inside_s obj_pair_inside_s;
DECLARE_FUNCTIONS_OBJ( obj_pair_inside_s )

obj_pair_inside_s* obj_pair_inside_s_create_pair( vc_t o1, vc_t o2 );

/**********************************************************************************************************************/
/// obj_pair_outside_s  (combination of two objects)

typedef struct obj_pair_outside_s obj_pair_outside_s;
DECLARE_FUNCTIONS_OBJ( obj_pair_outside_s )

obj_pair_outside_s* obj_pair_outside_s_create_pair( vc_t o1, vc_t o2 );

/**********************************************************************************************************************/
/// obj_neg_s  (negated objects inside <-> outside)

typedef struct obj_neg_s obj_neg_s;
DECLARE_FUNCTIONS_OBJ( obj_neg_s )

obj_neg_s* obj_neg_s_create_neg( vc_t o1 );

/**********************************************************************************************************************/
/// compound_s (array of objects)

typedef struct compound_s
{
    aware_t _;
    union
    {
        bcore_aware_link_array_s arr;
        struct
        {
            vd_t* data;
            sz_t size;
            sz_t space;
        };
    };
} compound_s;

/// empties compound
void compound_s_clear( compound_s* o );

/// pushes an object to compound
vd_t compound_s_push( compound_s* o, tp_t type );

/// pushes an object to compound (copies object)
vd_t compound_s_push_q( compound_s* o, const sr_s* object );

/// computes an object hit by given ray; returns f3_inf in case of no hit
f3_t compound_s_ray_hit( const compound_s* o, const ray_s* r, v3d_s* p_nor, vc_t* hit_obj );
f3_t compound_s_ray_trans_hit( const compound_s* o, const ray_s* r, v3d_s* p_exot_nor, vc_t* exit_obj, vc_t* enter_obj );

/// computes a subset of objects in given field of view
bcore_arr_sz_s* compound_s_in_fov_arr( const compound_s* o, const ray_cone_s* fov );

/// above hit function on a subset specified by idx_arr
f3_t compound_s_idx_ray_hit( const compound_s* o, const bcore_arr_sz_s* idx_arr, const ray_s* r, v3d_s* p_nor, vc_t* hit_obj );

/**********************************************************************************************************************/

vd_t objects_signal( tp_t target, tp_t signal, vd_t object );

#endif // OBJECTS_H
