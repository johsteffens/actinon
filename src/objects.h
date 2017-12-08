/** 3D Objects
 *  Author: Johannes Bernhard Steffens
 *
 *  Copyright (c) 2017 Johannes Bernhard Steffens. All rights reserved.
 */

#ifndef OBJECTS_H
#define OBJECTS_H

#include "bcore_arr.h"

#include "vectors.h"

/**********************************************************************************************************************/
/// properties_s  (object's properties)

typedef struct properties_s
{
    v3d_s pos;      // position of origin
    m3d_s pax;      // object's principal axes
    vd_t  txm;      // texture map
    f3_t  radiance; // radiance (>0: object is active light source)
    f3_t  n;        // refractive index
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

/// projects on object's surface
v2d_s obj_prj( vc_t o, v3d_s pos );

/// object's surface-normal at given position
v3d_s obj_nor( vc_t o, v3d_s pos );

/// returns a (minimal) ray-cone with entire object in field of view
ray_cone_s obj_fov( vc_t o, v3d_s pos );

/// tests is object is (at least partially) in field of view
bl_t obj_is_in_fov( vc_t o, const ray_cone_s* fov );

/// returns object's hit position (offset) or f3_inf if not hit.
f3_t obj_hit( vc_t o, const ray_s* ray );

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

/// pushes an object to compound
vd_t compound_s_push( compound_s* o, tp_t type );

/// computes an object hit by given ray; returns f3_inf in case of no hit
f3_t compound_s_hit( const compound_s* o, const ray_s* r, vc_t* hit_obj );

/// computes a subset of objects in given field of view
bcore_arr_sz_s* compound_s_in_fov_arr( const compound_s* o, const ray_cone_s* fov );

/// above hit function on a subset specified by idx_arr
f3_t compound_s_idx_hit( const compound_s* o, const bcore_arr_sz_s* idx_arr, const ray_s* r, vc_t* hit_obj );

/**********************************************************************************************************************/
// quicktypes
#define TYPEOF_properties_s typeof( "properties_s" )
#define TYPEOF_compound_s   typeof( "compound_s" )

/**********************************************************************************************************************/

vd_t objects_signal( tp_t target, tp_t signal, vd_t object );

#endif // OBJECTS_H
