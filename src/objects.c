/** 3D Objects
 *  Author: Johannes Bernhard Steffens
 *
 *  Copyright (c) 2017 Johannes Bernhard Steffens. All rights reserved.
 */

#include <math.h>
#include "bcore_spect_inst.h"
#include "bcore_life.h"
#include "bcore_spect.h"
#include "bcore_spect_array.h"
#include "bcore_trait.h"

#include "textures.h"
#include "objects.h"

/**********************************************************************************************************************/
/// properties_s  (object's properties)

static sc_t properties_s_def =
"properties_s = bcore_inst"
"{"
    "v3d_s   pos;"
    "m3d_s   pax;"
    "aware * txm;"
    "f3_t    radiance;"
    "f3_t    n;"
"}";

void properties_s_init( properties_s* o )
{
    bcore_memzero( o, sizeof( *o ) );
    o->pos   = ( v3d_s ){ 0, 0, 0 };
    o->pax.x = ( v3d_s ){ 1, 0, 0 };
    o->pax.y = ( v3d_s ){ 0, 1, 0 };
    o->pax.z = ( v3d_s ){ 0, 0, 1 };
    o->n     = 1.0;
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
    o->pax = m3d_s_mlm( mat, &o->pax );
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

typedef v2d_s (*prj_fp )( vc_t o, v3d_s pos );
typedef v3d_s (*nor_fp )( vc_t o, v3d_s pos );
typedef f3_t  (*hit_fp )( vc_t o, const ray_s* ray );
typedef ray_cone_s (*fov_fp )( vc_t o, v3d_s pos );
typedef bl_t  (*is_in_fov_fp )( vc_t o, const ray_cone_s* fov );

typedef void (*move_fp)(   vd_t o, const v3d_s* vec );
typedef void (*rotate_fp)( vd_t o, const m3d_s* mat );
typedef void (*scale_fp)(  vd_t o, f3_t fac );

typedef struct spect_obj_s
{
    aware_t p_type;
    tp_t    o_type;
    prj_fp  fp_prj;
    nor_fp  fp_nor;
    fov_fp  fp_fov;
    hit_fp  fp_hit;

    move_fp   fp_move;
    rotate_fp fp_rotate;
    scale_fp  fp_scale;

    is_in_fov_fp fp_is_in_fov;
} spect_obj_s;

DEFINE_FUNCTIONS_OBJ_INST( spect_obj_s )

const spect_obj_s* obj_get_spect( vc_t o ) { return ( ( obj_hdr_s* )o )->p; }

static spect_obj_s* spect_obj_s_create_from_self( const bcore_flect_self_s* self )
{
    assert( self != NULL );
    spect_obj_s* o = spect_obj_s_create();
    o->o_type = self->type;
    o->fp_prj = ( prj_fp )bcore_flect_self_s_get_external_fp( self, bcore_name_enroll( "prj_fp" ), 0 );
    o->fp_nor = ( nor_fp )bcore_flect_self_s_get_external_fp( self, bcore_name_enroll( "nor_fp" ), 0 );
    o->fp_fov = ( fov_fp )bcore_flect_self_s_get_external_fp( self, bcore_name_enroll( "fov_fp" ), 0 );
    o->fp_hit = ( hit_fp )bcore_flect_self_s_get_external_fp( self, bcore_name_enroll( "hit_fp" ), 0 );
    o->fp_is_in_fov = ( is_in_fov_fp )bcore_flect_self_s_get_external_fp( self, bcore_name_enroll( "is_in_fov_fp" ), 0 );

    o->fp_move   = ( move_fp   )bcore_flect_self_s_get_external_fp( self, bcore_name_enroll( "move_fp"   ), 0 );
    o->fp_rotate = ( rotate_fp )bcore_flect_self_s_get_external_fp( self, bcore_name_enroll( "rotate_fp" ), 0 );
    o->fp_scale  = ( scale_fp  )bcore_flect_self_s_get_external_fp( self, bcore_name_enroll( "scale_fp"  ), 0 );

    return o;
}

v2d_s      obj_prj( vc_t o, v3d_s pos ) { return obj_get_spect( o )->fp_prj( o, pos ); }
v3d_s      obj_nor( vc_t o, v3d_s pos ) { return obj_get_spect( o )->fp_nor( o, pos ); }
ray_cone_s obj_fov( vc_t o, v3d_s pos ) { return obj_get_spect( o )->fp_fov( o, pos ); }
bl_t       obj_is_in_fov( vc_t o, const ray_cone_s* fov ) { return obj_get_spect( o )->fp_is_in_fov( o, fov ); }
f3_t       obj_hit( vc_t o, const ray_s* ray ) { return obj_get_spect( o )->fp_hit( o, ray ); }
f3_t       obj_radiance( vc_t o ) { return ( ( obj_hdr_s* )o )->prp.radiance; }

void obj_move(   vd_t o, const v3d_s* vec ) { obj_get_spect( o )->fp_move(   o, vec ); }
void obj_rotate( vd_t o, const m3d_s* mat ) { obj_get_spect( o )->fp_rotate( o, mat ); }
void obj_scale(  vd_t o, f3_t fac         ) { obj_get_spect( o )->fp_scale(  o, fac ); }

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
    nc_l->o->prp.txm = txm_plain_s_create();
}

v2d_s obj_plane_s_prj( const obj_plane_s* o, v3d_s pos )
{
    v3d_s p = v3d_s_sub( pos, o->prp.pos );
    return ( v2d_s ) { v3d_s_mlv( p, o->prp.pax.x ), v3d_s_mlv( p, o->prp.pax.y ) };
}

v3d_s obj_plane_s_nor( const obj_plane_s* o, v3d_s pos ) { return o->prp.pax.z; }

ray_cone_s obj_plane_s_fov( const obj_plane_s* o, v3d_s pos )
{
    ray_cone_s cne;
    cne.ray.p = pos;
    cne.ray.d = v3d_s_neg( o->prp.pax.z );
    cne.cos_rs = v3d_s_mlv( v3d_s_sub( o->prp.pos, pos ), cne.ray.d ) > 0 ? 0 : 1;
    return cne;
}

f3_t obj_plane_s_hit( const obj_plane_s* o, const ray_s* r )
{
    f3_t div = v3d_s_mlv( o->prp.pax.z, r->d );
    if( div >= 0 ) return f3_inf; // plane can only be hit from the positive surface area
    f3_t offset = v3d_s_sub_mlv( o->prp.pos, r->p, o->prp.pax.z ) / div;
    return ( offset > 0 ) ? offset : f3_inf;
}

bl_t obj_plane_s_is_in_fov( const obj_plane_s* o, const ray_cone_s* fov )
{
    if( obj_plane_s_hit( o, &fov->ray ) < f3_inf ) return true;
    f3_t sin_a = v3d_s_mlv( o->prp.pax.z, fov->ray.d );
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
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_init_a, "ap_t", "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_prj, "prj_fp", "prj" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_nor, "nor_fp", "nor" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_fov, "fov_fp", "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_hit, "hit_fp", "hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_is_in_fov, "is_in_fov_fp", "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_move,   "move_fp",   "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_rotate, "rotate_fp", "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_scale,  "scale_fp",  "scale" );
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
    nc_l->o->prp.txm = txm_plain_s_create();
}

v2d_s obj_sphere_s_prj( const obj_sphere_s* o, v3d_s pos )
{
    v3d_s r = v3d_s_of_length( v3d_s_sub( pos, o->prp.pos ), 1.0 );
    f3_t x = v3d_s_mlv( r, o->prp.pax.x );
    f3_t y = v3d_s_mlv( r, v3d_s_mlx( o->prp.pax.z, o->prp.pax.x ) );
    f3_t z = v3d_s_mlv( r, o->prp.pax.z );

    f3_t azimuth = atan2( x, y );

    /// correct rounding errors
    z = z >  1.0 ?  1.0 : z;
    z = z < -1.0 ? -1.0 : z;
    f3_t elevation = asin( z );

    return ( v2d_s ) { azimuth, elevation };
}

v3d_s obj_sphere_s_nor( const obj_sphere_s* o, v3d_s pos ) { return v3d_s_of_length( v3d_s_sub( pos, o->prp.pos ), 1.0 ); }

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

f3_t obj_sphere_s_hit( const obj_sphere_s* o, const ray_s* r )
{
    v3d_s p = v3d_s_sub( r->p, o->prp.pos );
    f3_t _p = v3d_s_mlv( p, r->d );
    f3_t q = ( v3d_s_sqr( p ) - ( o->radius * o->radius ) );
    f3_t v = _p * _p;
    if( v < q ) return f3_inf; // missing the sphere
    if( q < 0 ) return f3_inf; // inside the sphere
    f3_t offset = -_p - sqrt( v - q );

    return ( offset > 0 ) ? offset : f3_inf;
}

void obj_sphere_s_move(   obj_sphere_s* o, const v3d_s* vec ) { properties_s_move  ( &o->prp, vec ); }
void obj_sphere_s_rotate( obj_sphere_s* o, const m3d_s* mat ) { properties_s_rotate( &o->prp, mat ); }
void obj_sphere_s_scale(  obj_sphere_s* o, f3_t fac         ) { properties_s_scale ( &o->prp, fac ); o->radius *= fac; }

static bcore_flect_self_s* obj_sphere_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_sphere_s_def, sizeof( obj_sphere_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_init_a, "ap_t", "init" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_prj, "prj_fp", "prj" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_nor, "nor_fp", "nor" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_fov, "fov_fp", "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_hit, "hit_fp", "hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_is_in_fov, "is_in_fov_fp", "is_in_fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_move,   "move_fp",   "move" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_rotate, "rotate_fp", "rotate" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_scale,  "scale_fp",  "scale" );
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

f3_t compound_s_hit( const compound_s* o, const ray_s* r, vc_t* hit_obj )
{
    f3_t min_a = f3_inf;
    for( sz_t i = 0; i < o->size; i++ )
    {
        f3_t a = obj_hit( o->data[ i ], r );
        if( a < min_a )
        {
            min_a = a;
            if( hit_obj ) *hit_obj = o->data[ i ];
        }
    }
    return min_a;
}

f3_t compound_s_idx_hit( const compound_s* o, const bcore_arr_sz_s* idx_arr, const ray_s* r, vc_t* hit_obj )
{
    f3_t min_a = f3_inf;
    for( sz_t i = 0; i < idx_arr->size; i++ )
    {
        sz_t idx = idx_arr->data[ i ];
        f3_t a = obj_hit( o->data[ idx ], r );
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

        bcore_flect_define_creator( typeof( "properties_s" ), properties_s_create_self );
        bcore_flect_define_creator( typeof( "spect_obj_s"  ), spect_obj_s_create_self  );
        bcore_flect_define_creator( typeof( "obj_plane_s"  ), obj_plane_s_create_self  );
        bcore_flect_define_creator( typeof( "obj_sphere_s" ), obj_sphere_s_create_self );
        bcore_flect_define_creator( typeof( "compound_s"   ), compound_s_create_self   );
    }

    return NULL;
}

/**********************************************************************************************************************/


