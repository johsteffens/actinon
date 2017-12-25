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

#include <math.h>
#include "bcore_spect_inst.h"
#include "bcore_life.h"
#include "bcore_spect.h"
#include "bcore_spect_array.h"
#include "bcore_trait.h"

#include "textures.h"
#include "objects.h"
#include "gmath.h"

/**********************************************************************************************************************/
/// properties_s  (object's properties)

static sc_t properties_s_def =
"properties_s = bcore_inst"
"{"
    "v3d_s   pos;"
    "m3d_s   rax;"
    "aware * texture_field;"
    "f3_t    radiance;"
    "f3_t    refractive_index;"
    "bl_t    transparent;"
    "cl_s    color;"
"}";

void properties_s_init( properties_s* o )
{
    bcore_memzero( o, sizeof( *o ) );
    o->pos   = ( v3d_s ){ 0, 0, 0 };
    o->rax.x = ( v3d_s ){ 1, 0, 0 };
    o->rax.y = ( v3d_s ){ 0, 1, 0 };
    o->rax.z = ( v3d_s ){ 0, 0, 1 };
    o->refractive_index = 1.0;
    o->color = ( cl_s  ){ 0.7, 0.7, 0.7 };
}

DEFINE_FUNCTION_COPY_INST( properties_s )
DEFINE_FUNCTION_DOWN_INST( properties_s )
DEFINE_FUNCTIONS_CDC( properties_s )

void properties_s_move( properties_s* o, const v3d_s* vec )
{
    v3d_s_o_add( &o->pos, *vec );
}

void properties_s_rotate( properties_s* o, const m3d_s* mat )
{
    o->rax = m3d_s_mlm( mat, &o->rax );
    o->pos = m3d_s_mlv( mat, o->pos );
}

void properties_s_scale( properties_s* o, f3_t fac )
{
    v3d_s_o_mlf( &o->pos, fac );
}

static bcore_flect_self_s* properties_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( properties_s_def, sizeof( properties_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )properties_s_init, "bcore_fp_init", "init" );
    return self;
}

/**********************************************************************************************************************/
/// spect_obj_s

typedef v2d_s      (*projection_fp )( vc_t o, v3d_s pos );
typedef v3d_s      (*normal_fp     )( vc_t o, v3d_s pos );
typedef bl_t       (*outside_fp    )( vc_t o, v3d_s pos );
typedef f3_t       (*fwd_hit_fp    )( vc_t o, const ray_s* ray );
typedef ray_cone_s (*fov_fp        )( vc_t o, v3d_s pos );
typedef bl_t       (*is_in_fov_fp  )( vc_t o, const ray_cone_s* fov );

typedef void (*move_fp  )( vd_t o, const v3d_s* vec );
typedef void (*rotate_fp)( vd_t o, const m3d_s* mat );
typedef void (*scale_fp )( vd_t o, f3_t fac );

typedef struct spect_obj_s
{
    aware_t p_type;
    tp_t    o_type;

    projection_fp fp_projection;
    normal_fp     fp_normal;
    outside_fp    fp_outside;
    fov_fp        fp_fov;
    fwd_hit_fp    fp_fwd_hit;

    move_fp       fp_move;
    rotate_fp     fp_rotate;
    scale_fp      fp_scale;

    is_in_fov_fp  fp_is_in_fov;
} spect_obj_s;

DEFINE_FUNCTIONS_OBJ_INST( spect_obj_s )

const spect_obj_s* obj_get_spect( vc_t o ) { return ( ( obj_hdr_s* )o )->p; }

static spect_obj_s* spect_obj_s_create_from_self( const bcore_flect_self_s* self )
{
    assert( self != NULL );
    spect_obj_s* o = spect_obj_s_create();
    o->o_type = self->type;
    o->fp_projection = ( projection_fp )bcore_flect_self_s_get_external_fp( self, entypeof( "projection_fp" ), 0 );
    o->fp_normal     = ( normal_fp     )bcore_flect_self_s_get_external_fp( self, entypeof( "normal_fp"     ), 0 );
    o->fp_outside    = ( outside_fp    )bcore_flect_self_s_get_external_fp( self, entypeof( "outside_fp"    ), 0 );
    o->fp_fov        = ( fov_fp        )bcore_flect_self_s_get_external_fp( self, entypeof( "fov_fp"        ), 0 );
    o->fp_fwd_hit    = ( fwd_hit_fp    )bcore_flect_self_s_get_external_fp( self, entypeof( "fwd_hit_fp"    ), 0 );
    o->fp_is_in_fov  = ( is_in_fov_fp  )bcore_flect_self_s_get_external_fp( self, entypeof( "is_in_fov_fp"  ), 0 );
    o->fp_move       = ( move_fp       )bcore_flect_self_s_get_external_fp( self, entypeof( "move_fp"       ), 0 );
    o->fp_rotate     = ( rotate_fp     )bcore_flect_self_s_get_external_fp( self, entypeof( "rotate_fp"     ), 0 );
    o->fp_scale      = ( scale_fp      )bcore_flect_self_s_get_external_fp( self, entypeof( "scale_fp"      ), 0 );
    return o;
}

v2d_s      obj_projection( vc_t o, v3d_s pos ) { return obj_get_spect( o )->fp_projection( o, pos ); }
v3d_s      obj_normal    ( vc_t o, v3d_s pos ) { return obj_get_spect( o )->fp_normal(  o, pos ); }
bl_t       obj_outside   ( vc_t o, v3d_s pos ) { return obj_get_spect( o )->fp_outside( o, pos ); }
ray_cone_s obj_fov       ( vc_t o, v3d_s pos ) { return obj_get_spect( o )->fp_fov( o, pos ); }
f3_t       obj_fwd_hit   ( vc_t o, const ray_s* ray ) { return obj_get_spect( o )->fp_fwd_hit( o, ray ); }
f3_t       obj_radiance  ( vc_t o                   ) { return ( ( obj_hdr_s* )o )->prp.radiance; }
void       obj_move      ( vd_t o, const v3d_s* vec ) { obj_get_spect( o )->fp_move(   o, vec ); }
void       obj_rotate    ( vd_t o, const m3d_s* mat ) { obj_get_spect( o )->fp_rotate( o, mat ); }
void       obj_scale     ( vd_t o, f3_t fac         ) { obj_get_spect( o )->fp_scale(  o, fac ); }
bl_t       obj_is_in_fov ( vc_t o, const ray_cone_s* fov )  { return obj_get_spect( o )->fp_is_in_fov( o, fov ); }

cl_s obj_color( vc_t obj, v3d_s pos )
{
    const obj_hdr_s* o = obj;
    if( o->prp.texture_field )
    {
        return txm_clr( o->prp.texture_field, o, pos );
    }
    else
    {
        return o->prp.color;
    }
}

void obj_set_color( vd_t obj, cl_s color )
{
    obj_hdr_s* o = obj;
    o->prp.color = color;
}

void obj_set_refractive_index( vd_t obj, f3_t refractive_index )
{
    obj_hdr_s* o = obj;
    o->prp.refractive_index = refractive_index;
}

void obj_set_radiance( vd_t obj, f3_t radiance )
{
    obj_hdr_s* o = obj;
    o->prp.radiance = radiance;
}

void obj_set_transparent( vd_t obj, bl_t flag )
{
    obj_hdr_s* o = obj;
    o->prp.transparent = flag;
}

void obj_set_texture_field( vd_t obj, vc_t texture_field )
{
    obj_hdr_s* o = obj;
    bcore_inst_aware_discard( o->prp.texture_field );
    o->prp.texture_field = bcore_inst_aware_clone( texture_field );
}

sr_s obj_meval_key( sr_s* sr_o, meval_s* ev, tp_t key )
{
    assert( bcore_trait_is( sr_s_type( sr_o ), TYPEOF_spect_obj ) );
    if( key == TYPEOF_move )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        v3d_s v = meval_s_eval_v3d( ev );
        obj_move( sr_o->o, &v );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_rotate )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        m3d_s rot = meval_s_eval_rot( ev );
        obj_rotate( sr_o->o, &rot );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_scale )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj_scale( sr_o->o, meval_s_eval_f3( ev ) );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_set_color )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj_set_color( sr_o->o, meval_s_eval_v3d( ev ) );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_set_refractive_index )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj_set_refractive_index( sr_o->o, meval_s_eval_f3( ev ) );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_set_radiance )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj_set_radiance( sr_o->o, meval_s_eval_f3( ev ) );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_set_transparent )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj_set_transparent( sr_o->o, meval_s_eval_bl( ev ) );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == TYPEOF_set_texture_field )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        sr_s v = meval_s_eval_texture_field( ev );
        obj_set_texture_field( sr_o->o, v.o );
        sr_down( v );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else
    {
        meval_s_err_fa( ev, "object has no member or function '#sc_t'.", meval_s_get_name( ev, key ) );
    }
    return sr_null();
}

static bcore_flect_self_s* spect_obj_s_create_self( void )
{
    sc_t def = "spect_obj_s = spect { aware_t p_type; tp_t o_type; ... }";
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( def, sizeof( spect_obj_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )spect_obj_s_create_from_self, "bcore_spect_fp_create_from_self", "create_from_self" );
    return self;
}

/**********************************************************************************************************************/
/// obj_plane_s

#define TYPEOF_obj_plane_s typeof( "obj_plane_s" )
typedef struct obj_plane_s
{
    union
    {
        obj_hdr_s hdr;
        struct
        {
            aware_t _;
            const spect_obj_s* p;
            properties_s prp;
        };
    };
} obj_plane_s;

static sc_t obj_plane_s_def =
"obj_plane_s = spect_obj"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_plane_s )

static void obj_plane_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; obj_plane_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
}

v2d_s obj_plane_s_projection( const obj_plane_s* o, v3d_s pos )
{
    v3d_s p = v3d_s_sub( pos, o->prp.pos );
    return ( v2d_s ) { v3d_s_mlv( p, o->prp.rax.x ), v3d_s_mlv( p, o->prp.rax.y ) };
}

v3d_s obj_plane_s_normal( const obj_plane_s* o, v3d_s pos )
{
    return plane_observer_normal( o->prp.pos, o->prp.rax.z, pos );
}

bl_t obj_plane_s_outside( const obj_plane_s* o, v3d_s pos )
{
    return plane_observer_outside( o->prp.pos, o->prp.rax.z, pos );
}

ray_cone_s obj_plane_s_fov( const obj_plane_s* o, v3d_s pos )
{
    ray_cone_s cne;
    cne.ray.p = pos;
    cne.ray.d = v3d_s_neg( o->prp.rax.z );
    cne.cos_rs = v3d_s_mlv( v3d_s_sub( o->prp.pos, pos ), cne.ray.d ) > 0 ? 0 : 1;
    return cne;
}

f3_t obj_plane_s_fwd_hit( const obj_plane_s* o, const ray_s* r )
{
    return plane_ray_offset( o->prp.pos, o->prp.rax.z, r );
}

bl_t obj_plane_s_is_in_fov( const obj_plane_s* o, const ray_cone_s* fov )
{
    if( obj_plane_s_fwd_hit( o, &fov->ray ) < f3_inf ) return true;
    f3_t sin_a = v3d_s_mlv( o->prp.rax.z, fov->ray.d );
    sin_a = sin_a < 1.0 ? sin_a : 1.0;
    f3_t cos_a = sqrt( 1.0 - sin_a * sin_a );
    return cos_a > fov->cos_rs;
}

void obj_plane_s_move(   obj_plane_s* o, const v3d_s* vec ) { properties_s_move  ( &o->prp, vec ); }
void obj_plane_s_rotate( obj_plane_s* o, const m3d_s* mat ) { properties_s_rotate( &o->prp, mat ); }
void obj_plane_s_scale(  obj_plane_s* o, f3_t fac         ) { properties_s_scale ( &o->prp, fac ); }

static bcore_flect_self_s* obj_plane_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_plane_s_def, sizeof( obj_plane_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_init_a,     "ap_t",          "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_projection, "projection_fp", "projection" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_normal,     "normal_fp",     "normal" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_outside,    "outside_fp",    "outside" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_fov,        "fov_fp",        "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_fwd_hit,    "fwd_hit_fp",    "fwd_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_is_in_fov,  "is_in_fov_fp",  "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_move,       "move_fp",       "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_rotate,     "rotate_fp",     "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_scale,      "scale_fp",      "scale" );
    return self;
}

/**********************************************************************************************************************/
/// obj_sphere_s

#define TYPEOF_obj_sphere_s typeof( "obj_sphere_s" )
typedef struct obj_sphere_s
{
    union
    {
        obj_hdr_s hdr;
        struct
        {
            aware_t _;
            const spect_obj_s* p;
            properties_s prp;
        };
    };
    f3_t  radius;
} obj_sphere_s;

static sc_t obj_sphere_s_def =
"obj_sphere_s = spect_obj"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
    "f3_t radius = 1.0;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_sphere_s )

static void obj_sphere_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; obj_sphere_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
}

v2d_s obj_sphere_s_projection( const obj_sphere_s* o, v3d_s pos )
{
    v3d_s r = v3d_s_of_length( v3d_s_sub( pos, o->prp.pos ), 1.0 );
    f3_t x = v3d_s_mlv( r, o->prp.rax.x );
    f3_t y = v3d_s_mlv( r, v3d_s_mlx( o->prp.rax.z, o->prp.rax.x ) );
    f3_t z = v3d_s_mlv( r, o->prp.rax.z );

    f3_t azimuth = atan2( x, y );

    /// correct rounding errors
    z = z >  1.0 ?  1.0 : z;
    z = z < -1.0 ? -1.0 : z;
    f3_t elevation = asin( z );

    return ( v2d_s ) { azimuth, elevation };
}

v3d_s obj_sphere_s_normal( const obj_sphere_s* o, v3d_s pos )
{
    return sphere_observer_normal( o->prp.pos, o->radius, pos );
}

bl_t obj_sphere_s_outside( const obj_sphere_s* o, v3d_s pos )
{
    return sphere_observer_outside( o->prp.pos, o->radius, pos );
}

ray_cone_s obj_sphere_s_fov( const obj_sphere_s* o, v3d_s pos )
{
    ray_cone_s cne;
    v3d_s diff = v3d_s_sub( o->prp.pos, pos );
    cne.ray.d = v3d_s_of_length( diff, 1.0 );
    cne.ray.p = pos;
    f3_t diff_sqr = v3d_s_sqr( diff );
    f3_t radius_sqr = f3_sqr( o->radius );

    if( diff_sqr > radius_sqr )
    {
        cne.cos_rs = sqrt( 1.0 - ( radius_sqr / diff_sqr ) );
    }
    else
    {
        cne.cos_rs = -1;
    }
    return cne;
}

bl_t obj_sphere_s_is_in_fov( const obj_sphere_s* o, const ray_cone_s* fov )
{
    v3d_s diff = v3d_s_sub( o->prp.pos, fov->ray.p );
    f3_t diff_sqr = v3d_s_sqr( diff );
    f3_t cos_ang0 = v3d_s_mlv( v3d_s_of_length( diff, 1.0 ), fov->ray.d );
    if( cos_ang0 > fov->cos_rs ) return true;

    f3_t radius_sqr = f3_sqr( o->radius );
    if( diff_sqr <= radius_sqr ) return false;
    f3_t cos_ang1 = ( diff_sqr > radius_sqr ) ? sqrt( 1.0 - ( radius_sqr / diff_sqr ) ) : 0;

    return acos( cos_ang0 ) - acos( cos_ang1 ) < acos( fov->cos_rs );
}

f3_t obj_sphere_s_fwd_hit( const obj_sphere_s* o, const ray_s* r )
{
    return sphere_ray_offset( o->prp.pos, o->radius, r );
}

void obj_sphere_s_move(   obj_sphere_s* o, const v3d_s* vec ) { properties_s_move  ( &o->prp, vec ); }
void obj_sphere_s_rotate( obj_sphere_s* o, const m3d_s* mat ) { properties_s_rotate( &o->prp, mat ); }
void obj_sphere_s_scale(  obj_sphere_s* o, f3_t fac         ) { properties_s_scale ( &o->prp, fac ); o->radius *= fac; }

static bcore_flect_self_s* obj_sphere_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_sphere_s_def, sizeof( obj_sphere_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_init_a,     "ap_t",          "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_projection, "projection_fp", "projection" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_outside,    "outside_fp",    "outside" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_normal,     "normal_fp",     "normal" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_fov,        "fov_fp",        "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_fwd_hit,    "fwd_hit_fp",    "fwd_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_is_in_fov,  "is_in_fov_fp",  "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_move,       "move_fp",       "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_rotate,     "rotate_fp",     "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_scale,      "scale_fp",      "scale" );
    return self;
}

/**********************************************************************************************************************/
/// obj_cylinder_s

#define TYPEOF_obj_cylinder_s typeof( "obj_cylinder_s" )
typedef struct obj_cylinder_s
{
    union
    {
        obj_hdr_s hdr;
        struct
        {
            aware_t _;
            const spect_obj_s* p;
            properties_s prp;
        };
    };
    f3_t radius;
} obj_cylinder_s;

static sc_t obj_cylinder_s_def =
"obj_cylinder_s = spect_obj"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
    "f3_t radius = 1.0;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_cylinder_s )

static void obj_cylinder_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; obj_cylinder_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
}

v2d_s obj_cylinder_s_projection( const obj_cylinder_s* o, v3d_s pos )
{
    v3d_s r = v3d_s_of_length( v3d_s_sub( pos, o->prp.pos ), 1.0 );
    f3_t x = v3d_s_mlv( r, o->prp.rax.x );
    f3_t y = v3d_s_mlv( r, v3d_s_mlx( o->prp.rax.z, o->prp.rax.x ) );
    f3_t z = v3d_s_mlv( r, o->prp.rax.z );

    f3_t azimuth = atan2( x, y );

    return ( v2d_s ) { azimuth, z };
}

v3d_s obj_cylinder_s_normal( const obj_cylinder_s* o, v3d_s pos )
{
    return cylinder_observer_normal( o->prp.pos, o->prp.rax.z, o->radius, pos );
}

bl_t obj_cylinder_s_outside( const obj_cylinder_s* o, v3d_s pos )
{
    return cylinder_observer_outside( o->prp.pos, o->prp.rax.z, o->radius, pos );
}

ray_cone_s obj_cylinder_s_fov( const obj_cylinder_s* o, v3d_s pos )
{
    ray_cone_s cne;
    cne.ray.p  = pos;
    cne.ray.d  = v3d_s_neg( obj_cylinder_s_normal( o, pos ) );
    cne.cos_rs = 0;
    return cne;
}

bl_t obj_cylinder_s_is_in_fov( const obj_cylinder_s* o, const ray_cone_s* fov )
{
    /// TODO: provide exact calculation
    return true;
}

f3_t obj_cylinder_s_fwd_hit( const obj_cylinder_s* o, const ray_s* r )
{
    return cylinder_ray_offset( o->prp.pos, o->prp.rax.z, o->radius, r );
}

void obj_cylinder_s_move(   obj_cylinder_s* o, const v3d_s* vec ) { properties_s_move  ( &o->prp, vec ); }
void obj_cylinder_s_rotate( obj_cylinder_s* o, const m3d_s* mat ) { properties_s_rotate( &o->prp, mat ); }
void obj_cylinder_s_scale(  obj_cylinder_s* o, f3_t fac         ) { properties_s_scale ( &o->prp, fac ); o->radius *= fac; }

static bcore_flect_self_s* obj_cylinder_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_cylinder_s_def, sizeof( obj_cylinder_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cylinder_s_init_a,     "ap_t",          "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cylinder_s_projection, "projection_fp", "projection" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cylinder_s_normal,     "normal_fp",     "normal" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cylinder_s_outside,    "outside_fp",    "outside" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cylinder_s_fov,        "fov_fp",        "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cylinder_s_fwd_hit,    "fwd_hit_fp",    "fwd_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cylinder_s_is_in_fov,  "is_in_fov_fp",  "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cylinder_s_move,       "move_fp",       "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cylinder_s_rotate,     "rotate_fp",     "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cylinder_s_scale,      "scale_fp",      "scale" );
    return self;
}

/**********************************************************************************************************************/
/// obj_cone_s

#define TYPEOF_obj_cone_s typeof( "obj_cone_s" )
typedef struct obj_cone_s
{
    union
    {
        obj_hdr_s hdr;
        struct
        {
            aware_t _;
            const spect_obj_s* p;
            properties_s prp;
        };
    };
    f3_t cosa;
} obj_cone_s;

static sc_t obj_cone_s_def =
"obj_cone_s = spect_obj"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
    "f3_t cosa = 0.5;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_cone_s )

static void obj_cone_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; obj_cone_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
}

v2d_s obj_cone_s_projection( const obj_cone_s* o, v3d_s pos )
{
    v3d_s r = v3d_s_of_length( v3d_s_sub( pos, o->prp.pos ), 1.0 );
    f3_t x = v3d_s_mlv( r, o->prp.rax.x );
    f3_t y = v3d_s_mlv( r, v3d_s_mlx( o->prp.rax.z, o->prp.rax.x ) );
    f3_t z = v3d_s_mlv( r, o->prp.rax.z );

    f3_t azimuth = atan2( x, y );

    return ( v2d_s ) { azimuth, z };
}

v3d_s obj_cone_s_normal( const obj_cone_s* o, v3d_s pos )
{
    return cone_observer_normal( o->prp.pos, o->prp.rax.z, o->cosa, pos );
}

bl_t obj_cone_s_outside( const obj_cone_s* o, v3d_s pos )
{
    return cone_observer_outside( o->prp.pos, o->prp.rax.z, o->cosa, pos );
}

ray_cone_s obj_cone_s_fov( const obj_cone_s* o, v3d_s pos )
{
    ray_cone_s cne;
    cne.ray.p  = pos;
    cne.ray.d  = v3d_s_neg( obj_cone_s_normal( o, pos ) );
    cne.cos_rs = 0;
    return cne;
}

bl_t obj_cone_s_is_in_fov( const obj_cone_s* o, const ray_cone_s* fov )
{
    /// TODO: provide exact calculation
    return true;
}

f3_t obj_cone_s_fwd_hit( const obj_cone_s* o, const ray_s* r )
{
    return cone_ray_offset( o->prp.pos, o->prp.rax.z, o->cosa, r );
}

void obj_cone_s_move(   obj_cone_s* o, const v3d_s* vec ) { properties_s_move  ( &o->prp, vec ); }
void obj_cone_s_rotate( obj_cone_s* o, const m3d_s* mat ) { properties_s_rotate( &o->prp, mat ); }
void obj_cone_s_scale(  obj_cone_s* o, f3_t fac         ) { properties_s_scale ( &o->prp, fac ); }

static bcore_flect_self_s* obj_cone_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_cone_s_def, sizeof( obj_cone_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cone_s_init_a,     "ap_t",          "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cone_s_projection, "projection_fp", "projection" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cone_s_normal,     "normal_fp",     "normal" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cone_s_outside,    "outside_fp",    "outside" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cone_s_fov,        "fov_fp",        "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cone_s_fwd_hit,    "fwd_hit_fp",    "fwd_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cone_s_is_in_fov,  "is_in_fov_fp",  "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cone_s_move,       "move_fp",       "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cone_s_rotate,     "rotate_fp",     "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_cone_s_scale,      "scale_fp",      "scale" );
    return self;
}

/**********************************************************************************************************************/
/// obj_pair_s

#define TYPEOF_obj_pair_s typeof( "obj_pair_s" )
typedef struct obj_pair_s
{
    union
    {
        obj_hdr_s hdr;
        struct
        {
            aware_t _;
            const spect_obj_s* p;
            properties_s prp;
        };
    };
    vd_t obj1;
    bl_t ext1; // valid area of obj1: true: outside obj2; false: inside obj2;
    vd_t obj2;
    bl_t ext2; // valid area of obj2: true: outside obj1; false: inside obj1;
} obj_pair_s;

static sc_t obj_pair_s_def =
"obj_pair_s = spect_obj"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
    "aware* obj1;"
    "bl_t   ext1;"
    "aware* obj2;"
    "bl_t   ext2;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_pair_s )

static void obj_pair_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; obj_pair_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
}

static bcore_flect_self_s* obj_pair_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_pair_s_def, sizeof( obj_pair_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_s_init_a,     "ap_t",          "init" );

//    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_s_projection, "projection_fp", "projection" );
//    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_s_normal,     "normal_fp",     "normal" );
//    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_s_outside,    "outside_fp",    "outside" );
//    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_s_fov,        "fov_fp",        "fov" );
//    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_s_fwd_hit,    "fwd_hit_fp",    "fwd_hit" );
//    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_s_is_in_fov,  "is_in_fov_fp",  "is_in_fov" );
//    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_s_move,       "move_fp",       "move" );
//    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_s_rotate,     "rotate_fp",     "rotate" );
//    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_s_scale,      "scale_fp",      "scale" );

    return self;
}

/**********************************************************************************************************************/
/// compound_s

static sc_t compound_s_def =
"compound_s = bcore_inst"
"{"
    "aware_t _;"
    "aware * [] arr;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( compound_s )

void compound_s_clear( compound_s* o )
{
    bcore_array_aware_set_size( o, 0 );
}

vd_t compound_s_push( compound_s* o, tp_t type )
{
    sr_s sr = sr_create( type );
    bcore_array_aware_push( o, sr );
    return sr.o;
}

vd_t compound_s_push_q( compound_s* o, const sr_s* object )
{
    if( !object ) return NULL;
    tp_t type = sr_s_type( object );
    vd_t dst = compound_s_push( o, type );
    bcore_inst_typed_copy( type, dst, object->o );
    return dst;
}

f3_t compound_s_fwd_hit( const compound_s* o, const ray_s* r, vc_t* hit_obj )
{
    f3_t min_a = f3_inf;
    for( sz_t i = 0; i < o->size; i++ )
    {
        f3_t a = obj_fwd_hit( o->data[ i ], r );
        if( a < min_a )
        {
            min_a = a;
            if( hit_obj ) *hit_obj = o->data[ i ];
        }
    }
    return min_a;
}

f3_t compound_s_idx_fwd_hit( const compound_s* o, const bcore_arr_sz_s* idx_arr, const ray_s* r, vc_t* hit_obj )
{
    f3_t min_a = f3_inf;
    for( sz_t i = 0; i < idx_arr->size; i++ )
    {
        sz_t idx = idx_arr->data[ i ];
        f3_t a = obj_fwd_hit( o->data[ idx ], r );
        if( a < min_a )
        {
            min_a = a;
            if( hit_obj ) *hit_obj = o->data[ idx ];
        }
    }
    return min_a;
}

bcore_arr_sz_s* compound_s_in_fov_arr( const compound_s* o, const ray_cone_s* fov )
{
    bcore_arr_sz_s* arr = bcore_arr_sz_s_create();
    for( sz_t i = 0; i < o->size; i++ )
    {
        if( obj_is_in_fov( o->data[ i ], fov ) ) bcore_arr_sz_s_push( arr, i );
    }
    return arr;
}

static bcore_flect_self_s* compound_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( compound_s_def, sizeof( compound_s ) );
    return self;
}

/**********************************************************************************************************************/

vd_t objects_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "objects" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        bcore_trait_set( entypeof( "spect_obj" ), entypeof( "bcore_inst" ) );

        bcore_flect_define_creator( typeof( "properties_s"   ), properties_s_create_self );
        bcore_flect_define_creator( typeof( "spect_obj_s"    ), spect_obj_s_create_self  );
        bcore_flect_define_creator( typeof( "obj_plane_s"    ), obj_plane_s_create_self  );
        bcore_flect_define_creator( typeof( "obj_sphere_s"   ), obj_sphere_s_create_self );
        bcore_flect_define_creator( typeof( "obj_cylinder_s" ), obj_cylinder_s_create_self );
        bcore_flect_define_creator( typeof( "obj_cone_s"     ), obj_cone_s_create_self );
        bcore_flect_define_creator( typeof( "obj_pair_s"     ), obj_pair_s_create_self );
        bcore_flect_define_creator( typeof( "compound_s"     ), compound_s_create_self   );
    }

    return NULL;
}

/**********************************************************************************************************************/


