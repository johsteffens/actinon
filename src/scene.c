/// Author & Copyright (C) 2017 Johannes Bernhard Steffens. All rights reserved.

#include <math.h>
#include "bcore_threads.h"
#include "scene.h"
#include "bcore_sinks.h"
#include "bcore_spect_inst.h"
#include "bcore_life.h"
#include "bcore_spect.h"
#include "bcore_spect_array.h"
#include "bcore_txt_ml.h"

/**********************************************************************************************************************/
/// reflectance

/// din: direction of incident ray
/// nor: surface normal
/// n: refractive index
/// cos_ai: cosine of incident angle == |incdent direction * surface normal|
f3_t get_reflectance( f3_t cos_ai, f3_t n )
{
    // ai: incident angle; at: transmission angle   (angle to surface normal)
    cos_ai = cos_ai > 1.0 ? 1.0 : cos_ai; // to prevent rounding errors
    f3_t sin_ai = sqrt( 1.0 - cos_ai * cos_ai );
    f3_t sin_at = sin_ai / n;
    f3_t cos_at = sqrt( 1.0 - sin_at * sin_at );
    f3_t rs = f3_sqr( ( cos_ai - n * cos_at ) / ( cos_ai + n * cos_at ) ); // perpendicular polarization
    f3_t rp = f3_sqr( ( cos_at - n * cos_ai ) / ( cos_at + n * cos_ai ) ); // parallel polarization

    // reflectance under evenly polarized light
    return ( rs + rp ) * 0.5;
}

/**********************************************************************************************************************/
/// photon_s

typedef struct photon_s { cl_s c; v3d_s p; } photon_s;
DEFINE_FUNCTIONS_OBJ_FLAT( photon_s )
DEFINE_CREATE_SELF( photon_s, "photon_s = bcore_inst { cl_s c; v3d_s p; }" )

/**********************************************************************************************************************/
/// photon_map_s

typedef struct photon_map_s
{
    aware_t _;
    union
    {
        bcore_static_array_s arr;
        struct
        {
            photon_s* data;
            sz_t size;
            sz_t space;
        };
    };
} photon_map_s;

DEFINE_FUNCTIONS_OBJ_FLAT( photon_map_s )
DEFINE_CREATE_SELF( photon_map_s, "photon_map_s = bcore_inst { aware_t _; photon_s [] arr; }" )

void photon_map_s_push( photon_map_s* o, photon_s photon )
{
    if( o->space <= o->size ) bcore_array_aware_set_space( o, o->size > 0 ? o->size * 2 : 1 );
    o->data[ o->size++ ] = photon;
}

/**********************************************************************************************************************/
/// spect_txm_s  (texture-map)

typedef cl_s (*clr_fp )( vc_t o, vc_t obj, v3d_s pos ); // converts position into color
v2d_s spect_obj_s_prj( vc_t o, v3d_s pos );

#define TYPEOF_spect_txm_s typeof( "spect_txm_s" )
typedef struct spect_txm_s
{
    aware_t p_type;
    tp_t    o_type;
    clr_fp  fp_clr;
} spect_txm_s;
DEFINE_FUNCTIONS_OBJ_INST( spect_txm_s )

/// common txm header
typedef struct txm_hdr_s
{
    aware_t _;
    const spect_txm_s* p;
} txm_hdr_s;

const spect_txm_s* txm_get_spect( vc_t o ) { return ( ( txm_hdr_s* )o )->p;     }

cl_s spect_txm_s_clr( vc_t o, vc_t obj, v3d_s pos )
{
    return txm_get_spect( o )->fp_clr( o, obj, pos );
}

static spect_txm_s* spect_txm_s_create_from_self( const bcore_flect_self_s* self )
{
    assert( self != NULL );
    spect_txm_s* o = spect_txm_s_create();
    o->o_type = self->type;
    o->fp_clr = ( clr_fp )bcore_flect_self_s_get_external_fp( self, bcore_name_enroll( "clr_fp" ), 0 );
    return o;
}

static bcore_flect_self_s* spect_txm_s_create_self( void )
{
    sc_t def = "spect_txm_s = spect { aware_t p_type; tp_t o_type; ... }";
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( def, sizeof( spect_txm_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )spect_txm_s_create_from_self, "bcore_spect_fp_create_from_self", "create_from_self" );
    return self;
}

/**********************************************************************************************************************/
/// txm_plain_s  (plain color texture map)

#define TYPEOF_txm_plain_s typeof( "txm_plain_s" )
typedef struct txm_plain_s
{
    aware_t _;
    const spect_txm_s* p;
    cl_s color;
} txm_plain_s;

static sc_t txm_plain_s_def =
"txm_plain_s = bcore_inst"
"{"
    "aware_t _;"
    "spect spect_txm_s* p;"
    "cl_s color;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( txm_plain_s )

cl_s txm_plain_s_clr( const txm_plain_s* o, vc_t obj, v3d_s pos )
{
    return o->color;
}

static bcore_flect_self_s* txm_plain_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( txm_plain_s_def, sizeof( txm_plain_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )txm_plain_s_clr, "clr_fp", "clr" );
    return self;
}

/**********************************************************************************************************************/
/// txm_chess_s  (chess color texture map)

#define TYPEOF_txm_chess_s typeof( "txm_chess_s" )
typedef struct txm_chess_s
{
    aware_t _;
    const spect_txm_s* p;
    cl_s color1;
    cl_s color2;
    f3_t scale;
} txm_chess_s;

static sc_t txm_chess_s_def =
"txm_chess_s = bcore_inst"
"{"
    "aware_t _;"
    "spect spect_txm_s* p;"
    "cl_s color1;"
    "cl_s color2;"
    "f3_t scale;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( txm_chess_s )

cl_s txm_chess_s_clr( const txm_chess_s* o, vc_t obj, v3d_s pos )
{
    v2d_s p = spect_obj_s_prj( obj, pos );
    s3_t x = llrint( p.x * o->scale );
    s3_t y = llrint( p.y * o->scale );
    return ( ( x ^ y ) & 1 ) ? o->color1 : o->color2;
}

static bcore_flect_self_s* txm_chess_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( txm_chess_s_def, sizeof( txm_chess_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )txm_chess_s_clr, "clr_fp", "clr" );
    return self;
}

/**********************************************************************************************************************/
/// properties_s  (object's properties)

#define TYPEOF_properties_s typeof( "properties_s" )
typedef struct properties_s
{
    v3d_s pos;      // position of origin
    m3d_s pax;      // object's principal axes
    vd_t  txm;      // texture map
    f3_t  radiance; // isotropic radiance (>0: object is active light source)
    f3_t  n;        // refractive index
} properties_s;

static sc_t properties_s_def =
"properties_s = bcore_inst"
"{"
    "v3d_s   pos;"
    "m3d_s   pax;"
    "aware * txm;"
    "f3_t    radiance;"
    "f3_t    n = 1.0;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( properties_s )
DEFINE_CREATE_SELF( properties_s, properties_s_def )

/**********************************************************************************************************************/
/// spect_obj_s

typedef v2d_s (*prj_fp )( vc_t o, v3d_s pos ); // converts position into projected coordinates
typedef v3d_s (*nor_fp )( vc_t o, v3d_s pos ); // calculates surface normal for position
typedef f3_t  (*hit_fp )( vc_t o, const ray_s* ray ); // returns ray-offset to hit-surface-point and object being hit
typedef ray_cone_s (*fov_fp )( vc_t o, v3d_s pos ); // computes the field of view of object from a given position
typedef bl_t  (*is_in_fov_fp )( vc_t o, const ray_cone_s* fov );


#define TYPEOF_spect_obj_s typeof( "spect_obj_s" )
typedef struct spect_obj_s
{
    aware_t p_type;
    tp_t    o_type;
    prj_fp  fp_prj;
    nor_fp  fp_nor;
    fov_fp  fp_fov;
    hit_fp  fp_hit;

    is_in_fov_fp fp_is_in_fov;
} spect_obj_s;

DEFINE_FUNCTIONS_OBJ_INST( spect_obj_s )

/// common object header
typedef struct obj_hdr_s
{
    aware_t _;
    const spect_obj_s* p;
    properties_s prp;
} obj_hdr_s;

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
    return o;
}

v2d_s spect_obj_s_prj( vc_t o, v3d_s pos ) { return obj_get_spect( o )->fp_prj( o, pos ); }
v3d_s spect_obj_s_nor( vc_t o, v3d_s pos ) { return obj_get_spect( o )->fp_nor( o, pos ); }
ray_cone_s spect_obj_s_fov( vc_t o, v3d_s pos ) { return obj_get_spect( o )->fp_fov( o, pos ); }
bl_t  spect_obj_s_is_in_fov( vc_t o, const ray_cone_s* fov ) { return obj_get_spect( o )->fp_is_in_fov( o, fov ); }

f3_t spect_obj_s_hit( vc_t o, const ray_s* ray )
{
    return obj_get_spect( o )->fp_hit( o, ray );
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
"obj_plane_s = bcore_inst"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_plane_s )

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

static bcore_flect_self_s* obj_plane_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_plane_s_def, sizeof( obj_plane_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_prj, "prj_fp", "prj" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_nor, "nor_fp", "nor" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_fov, "fov_fp", "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_hit, "hit_fp", "hit" );

    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_plane_s_is_in_fov, "is_in_fov_fp", "is_in_fov" );
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
"obj_sphere_s = bcore_inst"
"{"
    "aware_t _;"
    "spect spect_obj_s* p;"
    "properties_s prp;"
    "f3_t radius;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( obj_sphere_s )

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

/*
    f3_t sin_ang0 = sqrt( 1.0 - cos_ang0 * cos_ang0 );
    f3_t sin_ang1 = sqrt( 1.0 - cos_ang1 * cos_ang1 );
    return cos_ang0 * cos_ang1 + sin_ang0 * sin_ang1 > fov->cos_rs;
*/
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

static bcore_flect_self_s* obj_sphere_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( obj_sphere_s_def, sizeof( obj_sphere_s ) );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_prj, "prj_fp", "prj" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_nor, "nor_fp", "nor" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_fov, "fov_fp", "fov" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_hit, "hit_fp", "hit" );
    bcore_flect_self_s_push_ns_func( self, ( fp_t )obj_sphere_s_is_in_fov, "is_in_fov_fp", "is_in_fov" );
    return self;
}

/**********************************************************************************************************************/
/// image_cps_s

/// composite image format: 8bit rgb(a)
/// r = v & 0x0FF, g = ( v >> 8 ) & 0x0FF, b = ( v >> 16 ) & 0x0FF, a = ( v >> 24 ) & 0x0FF

#define TYPEOF_image_cps_s typeof( "image_cps_s" )
typedef struct image_cps_s
{
    aware_t _;
    sz_t w, h; // width, height,
    union
    {
        bcore_static_array_s arr;
        struct
        {
            u2_t* data;
            sz_t size, space;
        };
    };
} image_cps_s;

DECLARE_FUNCTIONS_OBJ( image_cps_s )

DEFINE_FUNCTIONS_OBJ_INST( image_cps_s )
DEFINE_CREATE_SELF( image_cps_s, "image_cps_s = bcore_inst { aware_t _; sz_t w; sz_t h; u2_t [] data; }" )

u2_t cps_from_rgb( u0_t r, u0_t g, u0_t b ) { return ( u2_t )r | ( ( u2_t )g ) << 8 | ( ( u2_t )b ) << 16; }
u0_t r_from_cps( u2_t v ) { return v;       }
u0_t g_from_cps( u2_t v ) { return v >>  8; }
u0_t b_from_cps( u2_t v ) { return v >> 16; }
u2_t cps_from_cl( cl_s cl )
{
    u0_t r = cl.x > 0.0 ? cl.x < 1.0 ? cl.x * 256 : 255 : 0;
    u0_t g = cl.y > 0.0 ? cl.y < 1.0 ? cl.y * 256 : 255 : 0;
    u0_t b = cl.z > 0.0 ? cl.z < 1.0 ? cl.z * 256 : 255 : 0;
    return cps_from_rgb( r, g, b );
}

void image_cps_s_set_size( image_cps_s* o, sz_t w, sz_t h, u2_t v )
{
    o->w = w;
    o->h = h;
    bcore_array_aware_set_size( o, w * h );
    for( sz_t i = 0; i < o->size; i++ ) o->data[ i ] = v;
}

void image_cps_s_set_pixel( image_cps_s* o, sz_t x, sz_t y, u2_t v )
{
    if( x < o->w && y < o->h ) o->data[ y * o->w + x ] = v;
}

void image_cps_s_set_pixel_cl( image_cps_s* o, sz_t x, sz_t y, cl_s cl )
{
    image_cps_s_set_pixel( o, x, y, cps_from_cl( cl ) );
}

void image_cps_s_copy_cl( image_cps_s* o, const image_cl_s* src )
{
    image_cps_s_set_size( o, src->w, src->h, 0 );
    for( sz_t i = 0; i < o->size; i++ ) o->data[ i ] = cps_from_cl( src->data[ i ] );
}

/**********************************************************************************************************************/

void image_cps_s_write_pnm( const image_cps_s* o, sc_t file )
{
    vd_t sink = bcore_sink_create_file( file );

    bcore_sink_aware_push_fa( sink, "P6\n#<sz_t> #<sz_t>\n255\n", o->w, o->h );
    for( sz_t i = 0; i < o->size; i++ )
    {
        u2_t v = o->data[ i ];
        u0_t c;
        c = r_from_cps( v ); bcore_sink_aware_push_data( sink, &c, 1 );
        c = g_from_cps( v ); bcore_sink_aware_push_data( sink, &c, 1 );
        c = b_from_cps( v ); bcore_sink_aware_push_data( sink, &c, 1 );
    }

    bcore_inst_aware_discard( sink );
}

tp_t image_cps_s_hash( const image_cps_s* o )
{
    tp_t hash = bcore_tp_init();
    for( sz_t i = 0; i < o->size; i++ ) hash = bcore_tp_fold_u2( hash, o->data[ i ] );
    return hash;
}

/**********************************************************************************************************************/
/// compound_s

#define TYPEOF_compound_s typeof( "compound_s" )
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

static sc_t compound_s_def =
"compound_s = bcore_inst"
"{"
    "aware_t _;"
    "aware * [] arr;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( compound_s )

vd_t compound_s_push( compound_s* o, tp_t type )
{
    sr_s sr = sr_create( type );
    bcore_array_aware_push( o, sr );
    return sr.o;
}

f3_t spect_obj_s_hit( vc_t o, const ray_s* ray );

f3_t compound_s_hit( const compound_s* o, const ray_s* r, vc_t* hit_obj )
{
    f3_t min_a = f3_inf;
    for( sz_t i = 0; i < o->size; i++ )
    {
        f3_t a = spect_obj_s_hit( o->data[ i ], r );
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
        f3_t a = spect_obj_s_hit( o->data[ idx ], r );
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
        if( spect_obj_s_is_in_fov( o->data[ i ], fov ) ) bcore_arr_sz_s_push( arr, i );
    }
    return arr;
}

static bcore_flect_self_s* compound_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( compound_s_def, sizeof( compound_s ) );
    return self;
}

/**********************************************************************************************************************/
/// scene_s

#define TYPEOF_scene_s typeof( "scene_s" )
typedef struct scene_s
{
    aware_t _;
    sz_t threads;
    sz_t image_width;
    sz_t image_height;
    f3_t gamma;
    cl_s background_color;

    v3d_s camera_position;
    v3d_s camera_view_direction;
    v3d_s camera_top_direction;
    f3_t  camera_focal_length;

    sz_t trace_depth;

    sz_t direct_samples;
    sz_t path_samples;
    sz_t photon_samples;
    f3_t photon_min_distance;

    compound_s light;  // light sources
    compound_s matter; // passive objects

    photon_map_s photon_map;
} scene_s;

static sc_t scene_s_def =
"scene_s = bcore_inst"
"{"
    "aware_t _;"
    "sz_t threads = 10;"
    "sz_t image_width = 800;"
    "sz_t image_height = 600;"
    "f3_t gamma = 1.0;"
    "cl_s background_color;"

    "v3d_s camera_position;"
    "v3d_s camera_view_direction;"
    "v3d_s camera_top_direction;"
    "f3_t  camera_focal_length = 1.0;"

    "sz_t trace_depth         = 11;"
    "sz_t direct_samples      = 100;"
    "sz_t path_samples        = 0;"  // requires trace_depth > 10
    "sz_t photon_samples      = 0;"
    "f3_t photon_min_distance = 0.05;"

    "compound_s light;"
    "compound_s matter;"
    "photon_map_s photon_map;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( scene_s )

static bcore_flect_self_s* scene_s_create_self( void )
{
    bcore_flect_self_s* self = bcore_flect_self_s_build_parse_sc( scene_s_def, sizeof( scene_s ) );
    return self;
}

f3_t scene_s_hit( const scene_s* o, const ray_s* r, vc_t* hit_obj, bl_t* is_light )
{
    f3_t min_a = f3_inf;
    f3_t a;
    vc_t hit_obj_l;

    if( ( a = compound_s_hit( &o->light, r, &hit_obj_l ) ) < min_a )
    {
        min_a = a;
        if( hit_obj ) *hit_obj = hit_obj_l;
        if( is_light ) *is_light = true;
    }

    if( ( a = compound_s_hit( &o->matter, r, &hit_obj_l ) ) < min_a )
    {
        min_a = a;
        if( hit_obj ) *hit_obj = hit_obj_l;
        if( is_light ) *is_light = false;
    }

    return min_a;
}

/// sends a photon and registers it in photon map
void scene_s_send_photon( scene_s* scene, const ray_s* ray, cl_s color, sz_t depth )
{
    if( depth == 0 ) return;
    obj_hdr_s* hit_obj;
    f3_t a = compound_s_hit( &scene->matter, ray, ( vc_t* )&hit_obj );
    if( a >= f3_inf ) return;
    v3d_s pos = ray_s_pos( ray, a );

    f3_t reflectance = 0;

    /// reflections
    if( hit_obj->prp.n > 1.0 )
    {
        v3d_s nor = spect_obj_s_nor( hit_obj, pos );
        ray_s out;
        out.p = pos;
        out.d = v3d_s_of_length( v3d_s_sub( ray->d, v3d_s_mlf( nor, 2.0 * v3d_s_mlv( ray->d, nor ) ) ), 1.0 );
        reflectance = get_reflectance( fabs( v3d_s_mlv( ray->d, nor ) ), hit_obj->prp.n );
        scene_s_send_photon( scene, &out, v3d_s_mlf( color, reflectance ), depth - 1 );
    }

    f3_t transmittance = 1.0 - reflectance; // only surface transmittance

    color = v3d_s_mlf( color, transmittance );

    if( v3d_s_sqr( color ) > 0 && hit_obj->prp.txm )
    {
        cl_s texture = spect_txm_s_clr( hit_obj->prp.txm, hit_obj, pos );
        color.x *= texture.x;
        color.y *= texture.y;
        color.z *= texture.z;
        photon_map_s_push( &scene->photon_map, ( photon_s ) { .c = color, .p = pos } );
    }
}

cl_s scene_s_lum( const scene_s* scene, const obj_hdr_s* obj, const ray_s* ray, f3_t offs, sz_t depth )
{
    cl_s lum = { 0, 0, 0 };
    if( depth == 0 ) return lum;

    v3d_s pos = ray_s_pos( ray, offs );

    if( obj->prp.radiance > 0 )
    {
        if( obj->prp.txm )
        {
            f3_t diff_sqr = v3d_s_diff_sqr( pos, obj->prp.pos );
            f3_t intensity = ( diff_sqr > 0 ) ? ( obj->prp.radiance / diff_sqr ) : f3_mag;
            lum = v3d_s_add( lum, v3d_s_mlf( spect_txm_s_clr( obj->prp.txm, obj, pos ), intensity ) );
        }
        return lum;
    }

    f3_t reflectance = 0;

    /// reflections
    if( obj->prp.n > 1.0 )
    {
        v3d_s nor = spect_obj_s_nor( obj, pos );
        ray_s out;
        out.p = pos;
        out.d = v3d_s_of_length( v3d_s_sub( ray->d, v3d_s_mlf( nor, 2.0 * v3d_s_mlv( ray->d, nor ) ) ), 1.0 );
        reflectance = get_reflectance( fabs( v3d_s_mlv( ray->d, nor ) ), obj->prp.n );
        vc_t hit_obj = NULL;
        f3_t a;
        if ( ( a = scene_s_hit( scene, &out, &hit_obj, NULL ) ) < f3_inf )
        {
            lum = v3d_s_mlf( scene_s_lum( scene, hit_obj, &out, a, depth - 1 ), reflectance );
        }
    }

    f3_t transmittance = 1.0 - reflectance; // only surface transmittance

    /// direct light
    if( scene->direct_samples > 0 )
    {
        ray_s surface = { .p = pos, .d = spect_obj_s_nor( obj, surface.p ) };

        /// random seed
        u2_t rv = surface.p.x * 32944792 + surface.p.y * 76403048 + surface.p.z * 24349373;

        /// peripheral light
        cl_s cl_per = { 0, 0, 0 };

        /// process sources with radiance directly  (light-sources)
        for( sz_t i = 0; i < scene->light.size; i++ )
        {
            cl_s cl_sum = { 0, 0, 0 };
            ray_s out = surface;
            obj_hdr_s* light_src = scene->light.data[ i ];
            ray_cone_s fov_to_src = spect_obj_s_fov( light_src, pos );
            m3d_s src_con = m3d_s_transposed( m3d_s_con_z( fov_to_src.ray.d ) );
            f3_t cyl_hgt = areal_coverage( fov_to_src.cos_rs );

            if( *( aware_t* )light_src->prp.txm != TYPEOF_txm_plain_s ) ERR( "Light sources with texture map are not yet supported." );
            cl_s color = v3d_s_mlf( ( ( txm_plain_s* )light_src->prp.txm )->color, light_src->prp.radiance );

            bcore_arr_sz_s* idx_arr = compound_s_in_fov_arr( &scene->matter, &fov_to_src );

            for( sz_t j = 0; j < scene->direct_samples; j++ )
            {
                out.d = m3d_s_mlv( &src_con, v3d_s_rsc( &rv, cyl_hgt ) );
                f3_t weight = v3d_s_mlv( out.d, surface.d );
                if( weight <= 0 ) continue;

                f3_t a = spect_obj_s_hit( light_src, &out );
                if( a >= f3_inf ) continue;
//                if( compound_s_hit( &scene->matter, &out, NULL ) > a ) cl_sum = v3d_s_add( cl_sum, v3d_s_mlf( color, weight ) );
                if( compound_s_idx_hit( &scene->matter, idx_arr, &out, NULL ) > a )
                {
                    v3d_s hit_pos = ray_s_pos( &out, a );

                    f3_t diff_sqr = v3d_s_diff_sqr( hit_pos, light_src->prp.pos );
                    f3_t intensity = ( diff_sqr > 0 ) ? ( light_src->prp.radiance / diff_sqr ) : f3_mag;

                    cl_sum = v3d_s_add( cl_sum, v3d_s_mlf( color, intensity * weight ) );
                }
            }

            bcore_arr_sz_s_discard( idx_arr );

            // factor 2 arises from the weight distribution across the half-sphere
            cl_per = v3d_s_add( cl_per, v3d_s_mlf( cl_sum, 2.0 * cyl_hgt / scene->direct_samples ) );

        }

        // path tracing
        if( scene->path_samples && depth > 10 )
        {
            // via path tracing
            cl_s cl_sum = { 0, 0, 0 };
            ray_s out = surface;
            m3d_s out_con = m3d_s_transposed( m3d_s_con_z( surface.d ) );
            for( sz_t i = 0; i < scene->path_samples; i++ )
            {
                out.d = m3d_s_mlv( &out_con, v3d_s_rsc( &rv, 1.0 ) );
                f3_t weight = v3d_s_mlv( out.d, surface.d );
                if( weight <= 0 ) continue;
                vc_t hit_obj = NULL;
                f3_t a = compound_s_hit( &scene->matter, &out, &hit_obj );
                if( a < f3_inf && hit_obj )
                {
                    cl_sum = v3d_s_add( cl_sum, v3d_s_mlf( scene_s_lum( scene, hit_obj, &out, a, depth - 10 ), weight ) );
                }
            }
            // factor 2 arises from the weight distribution across the half-sphere
            cl_per = v3d_s_add( cl_per, v3d_s_mlf( cl_sum, 2.0 / scene->path_samples ) );
        }
        // indirect light via photon map
        else if( scene->photon_map.size > 0 )
        {
            f3_t min_sqr_dist = f3_sqr( scene->photon_min_distance );

            cl_s cl_sum = { 0, 0, 0 };
            for( sz_t i = 0; i < scene->photon_map.size; i++ )
            {
                photon_s ph = scene->photon_map.data[ i ];
                ray_s out;
                out.p = ph.p;
                v3d_s diff = v3d_s_sub( pos, ph.p );
                f3_t diff_sqr = v3d_s_sqr( diff );

                if( diff_sqr < min_sqr_dist ) continue;

                out.d = v3d_s_of_length( diff, 1.0 );
                f3_t weight = -v3d_s_mlv( out.d, surface.d ) / diff_sqr;

                if( weight <= 0 ) continue;

                vc_t hit_obj = NULL;
                f3_t a = compound_s_hit( &scene->matter, &out, &hit_obj );

                if( a >= f3_inf || hit_obj == obj )
                {
                    cl_sum = v3d_s_add( cl_sum, v3d_s_mlf( ph.c, weight ) );
                }
            }
            cl_per = v3d_s_add( cl_per, v3d_s_mlf( cl_sum, 1.0 / scene->photon_samples ) );
        }

        cl_s texture = spect_txm_s_clr( obj->prp.txm, obj, pos );
        texture = v3d_s_mlf( texture, transmittance );

        cl_per.x *= texture.x;
        cl_per.y *= texture.y;
        cl_per.z *= texture.z;
        lum = v3d_s_add( lum, cl_per );
    }

    return lum;
}

/**********************************************************************************************************************/

void scene_s_create_photon_map( scene_s* scene )
{
    /// process sources with radiance directly  (light-sources)
    for( sz_t i = 0; i < scene->light.size; i++ )
    {
        obj_hdr_s* light_src = scene->light.data[ i ];
        if( *( aware_t* )light_src->prp.txm != TYPEOF_txm_plain_s ) ERR( "Light sources with texture map are not yet supported." );
        cl_s color = v3d_s_mlf( ( ( txm_plain_s* )light_src->prp.txm )->color, light_src->prp.radiance );
        ray_s out;
        out.p = light_src->prp.pos;
        u2_t rv = 1234;
        for( sz_t i = 0; i < scene->photon_samples; i++ )
        {
            out.d = v3d_s_rsc( &rv, 2.0 );
            scene_s_send_photon( scene, &out, color, scene->trace_depth );
        }
    }
    st_s_print_fa( " #<sz_t> photons\n", scene->photon_map.size );
}

/**********************************************************************************************************************/

image_cps_s* scene_s_show_photon_map( const scene_s* scene )
{
    image_cl_s* image = image_cl_s_create();
    image_cl_s_set_size( image, scene->image_width, scene->image_height, ( cl_s ){ 0, 0, 0 } );

    sz_t unit_sz = ( image->h >> 1 );

    v3d_s src = scene->camera_position;

    for( sz_t i = 0; i < scene->photon_map.size; i++ )
    {
        photon_s ph = scene->photon_map.data[ i ];
        v3d_s dir = v3d_s_of_length( v3d_s_sub( ph.p, src ), scene->camera_focal_length );
        f3_t img_x = dir.x;
        f3_t img_y = dir.z;

        sz_t pix_x = llrint( ( image->w >> 1 ) + img_x * unit_sz );
        sz_t pix_y = llrint( ( image->h >> 1 ) - img_y * unit_sz );


        if( pix_x < image->w && pix_y < image->h )
        {
            image_cl_s_add_pixel( image, pix_x, pix_y, ph.c );
        }
    }

    image_cps_s* image_cps = image_cps_s_create();
    image_cps_s_copy_cl( image_cps, image );
    image_cl_s_discard( image );
    return image_cps;

}

/**********************************************************************************************************************/

typedef struct image_creator_s
{
    const scene_s* scene;
    image_cl_s* image;
    sz_t row_count;
    bcore_mutex_t mutex;
} image_creator_s;

void image_creator_s_init( image_creator_s* o )
{
    bcore_memzero( o, sizeof( *o ) );
    bcore_mutex_init( &o->mutex );
}

void image_creator_s_down( image_creator_s* o )
{
    bcore_mutex_down( &o->mutex );
}

DEFINE_FUNCTION_CREATE( image_creator_s )
DEFINE_FUNCTION_DISCARD( image_creator_s )

image_creator_s* image_creator_s_plant( const scene_s* scene, image_cl_s* image )
{
    image_creator_s* o = image_creator_s_create();
    o->scene = scene;
    o->image = image;
    return o;
}

void image_creator_s_set_row( image_creator_s* o, sz_t row_num, const row_cl_s* row )
{
    bcore_mutex_lock( &o->mutex );
    image_cl_s_set_row( o->image, row_num, row );
    bcore_mutex_unlock( &o->mutex );
}

sz_t image_creator_s_get_row_num( image_creator_s* o )
{
    bcore_mutex_lock( &o->mutex );
    sz_t row_num = o->row_count++;
    if( ( row_num % 100 ) == 0 )
    {
        bcore_msg( "%5.1f%% ", 100.0 * row_num / o->scene->image_height );
    }
    bcore_mutex_unlock( &o->mutex );
    return row_num;
}

vd_t image_creator_s_func( image_creator_s* o )
{
    sz_t width = o->scene->image_width;
    sz_t height = o->scene->image_height;
    sz_t unit_sz = ( height >> 1 );
    f3_t unit_f = 1.0 / unit_sz;

    row_cl_s* row = row_cl_s_create();
    row_cl_s_set_size( row, width, o->scene->background_color );

    m3d_s camera_rotation;
    {
        v3d_s ry = v3d_s_of_length( o->scene->camera_view_direction, 1 );
        v3d_s rz = v3d_s_of_length( o->scene->camera_top_direction, 1 );
        rz = v3d_s_von( ry, rz );
        v3d_s rx = v3d_s_mlx( ry, rz );
        camera_rotation.x = rx;
        camera_rotation.y = ry;
        camera_rotation.z = rz;
        camera_rotation = m3d_s_transposed( camera_rotation );
    }

    sz_t row_num;
    while( ( row_num = image_creator_s_get_row_num( o ) ) < height )
    {
        f3_t z = unit_f * ( 0.5 + ( s3_t )( ( height >> 1 ) - row_num ) );

        for( sz_t i = 0; i < width; i++ )
        {
            f3_t x = unit_f * ( 0.5 + ( s3_t )( i - ( width >> 1 ) ) );
            v3d_s d = { x, o->scene->camera_focal_length, z };
            d = v3d_s_of_length( d, 1.0 );

            ray_s ray;
            ray.p = o->scene->camera_position;
            ray.d = m3d_s_mlv( &camera_rotation, d );

            vc_t hit_obj = NULL;
            f3_t offs = scene_s_hit( o->scene, &ray, &hit_obj, NULL );

            cl_s lum = o->scene->background_color;

            if( offs < f3_inf && hit_obj )
            {
                lum = cl_s_sat( scene_s_lum( o->scene, hit_obj, &ray, offs, o->scene->trace_depth ), o->scene->gamma );
            }
            row->data[ i ] = lum;
        }
        image_creator_s_set_row( o, row_num, row );

    }

    row_cl_s_discard( row );
    return NULL;
}

image_cps_s* scene_s_create_image( const scene_s* o )
{
    bcore_life_s* l = bcore_life_s_create();
    image_cl_s* image = bcore_life_s_push_aware( l, image_cl_s_create() );
    image_cl_s_set_size( image, o->image_width, o->image_height, cl_black() );

    image_creator_s* image_creator = image_creator_s_plant( o, image );

    {
        sz_t threads = o->threads > 0 ? o->threads : 1;
        pthread_t* thread_arr = bcore_u_alloc( sizeof( pthread_t ), NULL, threads, NULL );

        for( sz_t i = 0; i < threads; i++ )
        {
            pthread_create( &thread_arr[ i ], NULL, ( vd_t(*)(vd_t) )image_creator_s_func, image_creator );
        }

        for( sz_t i = 0; i < threads; i++ )
        {
            pthread_join( thread_arr[ i ], NULL );
        }

        bcore_free( thread_arr );
        image_creator_s_discard( image_creator );
    }

    image_cps_s* image_cps = image_cps_s_create();
    image_cps_s_copy_cl( image_cps, image );

    st_s_print_fa( "\nHash: #<tp_t>\n", image_cps_s_hash( image_cps ) );

    bcore_life_s_discard( l );
    return image_cps;
}

/**********************************************************************************************************************/

vd_t scene_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "scene" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        bcore_flect_define_creator( typeof( "photon_s"     ), photon_s_create_self  );
        bcore_flect_define_creator( typeof( "photon_map_s" ), photon_map_s_create_self  );

        bcore_flect_define_creator( typeof( "spect_txm_s" ), spect_txm_s_create_self );
        bcore_flect_define_creator( typeof( "txm_plain_s" ), txm_plain_s_create_self );
        bcore_flect_define_creator( typeof( "txm_chess_s" ), txm_chess_s_create_self );

        bcore_flect_define_creator( typeof( "properties_s" ), properties_s_create_self );

        bcore_flect_define_creator( typeof( "spect_obj_s"    ), spect_obj_s_create_self    );
        bcore_flect_define_creator( typeof( "obj_plane_s"    ), obj_plane_s_create_self    );
        bcore_flect_define_creator( typeof( "obj_sphere_s"   ), obj_sphere_s_create_self   );

        bcore_flect_define_creator( typeof( "compound_s" ), compound_s_create_self );
        bcore_flect_define_creator( typeof( "scene_s"    ), scene_s_create_self );

        bcore_flect_define_creator( typeof( "image_cps_s" ), image_cps_s_create_self );
    }

    return NULL;
}

/**********************************************************************************************************************/


