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
#include "quicktypes.h"
#include "distance.h"
#include "container.h"

/**********************************************************************************************************************/
/// envelope_s  (sphere used to define object boundaries)
static sc_t envelope_s_def =
"envelope_s = bcore_inst"
"{"
    "v3d_s pos;"
    "f3_t radius;"
"}";

DEFINE_FUNCTIONS_OBJ_FLAT( envelope_s )
DEFINE_CREATE_SELF( envelope_s, envelope_s_def )

void envelope_s_move( envelope_s* o, const v3d_s* vec )
{
    v3d_s_o_add( &o->pos, *vec );
}

void envelope_s_rotate( envelope_s* o, const m3d_s* mat )
{
    o->pos = m3d_s_mlv( mat, o->pos );
}

void envelope_s_scale( envelope_s* o, f3_t fac )
{
    v3d_s_o_mlf( &o->pos, fac );
    o->radius *= fac;
}

bl_t envelope_s_is_in_fov( const envelope_s* o, const ray_cone_s* fov )
{
    return sphere_is_in_fov( o->pos, o->radius, fov );
}

bl_t envelope_s_is_reachable( const envelope_s* o, const ray_s* ray_field, f3_t length )
{
    return sphere_intersects_half_sphere( o->pos, o->radius, ray_field, length );
}

ray_cone_s envelope_s_fov( const envelope_s* o, v3d_s pos )
{
    ray_cone_s cne;
    v3d_s diff = v3d_s_sub( o->pos, pos );
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

bl_t envelope_s_ray_hits( const envelope_s* o, const ray_s* r )
{
    return sphere_ray_hit( o->pos, o->radius, r, NULL ) < f3_inf;
}

f3_t envelope_s_ray_hit( const envelope_s* o, const ray_s* r )
{
    return sphere_ray_hit( o->pos, o->radius, r, NULL );
}

s3_t envelope_s_side( const envelope_s* o, v3d_s pos )
{
    return sphere_observer_side( o->pos, o->radius, pos );
}

envelope_s envelope_create( v3d_s pos, f3_t radius )
{
    envelope_s env;
    env.pos = pos;
    env.radius = radius;
    return env;
}

envelope_s envelope_of_pair( const envelope_s* env1, const envelope_s* env2 )
{
    f3_t r1 = env1->radius;
    f3_t r2 = env2->radius;
    v3d_s diff = v3d_s_sub( env1->pos, env2->pos );
    f3_t d = sqrt( v3d_s_sqr( diff ) );

    f3_t rmax = r1 > r2 ? r1 : r2;
    f3_t rmin = r1 < r2 ? r1 : r2;

    if( rmin + d <= rmax ) // the smaller envelope is completely inside the bigger one
    {
        return r1 > r2 ? *env1 : *env2;
    }
    else
    {
        v3d_s p1 = v3d_s_add( env1->pos, v3d_s_of_length( diff, r1 ) );
        v3d_s p2 = v3d_s_sub( env2->pos, v3d_s_of_length( diff, r2 ) );
        envelope_s env;
        env.pos = v3d_s_mlf( v3d_s_add( p1, p2 ), 0.5 );
        env.radius = ( r1 + r2 + d ) * 0.5;
        return env;
    }
}

/**********************************************************************************************************************/
/// properties_s  (object's properties)

static sc_t properties_s_def =
"properties_s = bcore_inst"
"{"
    "v3d_s   pos;"
    "m3d_s   rax;"
    "aware * texture_field;"
    "cl_s    color;"

    "f3_t    radiance;"
    "f3_t    refractive_index;"

    "f3_t fresnel_reflectivity;"   // incoming energy taken by fresnel reflection
    "f3_t chromatic_reflectivity;" // residual energy taken chromatic (specular) reflection
    "f3_t diffuse_reflectivity;"   // residual energy taken by diffuse reflection
    "f3_t sigma;"                  // sigma of Oren-Nayar reflectance model

    "cl_s transparency;"           // residual energy taken by material transition

    "envelope_s* envelope;" // optional envelope
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
    o->diffuse_reflectivity = 1.0;
    o->sigma = 0.0;
    o->fresnel_reflectivity = 1.0;
}

DEFINE_FUNCTION_COPY_INST( properties_s )
DEFINE_FUNCTION_DOWN_INST( properties_s )
DEFINE_FUNCTIONS_CDC( properties_s )

void properties_s_move( properties_s* o, const v3d_s* vec )
{
    v3d_s_o_add( &o->pos, *vec );
    if( o->envelope ) envelope_s_move( o->envelope, vec );
}

void properties_s_rotate( properties_s* o, const m3d_s* mat )
{
    o->rax = m3d_s_mlm( mat, &o->rax );
    o->pos = m3d_s_mlv( mat, o->pos );
    if( o->envelope ) envelope_s_rotate( o->envelope, mat );
}

void properties_s_scale( properties_s* o, f3_t fac )
{
    v3d_s_o_mlf( &o->pos, fac );
    if( o->envelope ) envelope_s_scale( o->envelope, fac );
}

static bcore_flect_self_s* properties_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( properties_s_def, sizeof( properties_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )properties_s_init, "bcore_fp_init", "init" );
    return self;
}

/**********************************************************************************************************************/
/// spect_obj_s

typedef v2d_s      (*projection_fp   )( vc_t o, v3d_s pos );
typedef f3_t       (*ray_hit_fp      )( vc_t o, const ray_s* ray, v3d_s* p_nor );
typedef s2_t       (*side_fp         )( vc_t o, v3d_s pos );
typedef ray_cone_s (*fov_fp          )( vc_t o, v3d_s pos );
typedef bl_t       (*is_in_fov_fp    )( vc_t o, const ray_cone_s* fov );
typedef bl_t       (*is_reachable_fp )( vc_t o, const ray_s* ray, f3_t length );

typedef void (*move_fp  )( vd_t o, const v3d_s* vec );
typedef void (*rotate_fp)( vd_t o, const m3d_s* mat );
typedef void (*scale_fp )( vd_t o, f3_t fac );

typedef struct spect_obj_s
{
    aware_t p_type;
    tp_t    o_type;

    projection_fp fp_projection;
    fov_fp        fp_fov;
    ray_hit_fp    fp_ray_hit;
    side_fp       fp_side;

    move_fp       fp_move;
    rotate_fp     fp_rotate;
    scale_fp      fp_scale;

    is_in_fov_fp    fp_is_in_fov;
    is_reachable_fp fp_is_reachable;
} spect_obj_s;

DEFINE_FUNCTIONS_OBJ_INST( spect_obj_s )

const spect_obj_s* obj_get_spect( vc_t o ) { return ( ( obj_hdr_s* )o )->p; }

static spect_obj_s* spect_obj_s_create_from_self( const bcore_flect_self_s* self )
{
    assert( self != NULL );
    spect_obj_s* o = spect_obj_s_create();
    o->o_type = self->type;
    o->fp_projection   = ( projection_fp   )bcore_flect_self_s_try_external_fp( self, entypeof( "projection_fp"   ), 0 );
    o->fp_fov          = ( fov_fp          )bcore_flect_self_s_try_external_fp( self, entypeof( "fov_fp"          ), 0 );
    o->fp_ray_hit      = ( ray_hit_fp      )bcore_flect_self_s_get_external_fp( self, entypeof( "ray_hit_fp"      ), 0 );
    o->fp_side         = ( side_fp         )bcore_flect_self_s_get_external_fp( self, entypeof( "side_fp"         ), 0 );
    o->fp_is_in_fov    = ( is_in_fov_fp    )bcore_flect_self_s_try_external_fp( self, entypeof( "is_in_fov_fp"    ), 0 );
    o->fp_is_reachable = ( is_reachable_fp )bcore_flect_self_s_try_external_fp( self, entypeof( "is_reachable_fp" ), 0 );
    o->fp_move         = ( move_fp         )bcore_flect_self_s_get_external_fp( self, entypeof( "move_fp"         ), 0 );
    o->fp_rotate       = ( rotate_fp       )bcore_flect_self_s_get_external_fp( self, entypeof( "rotate_fp"       ), 0 );
    o->fp_scale        = ( scale_fp        )bcore_flect_self_s_get_external_fp( self, entypeof( "scale_fp"        ), 0 );
    return o;
}

v2d_s obj_projection( vc_t o, v3d_s pos )
{
    const obj_hdr_s* hdr = o;
    if( !hdr->p->fp_projection ) ERR_fa( "Object '#<sc_t>' has no projection-function", ifnameof( *(aware_t*)o ) );
    return hdr->p->fp_projection( o, pos );
}

ray_cone_s obj_fov( vc_t o, v3d_s pos )
{
    const obj_hdr_s* hdr = o;
//    if( hdr->prp.envelope ) return envelope_s_fov( hdr->prp.envelope, pos );

    if( !hdr->p->fp_fov ) ERR_fa( "Object '#<sc_t>' has no fov-function", ifnameof( *(aware_t*)o ) );
    return hdr->p->fp_fov( o, pos );
}

f3_t obj_ray_hit( vc_t o, const ray_s* ray, v3d_s* p_nor )
{
    const obj_hdr_s* hdr = o;
    if( hdr->prp.envelope && !envelope_s_ray_hits( hdr->prp.envelope, ray ) ) return f3_inf;
    return hdr->p->fp_ray_hit( o, ray, p_nor );
}

f3_t obj_ray_exit( vc_t o, const ray_s* ray, v3d_s* p_nor )
{
    v3d_s nor;
    f3_t a = obj_ray_hit( o, ray, &nor );
    if( a >= f3_inf ) return f3_inf;
    ray_s ray_l = *ray;
    f3_t sum = 0;
    while( a < f3_inf )
    {
        a += f3_eps * 2;
        sum += a;
        ray_l.p = ray_s_pos( &ray_l, a );
        a = obj_ray_hit( o, &ray_l, &nor );
    }

    if( v3d_s_mlv( nor, ray->d ) > 0 )
    {
       if( p_nor ) *p_nor = nor;
       return sum;
    }
    else
    {
       return f3_inf;
    }
}

envelope_s obj_estimate_envelope( vc_t o, sz_t samples, u2_t rseed, f3_t radius_factor )
{
    const obj_hdr_s* hdr = o;

    tp_t pos_arr_type = bcore_flect_type_parse_sc( "{ v3d_s [] arr; }" );
    const bcore_array_s* pos_arr_spect = bcore_array_s_get_typed( pos_arr_type );
    struct { v3d_s* data; sz_t size; sz_t space; } * pos_arr = bcore_inst_typed_create( pos_arr_type );

    v3d_s sum = v3d_s_zero();

    u2_t rv = rseed;
    ray_s ray;
    ray.p = hdr->prp.pos;
    for( sz_t i = 0; i < samples; i++ )
    {
        ray.d = v3d_s_random_sphere_belt( &rv, 1.0 );
        f3_t a = obj_ray_exit( o, &ray, NULL );
        if( a < f3_inf )
        {
            v3d_s pos = ray_s_pos( &ray, a );
            bcore_array_spect_push( pos_arr_spect, pos_arr, sr_twc( TYPEOF_v3d_s, &pos ) );
            sum = v3d_s_add( sum, ray_s_pos( &ray, a ) );
            ray.p = v3d_s_mlf( sum, ( 1.0 / pos_arr->size ) );

            /// add a little noise to the position in case it sits exactly on a surface
            ray.p.x += f3_eps * f3_rnd0( &rv );
            ray.p.y += f3_eps * f3_rnd0( &rv );
            ray.p.z += f3_eps * f3_rnd0( &rv );
        }
    }

    envelope_s env;
    env.pos = ray.p;
    env.radius = f3_mag;

    if( pos_arr->size > 0 )
    {
        f3_t max_r2 = 0;
        for( sz_t i = 0; i < pos_arr->size; i++ )
        {
            v3d_s pos = pos_arr->data[ i ];
            f3_t r = v3d_s_diff_sqr( ray.p, pos );
            max_r2 = r > max_r2 ? r : max_r2;
        }

        env.radius = sqrt( max_r2 ) * radius_factor;
    }

    bcore_inst_typed_discard( pos_arr_type, pos_arr );

    return env;
}

s2_t obj_side( vc_t o, v3d_s pos )
{
    const obj_hdr_s* hdr = o;
    if( hdr->prp.envelope && envelope_s_side( hdr->prp.envelope, pos ) == 1 ) return 1;
    return hdr->p->fp_side( o, pos );
}

bl_t obj_is_in_fov( vc_t o, const ray_cone_s* fov )
{
    const obj_hdr_s* hdr = o;
    if( hdr->prp.envelope && !envelope_s_is_in_fov( hdr->prp.envelope, fov ) ) return false;
    if( hdr->p->fp_is_in_fov ) return hdr->p->fp_is_in_fov( o, fov );
    return true;
}

bl_t obj_is_reachable( vc_t o, const ray_s* ray_field, f3_t length )
{
    const obj_hdr_s* hdr = o;
    if( hdr->prp.envelope && !envelope_s_is_reachable( hdr->prp.envelope, ray_field, length ) ) return false;
    if( hdr->p->fp_is_reachable ) return hdr->p->fp_is_reachable( o, ray_field, length );
    return true;
}

f3_t obj_radiance( vc_t o )
{
    return ( ( obj_hdr_s* )o )->prp.radiance;
}

void obj_move( vd_t o, const v3d_s* vec )
{
    const obj_hdr_s* hdr = o;
    hdr->p->fp_move( o, vec );
}

void obj_rotate( vd_t o, const m3d_s* mat )
{
    const obj_hdr_s* hdr = o;
    hdr->p->fp_rotate( o, mat );
}

void obj_scale( vd_t o, f3_t fac )
{
    const obj_hdr_s* hdr = o;
    hdr->p->fp_scale( o, fac );
}

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

void obj_set_transparency( vd_t obj, cl_s color )
{
    obj_hdr_s* o = obj;
    o->prp.transparency = color;
}

void obj_set_refractive_index( vd_t obj, f3_t refractive_index )
{
    obj_hdr_s* o = obj;
    o->prp.refractive_index = refractive_index;
    if( refractive_index == 1.0 )
    {
        o->prp.fresnel_reflectivity = 0.0;
    }
    else
    {
        o->prp.fresnel_reflectivity = 1.0;
    }
}

void obj_set_radiance( vd_t obj, f3_t radiance )
{
    obj_hdr_s* o = obj;
    o->prp.radiance = radiance;
}

void obj_set_texture_field( vd_t obj, vc_t texture_field )
{
    obj_hdr_s* o = obj;
    bcore_inst_aware_discard( o->prp.texture_field );
    o->prp.texture_field = bcore_inst_aware_clone( texture_field );
}

void obj_set_envelope( vd_t obj, const envelope_s* env )
{
    obj_hdr_s* o = obj;
    if( o->prp.envelope ) envelope_s_discard( o->prp.envelope );
    o->prp.envelope = envelope_s_clone( env );
}

void obj_set_auto_envelope( vd_t obj )
{
    envelope_s env = obj_estimate_envelope( obj, 1000, 123, 1.1 );
    obj_hdr_s* o = obj;
    if( o->prp.envelope ) envelope_s_discard( o->prp.envelope );
    o->prp.envelope = envelope_s_clone( &env );
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

ray_cone_s obj_plane_s_fov( const obj_plane_s* o, v3d_s pos )
{
    ray_cone_s cne;
    cne.ray.p = pos;
    cne.ray.d = v3d_s_neg( o->prp.rax.z );
    cne.cos_rs = v3d_s_mlv( v3d_s_sub( o->prp.pos, pos ), cne.ray.d ) > 0 ? 0 : 1;
    return cne;
}

f3_t obj_plane_s_ray_hit( const obj_plane_s* o, const ray_s* r, v3d_s* p_nor )
{
    return plane_ray_hit( o->prp.pos, o->prp.rax.z, r, p_nor );
}

s2_t obj_plane_s_side( const obj_plane_s* o, v3d_s pos )
{
    return plane_observer_side( o->prp.pos, o->prp.rax.z, pos );
}

bl_t obj_plane_s_is_in_fov( const obj_plane_s* o, const ray_cone_s* fov )
{
    if( o->prp.envelope ) return envelope_s_is_in_fov( o->prp.envelope, fov );
    if( obj_plane_s_ray_hit( o, &fov->ray, NULL ) < f3_inf ) return true;
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
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_fov,        "fov_fp",        "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_ray_hit,    "ray_hit_fp",    "ray_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_side,       "side_fp",       "side" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_is_in_fov,  "is_in_fov_fp",  "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_move,       "move_fp",       "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_rotate,     "rotate_fp",     "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_scale,      "scale_fp",      "scale" );
    return self;
}

/**********************************************************************************************************************/
/// obj_sphere_s

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

void obj_sphere_s_set_radius( obj_sphere_s* o, f3_t radius )
{
    o->radius = radius;
}

f3_t obj_sphere_s_get_radius( const obj_sphere_s* o )
{
    return o->radius;
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
    return sphere_is_in_fov( o->prp.pos, o->radius, fov );
}

bl_t obj_sphere_s_is_reachable( const obj_sphere_s* o, const ray_s* ray_field, f3_t length )
{
    return sphere_intersects_half_sphere( o->prp.pos, o->radius, ray_field, length );
}

f3_t obj_sphere_s_ray_hit( const obj_sphere_s* o, const ray_s* r, v3d_s* p_nor )
{
    return sphere_ray_hit( o->prp.pos, o->radius, r, p_nor );
}

s2_t obj_sphere_s_side( const obj_sphere_s* o, v3d_s pos )
{
    return sphere_observer_side( o->prp.pos, o->radius, pos );
}

void obj_sphere_s_move(   obj_sphere_s* o, const v3d_s* vec ) { properties_s_move  ( &o->prp, vec ); }
void obj_sphere_s_rotate( obj_sphere_s* o, const m3d_s* mat ) { properties_s_rotate( &o->prp, mat ); }
void obj_sphere_s_scale(  obj_sphere_s* o, f3_t fac         ) { properties_s_scale ( &o->prp, fac ); o->radius *= fac; }

static bcore_flect_self_s* obj_sphere_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_sphere_s_def, sizeof( obj_sphere_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_init_a,       "ap_t",          "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_projection,   "projection_fp", "projection" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_fov,          "fov_fp",        "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_ray_hit,      "ray_hit_fp",    "ray_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_side,         "side_fp",       "side" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_is_in_fov,    "is_in_fov_fp",  "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_is_reachable, "is_reachable_fp", "is_reachable" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_move,         "move_fp",       "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_rotate,       "rotate_fp",     "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_scale,        "scale_fp",      "scale" );
    return self;
}

/**********************************************************************************************************************/
/** obj_squaroid_s
 *  A surface with distance function a*x^2 + b*y^2 + c*z^2 + r = 0;
 *  Many basic surfaces like sphere, ellipsoid, hyperboloid, cone, plane, etc are special cases of the squaroid
 */

typedef struct obj_squaroid_s
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

    f3_t a, b, c, r;
} obj_squaroid_s;

static sc_t obj_squaroid_s_def =
"obj_squaroid_s = spect_obj"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
    "f3_t a =  1.0;"
    "f3_t b =  1.0;"
    "f3_t c =  1.0;"
    "f3_t r = -1.0;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_squaroid_s )

static void obj_squaroid_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; obj_squaroid_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
}

void obj_squaroid_s_set_param( obj_squaroid_s* o, f3_t a, f3_t b, f3_t c, f3_t r )
{
    o->a = a;
    o->b = b;
    o->c = c;
    o->r = r;
}

obj_squaroid_s* obj_squaroid_s_create_squaroid( f3_t a, f3_t b, f3_t c, f3_t r )
{
    obj_squaroid_s* o = obj_squaroid_s_create();
    o->a = a;
    o->b = b;
    o->c = c;
    o->r = r;
    return o;
}

obj_squaroid_s* obj_squaroid_s_create_ellipsoid( f3_t rx, f3_t ry, f3_t rz )
{
    obj_squaroid_s* o = obj_squaroid_s_create();
    o->a = ( rx != 0 ) ? 1.0 / f3_sqr( rx ) : 1.0;
    o->b = ( ry != 0 ) ? 1.0 / f3_sqr( ry ) : 1.0;
    o->c = ( rz != 0 ) ? 1.0 / f3_sqr( rz ) : 1.0;
    o->r = -1;
    f3_t rmax = rx > ry ? rx : ry;
    rmax = rmax > rz ? rmax : rz;
    envelope_s env = envelope_create( v3d_s_zero(), rmax + 2 * f3_eps );
    obj_set_envelope( o, &env );
    return o;
}

obj_squaroid_s* obj_squaroid_s_create_hyperboloid1( f3_t rx, f3_t ry, f3_t rz )
{
    obj_squaroid_s* o = obj_squaroid_s_create();
    o->a =    ( rx != 0 ) ? 1.0 / f3_sqr( rx ) : 1.0;
    o->b =    ( ry != 0 ) ? 1.0 / f3_sqr( ry ) : 1.0;
    o->c = -( ( rz != 0 ) ? 1.0 / f3_sqr( rz ) : 1.0 );
    o->r = -1;
    return o;
}

obj_squaroid_s* obj_squaroid_s_create_hyperboloid2( f3_t rx, f3_t ry, f3_t rz )
{
    obj_squaroid_s* o = obj_squaroid_s_create();
    o->a =    ( rx != 0 ) ? 1.0 / f3_sqr( rx ) : 1.0;
    o->b =    ( ry != 0 ) ? 1.0 / f3_sqr( ry ) : 1.0;
    o->c = -( ( rz != 0 ) ? 1.0 / f3_sqr( rz ) : 1.0 );
    o->r =  1;
    return o;
}

obj_squaroid_s* obj_squaroid_s_create_cone( f3_t rx, f3_t ry, f3_t rz )
{
    obj_squaroid_s* o = obj_squaroid_s_create();
    o->a =    ( rx != 0 ) ? 1.0 / f3_sqr( rx ) : 1.0;
    o->b =    ( ry != 0 ) ? 1.0 / f3_sqr( ry ) : 1.0;
    o->c = -( ( rz != 0 ) ? 1.0 / f3_sqr( rz ) : 1.0 );
    o->r = 0;
    return o;
}

obj_squaroid_s* obj_squaroid_s_create_cylinder( f3_t rx, f3_t ry )
{
    obj_squaroid_s* o = obj_squaroid_s_create();
    o->a =    ( rx != 0 ) ? 1.0 / f3_sqr( rx ) : 1.0;
    o->b =    ( ry != 0 ) ? 1.0 / f3_sqr( ry ) : 1.0;
    o->c =  0;
    o->r = -1;
    return o;
}

f3_t obj_squaroid_s_ray_hit( const obj_squaroid_s* o, const ray_s* r, v3d_s* p_nor )
{
    v3d_s p = m3d_s_mlv( &o->prp.rax, v3d_s_sub( r->p, o->prp.pos ) );
    v3d_s d = m3d_s_mlv( &o->prp.rax, r->d );

    f3_t f  = o->a * d.x * d.x + o->b * d.y * d.y + o->c * d.z * d.z;
    f3_t fs = o->a * d.x * p.x + o->b * d.y * p.y + o->c * d.z * p.z;
    f3_t fq = o->a * p.x * p.x + o->b * p.y * p.y + o->c * p.z * p.z + o->r;
    f3_t a = f3_inf;

    if( f != 0 )
    {
        f3_t f_inv = 1.0 / f;
        f3_t s = fs * f_inv;
        f3_t q = fq * f_inv;
        f3_t r = s * s - q;
        if( r < 0 )  return f3_inf; // missing object
        r = sqrt( r );
        a = -s - r;
        if( a < 0 ) a = -s + r;
        if( a < 0 ) a = f3_inf;
    }
    else
    {
        a = ( fq != 0 ) ? -fs / ( 2 * fq ) : f3_inf;
    }

    if( a == f3_inf ) return f3_inf;

    if( p_nor )
    {
        f3_t x = p.x + a * d.x;
        f3_t y = p.y + a * d.y;
        f3_t z = p.z + a * d.z;

        v3d_s n1;
        n1.x = x * o->a;
        n1.y = y * o->b;
        n1.z = z * o->c;
        *p_nor = v3d_s_of_length( m3d_s_tmlv( &o->prp.rax, n1 ), 1.0 );
    }

    return a - f3_eps;
}

s2_t obj_squaroid_s_side( const obj_squaroid_s* o, v3d_s pos )
{
    v3d_s p = m3d_s_mlv( &o->prp.rax, v3d_s_sub( pos, o->prp.pos ) );
    return ( o->a * p.x * p.x + o->b * p.y * p.y + o->c * p.z * p.z + o->r ) > 0  ? 1 : -1;
}

void obj_squaroid_s_move(   obj_squaroid_s* o, const v3d_s* vec ) { properties_s_move  ( &o->prp, vec ); }
void obj_squaroid_s_rotate( obj_squaroid_s* o, const m3d_s* mat ) { properties_s_rotate( &o->prp, mat ); }

void obj_squaroid_s_scale(  obj_squaroid_s* o, f3_t fac         ) { properties_s_scale ( &o->prp, fac ); o->r *= f3_sqr( fac ); }

static bcore_flect_self_s* obj_squaroid_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_squaroid_s_def, sizeof( obj_squaroid_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_squaroid_s_init_a,     "ap_t",          "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_squaroid_s_ray_hit,    "ray_hit_fp",    "ray_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_squaroid_s_side,       "side_fp",       "side" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_squaroid_s_move,       "move_fp",       "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_squaroid_s_rotate,     "rotate_fp",     "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_squaroid_s_scale,      "scale_fp",      "scale" );
    return self;
}

/**********************************************************************************************************************/
/// obj_distance_s  (object based on distance function)

typedef struct obj_distance_s
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
    f3_t inv_scale;
    sz_t cycles;
    vd_t distance;
} obj_distance_s;

static sc_t obj_distance_s_def =
"obj_distance_s = spect_obj"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
    "f3_t inv_scale = 1.0;"
    "sz_t cycles = 200;"
    "aware* distance;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_distance_s )

void obj_distance_s_set_distance( obj_distance_s* o, vc_t distance )
{
    o->distance = bcore_inst_aware_clone( distance );
}

void obj_distance_s_set_cycles( obj_distance_s* o, sz_t cycles )
{
    o->cycles = cycles;
}

obj_distance_s* obj_distance_s_create_distance( vc_t distance, envelope_s* envelope )
{
    obj_distance_s* o = obj_distance_s_create();
    o->prp.envelope = envelope;
    o->distance = bcore_inst_aware_clone( distance );
    return o;
}

static void obj_distance_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; obj_distance_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
}

v2d_s obj_distance_s_projection( const obj_distance_s* o, v3d_s pos )
{
    return ( v2d_s ) { 0, 0 };
}

bl_t obj_distance_s_is_in_fov( const obj_distance_s* o, const ray_cone_s* fov )
{
    if( o->prp.envelope ) return envelope_s_is_in_fov( o->prp.envelope, fov );
    return true;
}

f3_t obj_distance_s_ray_hit( const obj_distance_s* o, const ray_s* r, v3d_s* p_nor )
{
    ray_s ray = *r;
    f3_t offs0 = 0;
    if( o->prp.envelope )
    {
        if( envelope_s_side( o->prp.envelope, r->p ) == 1 )
        {
            offs0 = envelope_s_ray_hit( o->prp.envelope, &ray );
            if( offs0 >= f3_inf ) return f3_inf;
            ray.p = ray_s_pos( &ray, offs0 );
        }
    }

    ray.p = v3d_s_mlf( m3d_s_mlv( &o->prp.rax, v3d_s_sub( ray.p, o->prp.pos ) ), o->inv_scale );
    ray.d = m3d_s_mlv( &o->prp.rax, ray.d );

    f3_t offs1 = 0;
    f3_t dist = distance( o->distance, ray.p );

    if( dist > 0 )
    {
        for( sz_t i = 0; i < o->cycles; i++ )
        {
            offs1 += dist + f3_eps;
            dist = distance( o->distance, ray_s_pos( &ray, offs1 ) );
            if( dist < 0 || dist > f3_mag ) break;
        }
    }
    else
    {
        for( sz_t i = 0; i < o->cycles; i++ )
        {
            offs1 -= dist - f3_eps;
            dist = distance( o->distance, ray_s_pos( &ray, offs1 ) );
            if( dist > 0 || dist < -f3_mag ) break;
        }
    }

    if( f3_abs( dist ) <= f3_eps )
    {
        // we compute p_nor by taking the (approximate) gradient from the distance field
        if( p_nor )
        {
            v3d_s p = ray_s_pos( &ray, offs1 );
            f3_t d0 = distance( o->distance, p );
            v3d_s n;
            n.x = ( distance( o->distance, ( v3d_s ){ p.x + f3_eps, p.y, p.z } ) - d0 ) / f3_eps;
            n.y = ( distance( o->distance, ( v3d_s ){ p.x, p.y + f3_eps, p.z } ) - d0 ) / f3_eps;
            n.z = ( distance( o->distance, ( v3d_s ){ p.x, p.y, p.z + f3_eps } ) - d0 ) / f3_eps;
            *p_nor = v3d_s_of_length( m3d_s_tmlv( &o->prp.rax, n ), 1.0 );
        }

        return offs0 + ( offs1 / o->inv_scale ) - f3_eps;
    }
    return f3_inf;
}

s2_t obj_distance_s_side( const obj_distance_s* o, v3d_s pos )
{
    if( o->prp.envelope && envelope_s_side( o->prp.envelope, pos ) == 1 ) return 1;
    v3d_s p = v3d_s_mlf( m3d_s_mlv( &o->prp.rax, v3d_s_sub( pos, o->prp.pos ) ), o->inv_scale );
    return distance( o->distance, p ) > 0 ? 1 : -1;
}

void obj_distance_s_move(   obj_distance_s* o, const v3d_s* vec ) { properties_s_move  ( &o->prp, vec ); }
void obj_distance_s_rotate( obj_distance_s* o, const m3d_s* mat ) { properties_s_rotate( &o->prp, mat ); }
void obj_distance_s_scale(  obj_distance_s* o, f3_t fac         ) { properties_s_scale ( &o->prp, fac ); o->inv_scale *= 1.0 / fac; }

static bcore_flect_self_s* obj_distance_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_distance_s_def, sizeof( obj_distance_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_distance_s_init_a,       "ap_t",          "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_distance_s_projection,   "projection_fp", "projection" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_distance_s_ray_hit,      "ray_hit_fp",    "ray_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_distance_s_side,         "side_fp",       "side" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_distance_s_is_in_fov,    "is_in_fov_fp",  "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_distance_s_move,         "move_fp",       "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_distance_s_rotate,       "rotate_fp",     "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_distance_s_scale,        "scale_fp",      "scale" );
    return self;
}

/**********************************************************************************************************************/
/// obj_pair_inside_s  (combining two objects mutual inside area)

typedef struct obj_pair_inside_s
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
    vd_t o1;
    vd_t o2;
} obj_pair_inside_s;

static sc_t obj_pair_inside_s_def =
"obj_pair_inside_s = spect_obj"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
    "aware* o1;"
    "aware* o2;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_pair_inside_s )

obj_pair_inside_s* obj_pair_inside_s_create_pair( vc_t o1, vc_t o2 )
{
    obj_pair_inside_s* o = obj_pair_inside_s_create();
    properties_s_copy( &o->prp, &( ( obj_hdr_s* )o1 )->prp );
    o->o1 = bcore_inst_aware_clone( o1 );
    o->o2 = bcore_inst_aware_clone( o2 );
    return o;
}

sr_s obj_pair_inside_s_create_pair_sr( sr_s o1, sr_s o2 )
{
    tp_t t1 = sr_s_type( &o1 );
    tp_t t2 = sr_s_type( &o2 );
    if( !bcore_trait_is_of( t1, TYPEOF_spect_obj ) || !bcore_trait_is_of( t2, TYPEOF_spect_obj ) )
    {
        ERR( "Objects '#<sc_t>' and '#<sc_t>' cannot be composed.", ifnameof( t1 ), ifnameof( t2 ) );
    }
    sr_s ret = sr_tsd( TYPEOF_obj_pair_inside_s, obj_pair_inside_s_create_pair( o1.o, o2.o ) );
    sr_down( o1 );
    sr_down( o2 );

    return ret;
}

static void obj_pair_inside_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; obj_pair_inside_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
}

ray_cone_s obj_pair_inside_s_fov( const obj_pair_inside_s* o, v3d_s pos )
{
    if( o->prp.envelope ) return envelope_s_fov( o->prp.envelope, pos );
    ray_cone_s cne;
    v3d_s diff = v3d_s_sub( o->prp.pos, pos );
    cne.ray.d = v3d_s_of_length( diff, 1.0 );
    cne.ray.p = pos;
    cne.cos_rs = 0;
    return cne;
}

bl_t obj_pair_inside_s_is_in_fov( const obj_pair_inside_s* o, const ray_cone_s* fov )
{
    if( o->prp.envelope ) envelope_s_is_in_fov( o->prp.envelope, fov );
    return obj_is_in_fov( o->o1, fov ) || obj_is_in_fov( o->o2, fov );
}

f3_t obj_pair_inside_s_ray_hit( const obj_pair_inside_s* o, const ray_s* r, v3d_s* p_nor )
{
    v3d_s n1, n2;
    f3_t a1 = obj_ray_hit( o->o1, r, &n1 );
    f3_t a2 = obj_ray_hit( o->o2, r, &n2 );
    if( a1 < a2 && obj_side( o->o2, ray_s_pos( r, a1 ) ) == -1 )
    {
        if( p_nor ) *p_nor = n1;
        return a1;
    }

    if( a2 >= f3_inf ) return f3_inf;

    if( obj_side( o->o1, ray_s_pos( r, a2 ) ) == -1 )
    {
        if( p_nor ) *p_nor = n2;
        return a2;
    }

    f3_t offs = a2;
    ray_s ray;
    ray.d = r->d;
    ray.p = ray_s_pos( r, offs );
    vc_t obj1 = o->o1;
    vc_t obj2 = o->o2;

    while( offs < f3_inf )
    {
        f3_t a = obj_ray_hit( obj1, &ray, &n1 );
        if( a >= f3_inf ) return f3_inf;
        if( obj_side( obj2, ray_s_pos( &ray, a ) ) == -1 )
        {
            if( p_nor ) *p_nor = n1;
            return offs + a;
        }
        offs += a + 2 * f3_eps;
        ray.p = ray_s_pos( r, offs );
        vc_t tmp = obj2;
        obj2 = obj1;
        obj1 = tmp;
    }
    return f3_inf;
}

s2_t obj_pair_inside_s_side( const obj_pair_inside_s* o, v3d_s pos )
{
    return ( obj_side( o->o1, pos ) + obj_side( o->o2, pos ) == -2 ) ? -1 : 1;
}

void obj_pair_inside_s_move(   obj_pair_inside_s* o, const v3d_s* vec )
{
    properties_s_move  ( &o->prp, vec );
    obj_move( o->o1, vec );
    obj_move( o->o2, vec );
}

void obj_pair_inside_s_rotate( obj_pair_inside_s* o, const m3d_s* mat )
{
    properties_s_rotate( &o->prp, mat );
    obj_rotate( o->o1, mat );
    obj_rotate( o->o2, mat );
}

void obj_pair_inside_s_scale( obj_pair_inside_s* o, f3_t fac )
{
    properties_s_scale ( &o->prp, fac );
    obj_scale( o->o1, fac );
    obj_scale( o->o2, fac );
}

static bcore_flect_self_s* obj_pair_inside_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_pair_inside_s_def, sizeof( obj_pair_inside_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_inside_s_init_a,       "ap_t",            "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_inside_s_fov,          "fov_fp",          "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_inside_s_ray_hit,      "ray_hit_fp",      "ray_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_inside_s_side,         "side_fp",         "side" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_inside_s_is_in_fov,    "is_in_fov_fp",    "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_inside_s_move,         "move_fp",         "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_inside_s_rotate,       "rotate_fp",       "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_inside_s_scale,        "scale_fp",        "scale" );
    return self;
}

/**********************************************************************************************************************/
/// obj_pair_outside_s  (combining two objects mutual outside area)

typedef struct obj_pair_outside_s
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
    vd_t o1;
    vd_t o2;
} obj_pair_outside_s;

static sc_t obj_pair_outside_s_def =
"obj_pair_outside_s = spect_obj"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
    "aware* o1;"
    "aware* o2;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_pair_outside_s )

obj_pair_outside_s* obj_pair_outside_s_create_pair( vc_t o1, vc_t o2 )
{
    obj_pair_outside_s* o = obj_pair_outside_s_create();
    properties_s_copy( &o->prp, &( ( obj_hdr_s* )o1 )->prp );

    o->o1 = bcore_inst_aware_clone( o1 );
    o->o2 = bcore_inst_aware_clone( o2 );

    if( o->prp.envelope ) // discard envelope because o2 is outside o1 (true envelope would be bigger)
    {
        envelope_s_discard( o->prp.envelope );
        o->prp.envelope = NULL;
    }

    return o;
}

sr_s obj_pair_outside_s_create_pair_sr( sr_s o1, sr_s o2 )
{
    tp_t t1 = sr_s_type( &o1 );
    tp_t t2 = sr_s_type( &o2 );
    if( !bcore_trait_is_of( t1, TYPEOF_spect_obj ) || !bcore_trait_is_of( t2, TYPEOF_spect_obj ) )
    {
        ERR( "Objects '#<sc_t>' and '#<sc_t>' cannot be composed.", ifnameof( t1 ), ifnameof( t2 ) );
    }
    sr_s ret = sr_tsd( TYPEOF_obj_pair_outside_s, obj_pair_outside_s_create_pair( o1.o, o2.o ) );
    sr_down( o1 );
    sr_down( o2 );
    return ret;
}

static void obj_pair_outside_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; obj_pair_outside_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
}

ray_cone_s obj_pair_outside_s_fov( const obj_pair_outside_s* o, v3d_s pos )
{
    if( o->prp.envelope ) return envelope_s_fov( o->prp.envelope, pos );
    ray_cone_s cne;
    v3d_s diff = v3d_s_sub( o->prp.pos, pos );
    cne.ray.d = v3d_s_of_length( diff, 1.0 );
    cne.ray.p = pos;
    cne.cos_rs = 0;
    return cne;
}

bl_t obj_pair_outside_s_is_in_fov( const obj_pair_outside_s* o, const ray_cone_s* fov )
{
    return obj_is_in_fov( o->o1, fov ) || obj_is_in_fov( o->o2, fov );
    if( o->prp.envelope ) envelope_s_is_in_fov( o->prp.envelope, fov );
}

f3_t obj_pair_outside_s_ray_hit( const obj_pair_outside_s* o, const ray_s* r, v3d_s* p_nor )
{
    v3d_s n1, n2;
    f3_t a1 = obj_ray_hit( o->o1, r, &n1 );
    f3_t a2 = obj_ray_hit( o->o2, r, &n2 );
    if( a1 < a2 && obj_side( o->o2, ray_s_pos( r, a1 ) ) == 1 )
    {
        if( p_nor ) *p_nor = n1;
        return a1;
    }

    if( a2 >= f3_inf ) return f3_inf;

    if( obj_side( o->o1, ray_s_pos( r, a2 ) ) == 1 )
    {
        if( p_nor ) *p_nor = n2;
        return a2;
    }

    f3_t offs = a2;
    ray_s ray;
    ray.d = r->d;
    ray.p = ray_s_pos( r, offs );
    vc_t obj1 = o->o1;
    vc_t obj2 = o->o2;

    while( offs < f3_inf )
    {
        f3_t a = obj_ray_hit( obj1, &ray, &n1 );
        if( a >= f3_inf ) return f3_inf;
        if( obj_side( obj2, ray_s_pos( &ray, a ) ) == 1 )
        {
            if( p_nor ) *p_nor = n1;
            return offs + a;
        }
        offs += a + 2 * f3_eps;
        ray.p = ray_s_pos( r, offs );
        vc_t tmp = obj2;
        obj2 = obj1;
        obj1 = tmp;
    }
    return f3_inf;
}

s2_t obj_pair_outside_s_side( const obj_pair_outside_s* o, v3d_s pos )
{
    return ( obj_side( o->o1, pos ) + obj_side( o->o2, pos ) == 2 ) ? 1 : -1;
}

void obj_pair_outside_s_move(   obj_pair_outside_s* o, const v3d_s* vec )
{
    properties_s_move  ( &o->prp, vec );
    obj_move( o->o1, vec );
    obj_move( o->o2, vec );
}

void obj_pair_outside_s_rotate( obj_pair_outside_s* o, const m3d_s* mat )
{
    properties_s_rotate( &o->prp, mat );
    obj_rotate( o->o1, mat );
    obj_rotate( o->o2, mat );
}

void obj_pair_outside_s_scale(  obj_pair_outside_s* o, f3_t fac )
{
    properties_s_scale ( &o->prp, fac );
    obj_scale( o->o1, fac );
    obj_scale( o->o2, fac );
}

static bcore_flect_self_s* obj_pair_outside_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_pair_outside_s_def, sizeof( obj_pair_outside_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_outside_s_init_a,       "ap_t",            "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_outside_s_fov,          "fov_fp",          "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_outside_s_ray_hit,      "ray_hit_fp",      "ray_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_outside_s_side,         "side_fp",         "side" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_outside_s_is_in_fov,    "is_in_fov_fp",    "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_outside_s_move,         "move_fp",         "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_outside_s_rotate,       "rotate_fp",       "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_pair_outside_s_scale,        "scale_fp",        "scale" );
    return self;
}

/**********************************************************************************************************************/
/// obj_neg_s  (negated object inside <-> outside)

typedef struct obj_neg_s
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
    vd_t o1;
} obj_neg_s;

static sc_t obj_neg_s_def =
"obj_neg_s = spect_obj"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
    "aware* o1;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_neg_s )

obj_neg_s* obj_neg_s_create_neg( vc_t o1 )
{
    obj_neg_s* o = obj_neg_s_create();
    properties_s_copy( &o->prp, &( ( obj_hdr_s* )o1 )->prp );
    o->o1 = bcore_inst_aware_clone( o1 );
    return o;
}

static void obj_neg_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; obj_neg_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
}

bl_t obj_neg_s_is_in_fov( const obj_neg_s* o, const ray_cone_s* fov )
{
    if( o->prp.envelope ) return envelope_s_is_in_fov( o->prp.envelope, fov );
    return obj_is_in_fov( o->o1, fov );
}

f3_t obj_neg_s_ray_hit( const obj_neg_s* o, const ray_s* r, v3d_s* p_nor )
{
    v3d_s n1;
    f3_t a1 = obj_ray_hit( o->o1, r, &n1 );
    if( a1 < f3_inf )
    {
        if( p_nor ) *p_nor = v3d_s_neg( n1 );
        return a1;
    }
    return f3_inf;
}

s2_t obj_neg_s_side( const obj_neg_s* o, v3d_s pos )
{
    return -1 * obj_side( o->o1, pos );
}

void obj_neg_s_move(   obj_neg_s* o, const v3d_s* vec ) { properties_s_move  ( &o->prp, vec ); obj_move(   o->o1, vec ); }
void obj_neg_s_rotate( obj_neg_s* o, const m3d_s* mat ) { properties_s_rotate( &o->prp, mat ); obj_rotate( o->o1, mat ); }
void obj_neg_s_scale(  obj_neg_s* o, f3_t fac         ) { properties_s_scale ( &o->prp, fac ); obj_scale(  o->o1, fac ); }

static bcore_flect_self_s* obj_neg_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_neg_s_def, sizeof( obj_neg_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_neg_s_init_a,     "ap_t",          "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_neg_s_ray_hit,    "ray_hit_fp",    "ray_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_neg_s_side,       "side_fp",       "side" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_neg_s_is_in_fov,  "is_in_fov_fp",  "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_neg_s_move,       "move_fp",       "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_neg_s_rotate,     "rotate_fp",     "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_neg_s_scale,      "scale_fp",      "scale" );
    return self;
}

/**********************************************************************************************************************/
/// obj_scale_s  (scales object independently in directions)

typedef struct obj_scale_s
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
    v3d_s inv_scale;
    vd_t o1;
} obj_scale_s;

static sc_t obj_scale_s_def =
"obj_scale_s = spect_obj"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
    "v3d_s inv_scale;"
    "aware* o1;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_scale_s )

obj_scale_s* obj_scale_s_create_scale( vc_t o1, v3d_s scale )
{
    obj_scale_s* o = obj_scale_s_create();
    properties_s_copy( &o->prp, &( ( obj_hdr_s* )o1 )->prp );

    o->prp.pos = v3d_s_zero();
    o->prp.rax = m3d_s_ident();

    if( o->prp.envelope )
    {
        o->prp.envelope->pos = v3d_s_mld( o->prp.envelope->pos, scale );
        o->prp.envelope->radius *= v3d_s_max( scale );
    }

    o->o1 = bcore_inst_aware_clone( o1 );
    o->inv_scale.x = ( scale.x != 0 ) ? ( 1.0 / scale.x ) : 1.0;
    o->inv_scale.y = ( scale.y != 0 ) ? ( 1.0 / scale.y ) : 1.0;
    o->inv_scale.z = ( scale.z != 0 ) ? ( 1.0 / scale.z ) : 1.0;
    return o;
}

static void obj_scale_s_init_a( vd_t nc )
{
    struct { ap_t a; vc_t p; obj_scale_s* o; } * nc_l = nc;
    nc_l->a( nc ); // default
    nc_l->o->inv_scale.x = 1.0;
    nc_l->o->inv_scale.y = 1.0;
    nc_l->o->inv_scale.z = 1.0;
}

f3_t obj_scale_s_ray_hit( const obj_scale_s* o, const ray_s* r, v3d_s* p_nor )
{
    ray_s ray;
    ray.p = v3d_s_mld( m3d_s_mlv( &o->prp.rax, v3d_s_sub( r->p, o->prp.pos ) ), o->inv_scale );
    ray.d = v3d_s_mld( m3d_s_mlv( &o->prp.rax, r->d ), o->inv_scale );

    f3_t d_length = sqrt( v3d_s_sqr( ray.d ) );
    f3_t d_factor = ( d_length > 0 ) ? ( 1.0 / d_length ) : 0;
    ray.d = v3d_s_mlf( ray.d, d_factor );

    v3d_s n1;
    f3_t a1 = obj_ray_hit( o->o1, &ray, &n1 );
    if( a1 < f3_inf )
    {
        n1 = v3d_s_mld( n1, o->inv_scale );
        if( p_nor ) *p_nor = v3d_s_of_length( m3d_s_tmlv( &o->prp.rax, n1 ), 1.0 );
        return a1 * d_factor;
    }
    return f3_inf;
}

s2_t obj_scale_s_side( const obj_scale_s* o, v3d_s pos )
{
    return obj_side( o->o1, v3d_s_mld( pos, o->inv_scale ) );
}

void obj_scale_s_move( obj_scale_s* o, const v3d_s* vec )
{
    properties_s_move  ( &o->prp, vec );
}

void obj_scale_s_rotate( obj_scale_s* o, const m3d_s* mat )
{
    properties_s_rotate( &o->prp, mat );
}

void obj_scale_s_scale(  obj_scale_s* o, f3_t fac )
{
    properties_s_scale ( &o->prp, fac );
    o->inv_scale = v3d_s_mlf( o->inv_scale, ( fac != 0 ) ? 1.0 / fac : 1.0 );
}

static bcore_flect_self_s* obj_scale_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_scale_s_def, sizeof( obj_scale_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_scale_s_init_a,     "ap_t",          "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_scale_s_ray_hit,    "ray_hit_fp",    "ray_hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_scale_s_side,       "side_fp",       "side" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_scale_s_move,       "move_fp",       "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_scale_s_rotate,     "rotate_fp",     "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_scale_s_scale,      "scale_fp",      "scale" );
    return self;
}

/**********************************************************************************************************************/

sr_s obj_meval_key( sr_s* sr_o, meval_s* ev, tp_t key )
{
    assert( bcore_trait_is_of( sr_s_type( sr_o ), TYPEOF_spect_obj ) );
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
    else if( key == TYPEOF_set_transparency )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj_set_transparency( sr_o->o, meval_s_eval_v3d( ev ) );
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
    else if( key == TYPEOF_set_texture_field )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        sr_s v = meval_s_eval_texture_field( ev );
        obj_set_texture_field( sr_o->o, v.o );
        sr_down( v );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == typeof( "set_envelope" ) )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        sr_s v = meval_s_eval( ev, sr_null() );
        if( sr_s_type( &v ) == TYPEOF_envelope_s )
        {
            obj_set_envelope( sr_o->o, ( const envelope_s* )v.o );
        }
        else if( sr_s_type( &v ) == TYPEOF_obj_sphere_s )
        {
            envelope_s env;
            env.pos = ( ( obj_sphere_s* )v.o )->prp.pos;
            env.radius = ( ( obj_sphere_s* )v.o )->radius;
            obj_set_envelope( sr_o->o, &env );
        }
        else
        {
            meval_s_err_fa( ev, "Object '#<sc_t>' cannot be used as envelope (use a sphere).", ifnameof( sr_s_type( &v ) ) );
        }

        sr_down( v );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == typeof( "set_auto_envelope" ) )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
        obj_set_auto_envelope( sr_o->o );
    }
    else if( key == typeof( "set_fresnel_reflectivity" ) )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj_hdr_s* hdr = sr_o->o;
        hdr->prp.fresnel_reflectivity = meval_s_eval_f3( ev );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == typeof( "set_chromatic_reflectivity" ) )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj_hdr_s* hdr = sr_o->o;
        hdr->prp.chromatic_reflectivity = meval_s_eval_f3( ev );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == typeof( "set_diffuse_reflectivity" ) )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj_hdr_s* hdr = sr_o->o;
        hdr->prp.diffuse_reflectivity = meval_s_eval_f3( ev );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == typeof( "set_sigma" ) )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        obj_hdr_s* hdr = sr_o->o;
        hdr->prp.sigma = meval_s_eval_f3( ev );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == typeof( "set_material" ) )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        sr_s v = meval_s_eval( ev, sr_null() );
        if( sr_s_type( &v ) != TYPEOF_st_s ) meval_s_err_fa( ev, "set_surface: string-argument expected." );
        st_s* string = v.o;
        obj_hdr_s* hdr = sr_o->o;
        if( st_s_equal_sc( string, "transparent" ) )
        {
            hdr->prp.refractive_index = 1;
            hdr->prp.transparency = ( cl_s ) { 1, 1, 1 };
            hdr->prp.fresnel_reflectivity = 1;
            hdr->prp.chromatic_reflectivity = 0;
            hdr->prp.diffuse_reflectivity = 0;
        }
        else if( st_s_equal_sc( string, "glass" ) )
        {
            hdr->prp.refractive_index = 1.46; // fused silica
            hdr->prp.transparency = ( cl_s ) { 0.8, 0.9, 0.9 }; // transparency varies very strongly by glass type
            hdr->prp.fresnel_reflectivity = 1;
            hdr->prp.chromatic_reflectivity = 0;
            hdr->prp.diffuse_reflectivity = 0;
        }
        else if( st_s_equal_sc( string, "water" ) )
        {
            hdr->prp.refractive_index = 1.32;
            hdr->prp.transparency = ( cl_s ) { 0.5, 0.9, 0.99 }; // coarse approximation of absoption curve of water
            hdr->prp.fresnel_reflectivity = 1;
            hdr->prp.chromatic_reflectivity = 0;
            hdr->prp.diffuse_reflectivity = 0;
        }
        else if( st_s_equal_sc( string, "sapphire" ) )
        {
            hdr->prp.refractive_index = 1.76;
            hdr->prp.transparency = ( cl_s ) { 0.7, 0.7, 0.7 }; // TBD
            hdr->prp.fresnel_reflectivity = 1;
            hdr->prp.chromatic_reflectivity = 0;
            hdr->prp.diffuse_reflectivity = 0;
        }
        else if( st_s_equal_sc( string, "diamond" ) )
        {
            hdr->prp.refractive_index = 2.42;
            hdr->prp.transparency = ( cl_s ) { 0.8, 0.8, 0.8 }; // TBD
            hdr->prp.fresnel_reflectivity = 1;
            hdr->prp.chromatic_reflectivity = 0;
            hdr->prp.diffuse_reflectivity = 0;
        }
        else if( st_s_equal_sc( string, "diffuse" ) )
        {
            hdr->prp.refractive_index = 1;
            hdr->prp.transparency = ( cl_s ) { 0, 0, 0 };
            hdr->prp.fresnel_reflectivity = 0;
            hdr->prp.chromatic_reflectivity = 0;
            hdr->prp.diffuse_reflectivity = 1;
            hdr->prp.sigma = 0.29;
        }
        else if( st_s_equal_sc( string, "diffuse_polished" ) )
        {
            hdr->prp.refractive_index = 1.5;
            hdr->prp.transparency = ( cl_s ) { 0, 0, 0 };
            hdr->prp.fresnel_reflectivity = 1;
            hdr->prp.chromatic_reflectivity = 0;
            hdr->prp.diffuse_reflectivity = 1;
            hdr->prp.sigma = 0.29;
        }
        else if( st_s_equal_sc( string, "perfect_mirror" ) )
        {
            hdr->prp.refractive_index = 1;
            hdr->prp.transparency = ( cl_s ) { 0, 0, 0 };
            hdr->prp.color        = ( cl_s ) { 1, 1, 1 };
            hdr->prp.fresnel_reflectivity = 0;
            hdr->prp.chromatic_reflectivity = 1;
            hdr->prp.diffuse_reflectivity = 0;
        }
        else if( st_s_equal_sc( string, "mirror" ) )
        {
            hdr->prp.refractive_index = 1;
            hdr->prp.transparency = ( cl_s ) { 0, 0, 0 };
            hdr->prp.color        = ( cl_s ) { 0.92, 0.94, 0.87 };
            hdr->prp.fresnel_reflectivity = 0;
            hdr->prp.chromatic_reflectivity = 1;
            hdr->prp.diffuse_reflectivity = 0;
        }
        else if( st_s_equal_sc( string, "gold" ) )
        {
            hdr->prp.refractive_index = 1;
            hdr->prp.transparency = ( cl_s ) { 0, 0, 0 };
            hdr->prp.color        = ( cl_s ) { 0.83, 0.69, 0.22 };
            hdr->prp.fresnel_reflectivity = 0;
            hdr->prp.chromatic_reflectivity = 1;
            hdr->prp.diffuse_reflectivity = 0;
        }
        else if( st_s_equal_sc( string, "silver" ) )
        {
            hdr->prp.refractive_index = 1;
            hdr->prp.transparency = ( cl_s ) { 0, 0, 0 };
            hdr->prp.color        = ( cl_s ) { 0.8, 0.8, 0.8 };
            hdr->prp.fresnel_reflectivity = 0;
            hdr->prp.chromatic_reflectivity = 1;
            hdr->prp.diffuse_reflectivity = 0;
        }
        else
        {
            meval_s_err_fa( ev, "set_surface: Unknown material specification '#<sc_t>.", string->sc );
        }

        sr_down( v );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else if( key == typeof( "set_distance_function" ) )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        if( sr_s_type( sr_o ) != TYPEOF_obj_distance_s ) meval_s_err_fa( ev, "Object '#<sc_t>' must be 'obj_distance_s'.", ifnameof( sr_s_type( sr_o ) ) );
        obj_distance_s* o = sr_o->o;

        sr_s v = meval_s_eval( ev, sr_null() );
        if( bcore_trait_is_of( sr_s_type( &v ), typeof( "distance" ) ) )
        {
            bcore_inst_aware_discard( o->distance );
            o->distance = bcore_inst_aware_clone( v.o );
        }
        else
        {
            meval_s_err_fa( ev, "Object '#<sc_t>' cannot be used as distance function.", ifnameof( sr_s_type( &v ) ) );
        }

        sr_down( v );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
    }
    else
    {
        meval_s_err_fa( ev, "Object has no member or function '#sc_t'.", meval_s_get_name( ev, key ) );
    }
    return sr_null();
}

/**********************************************************************************************************************/

vd_t objects_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "objects" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        bcore_trait_set( entypeof( "spect_obj" ), entypeof( "bcore_inst" ) );

        bcore_flect_define_creator( typeof( "envelope_s"         ), envelope_s_create_self );
        bcore_flect_define_creator( typeof( "properties_s"       ), properties_s_create_self );
        bcore_flect_define_creator( typeof( "spect_obj_s"        ), spect_obj_s_create_self  );
        bcore_flect_define_creator( typeof( "obj_plane_s"        ), obj_plane_s_create_self  );
        bcore_flect_define_creator( typeof( "obj_sphere_s"       ), obj_sphere_s_create_self );
        bcore_flect_define_creator( typeof( "obj_squaroid_s"     ), obj_squaroid_s_create_self );
        bcore_flect_define_creator( typeof( "obj_pair_inside_s"  ), obj_pair_inside_s_create_self  );
        bcore_flect_define_creator( typeof( "obj_pair_outside_s" ), obj_pair_outside_s_create_self );
        bcore_flect_define_creator( typeof( "obj_neg_s"          ), obj_neg_s_create_self  );
        bcore_flect_define_creator( typeof( "obj_scale_s"        ), obj_scale_s_create_self  );
        bcore_flect_define_creator( typeof( "obj_distance_s"     ), obj_distance_s_create_self  );
    }

    return NULL;
}

/**********************************************************************************************************************/


