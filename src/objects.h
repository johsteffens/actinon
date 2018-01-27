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
/// envelope_s  (sphere used to define object boundaries)
#define TYPEOF_envelope_s typeof( "envelope_s" )
typedef struct envelope_s
{
    v3d_s pos;
    f3_t radius;
} envelope_s;

DECLARE_FUNCTIONS_OBJ( envelope_s )

void envelope_s_move(           envelope_s* o, const v3d_s* vec );
void envelope_s_rotate(         envelope_s* o, const m3d_s* mat );
void envelope_s_scale(          envelope_s* o, f3_t fac );
bl_t envelope_s_ray_hits( const envelope_s* o, const ray_s* r );
f3_t envelope_s_ray_hit(  const envelope_s* o, const ray_s* r );
s3_t envelope_s_side(     const envelope_s* o, v3d_s pos );

/**********************************************************************************************************************/
/// properties_s  (object's properties)

typedef struct properties_s
{
    v3d_s pos;              // reference position of object
    m3d_s rax;              // object's local orthonormal system (reference-axes)
    vd_t  texture_field;    // 3D texture field
    cl_s  color;            // reflective albedo or radiance color in absence of a texture field

    f3_t  radiance;         // radiance (>0: object is active light source)
    f3_t  refractive_index;

    // incoming energy is processed in the order below
    f3_t fresnel_reflectivity;   // incoming energy taken by fresnel reflection
    f3_t chromatic_reflectivity; // residual energy taken chromatic (specular) reflection
    f3_t diffuse_reflectivity;   // residual energy taken by diffuse reflection
    f3_t sigma;                  // sigma of Oren-Nayar reflectance model
    cl_s transparency;           // residual energy taken by material transition
    // transparency defines the (per color channel) amount of energy absorbed at a transition length of 1 unit

    envelope_s* envelope; // optional envelope

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

/** tests if object is potentially (at least partially) in field of view
 *  true: object may be in fov
 *  false: object is definitely not in fov
 */
bl_t obj_is_in_fov( vc_t o, const ray_cone_s* fov );

/** tests if object is 'potentially' reachable by rays in a half-sphere around 'ray_field' of maximum length 'length'
 *  true: object may be reachable
 *  false: object is definitely not reachable
 */
bl_t obj_is_reachable( vc_t o, const ray_s* ray_field, f3_t length );

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
void obj_set_texture_field   ( vd_t o, vc_t texture_field );
void obj_set_envelope        ( vd_t obj, const envelope_s* env );

/**********************************************************************************************************************/
/// obj_plane_s

#define TYPEOF_obj_plane_s typeof( "obj_plane_s" )
typedef struct obj_plane_s obj_plane_s;
DECLARE_FUNCTIONS_OBJ( obj_plane_s )

/**********************************************************************************************************************/
/// obj_sphere_s

#define TYPEOF_obj_sphere_s typeof( "obj_sphere_s" )
typedef struct obj_sphere_s obj_sphere_s;
DECLARE_FUNCTIONS_OBJ( obj_sphere_s )

void obj_sphere_s_set_radius( obj_sphere_s* o, f3_t radius );
f3_t obj_sphere_s_get_radius( const obj_sphere_s* o );

/**********************************************************************************************************************/
/// obj_cylinder_s

#define TYPEOF_obj_cylinder_s typeof( "obj_cylinder_s" )
typedef struct obj_cylinder_s obj_cylinder_s;
DECLARE_FUNCTIONS_OBJ( obj_cylinder_s )

void obj_cylinder_s_set_radius( obj_cylinder_s* o, f3_t radius );

/**********************************************************************************************************************/
/// obj_cone_s

#define TYPEOF_obj_cone_s typeof( "obj_cone_s" )
typedef struct obj_cone_s obj_cone_s;
DECLARE_FUNCTIONS_OBJ( obj_cone_s )

/// full angle of cone opening (not half-angle) in degrees
void obj_cone_s_set_angle_d( obj_cone_s* o, f3_t angle );

/**********************************************************************************************************************/
/// obj_distance_s

#define TYPEOF_obj_distance_s typeof( "obj_distance_s" )
typedef struct obj_distance_s obj_distance_s;
DECLARE_FUNCTIONS_OBJ( obj_distance_s )

void obj_distance_s_set_distance( obj_distance_s* o, vc_t distance );
void obj_distance_s_set_cycles( obj_distance_s* o, sz_t cycles );

/**********************************************************************************************************************/
/// obj_pair_inside_s  (combination of two objects)

#define TYPEOF_obj_pair_inside_s typeof( "obj_pair_inside_s" )
typedef struct obj_pair_inside_s obj_pair_inside_s;
DECLARE_FUNCTIONS_OBJ( obj_pair_inside_s )

obj_pair_inside_s* obj_pair_inside_s_create_pair( vc_t o1, vc_t o2 );
sr_s obj_pair_inside_s_create_pair_sr( sr_s o1, sr_s o2 );

/**********************************************************************************************************************/
/// obj_pair_outside_s  (combination of two objects)

#define TYPEOF_obj_pair_outside_s typeof( "obj_pair_outside_s" )
typedef struct obj_pair_outside_s obj_pair_outside_s;
DECLARE_FUNCTIONS_OBJ( obj_pair_outside_s )

obj_pair_outside_s* obj_pair_outside_s_create_pair( vc_t o1, vc_t o2 );
sr_s obj_pair_outside_s_create_pair_sr( sr_s o1, sr_s o2 );

/**********************************************************************************************************************/
/// obj_neg_s  (negated objects inside <-> outside)

#define TYPEOF_obj_neg_s typeof( "obj_neg_s" )
typedef struct obj_neg_s obj_neg_s;
DECLARE_FUNCTIONS_OBJ( obj_neg_s )

obj_neg_s* obj_neg_s_create_neg( vc_t o1 );

/**********************************************************************************************************************/
/// obj_scale_s  (scales object independently in directions)

#define TYPEOF_obj_scale_s typeof( "obj_scale_s" )
typedef struct obj_scale_s obj_scale_s;
DECLARE_FUNCTIONS_OBJ( obj_scale_s )

obj_scale_s* obj_scale_s_create_scale( vc_t o1, v3d_s scale );

/**********************************************************************************************************************/

vd_t objects_signal( tp_t target, tp_t signal, vd_t object );

#endif // OBJECTS_H
