/** Rayflux Scene
 *  Author: Johannes Bernhard Steffens
 *
 *  Copyright (c) 2017 Johannes Bernhard Steffens. All rights reserved.
 */

#include <math.h>
#include "bcore_threads.h"
#include "bcore_sinks.h"
#include "bcore_spect_inst.h"
#include "bcore_life.h"
#include "bcore_spect.h"
#include "bcore_spect_array.h"
#include "bcore_txt_ml.h"
#include "bcore_trait.h"

#include "scene.h"
#include "textures.h"
#include "objects.h"
#include "container.h"


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
/// scene_s

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
    f3_t photon_max_travel_distance;

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
    "f3_t photon_max_travel_distance = 1E300;"

    "compound_s light;"
    "compound_s matter;"
    "photon_map_s photon_map;"
"}";

DEFINE_FUNCTIONS_OBJ_INST( scene_s )
DEFINE_CREATE_SELF( scene_s, scene_s_def )

void scene_s_clear( scene_s* o )
{
    compound_s_clear( &o->light );
    compound_s_clear( &o->matter );
}

sz_t scene_s_push( scene_s* o, const sr_s* object )
{
    tp_t type = sr_s_type( object );
    if( bcore_trait_is( type, typeof( "spect_obj" ) ) )
    {
        if( obj_radiance( object->o ) > 0 )
        {
            compound_s_push_q( &o->light, object );
        }
        else
        {
            compound_s_push_q( &o->matter, object );
        }
        return 1;
    }
    else if( type == typeof( "map_s" ) )
    {
        map_s* map = object->o;
        sz_t size = bcore_hmap_tp_sr_s_size( &map->m );
        for( sz_t i = 0; i < size; i++ )
        {
            if( bcore_hmap_tp_sr_s_idx_key( &map->m, i ) )
            {
                scene_s_push( o, bcore_hmap_tp_sr_s_idx_val( &map->m, i ) );
            }
        }
    }
    else if( type == typeof( "arr_s" ) )
    {
        arr_s* arr = object->o;
        sz_t size = arr_s_get_size( arr );
        for( sz_t i = 0; i < size; i++ )
        {
            scene_s_push( o, arr_s_get( arr, i ) );
        }
    }
    return 0;
}

sz_t scene_s_objects( const scene_s* o )
{
    return o->light.size + o->matter.size;
}

sr_s scene_s_meval_key( sr_s* sr_o, meval_s* ev, tp_t key )
{
    assert( sr_s_type( sr_o ) == TYPEOF_scene_s );
    scene_s* o = sr_o->o;
    if( key == TYPEOF_clear )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
        scene_s_clear( o );
        return sr_null();
    }
    else if( key == TYPEOF_push )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        sr_s obj = meval_s_eval( ev, sr_null() );
        scene_s_push( o, &obj );
        sr_down( obj );
        meval_s_expect_code( ev, CL_ROUND_BRACKET_CLOSE );
        return sr_null();
    }
    else
    {
        meval_s_err_fa( ev, "scene_s has no member '#sc_t'.", meval_s_get_name( ev, key ) );
    }
    return sr_null();
}

f3_t scene_s_hit( const scene_s* o, const ray_s* r, vc_t* hit_obj, bl_t* is_light )
{
    f3_t min_a = f3_inf;
    f3_t a;
    vc_t hit_obj_l;

    if( ( a = compound_s_fwd_hit( &o->light, r, &hit_obj_l ) ) < min_a )
    {
        min_a = a;
        if( hit_obj ) *hit_obj = hit_obj_l;
        if( is_light ) *is_light = true;
    }

    if( ( a = compound_s_fwd_hit( &o->matter, r, &hit_obj_l ) ) < min_a )
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
    f3_t a = compound_s_fwd_hit( &scene->matter, ray, ( vc_t* )&hit_obj );
    if( a >= scene->photon_max_travel_distance ) return;
    v3d_s pos = ray_s_pos( ray, a );

    f3_t reflectance = 0;

    /// reflections
    if( hit_obj->prp.n > 1.0 )
    {
        v3d_s nor = obj_normal( hit_obj, pos );
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
        cl_s texture = txm_clr( hit_obj->prp.txm, hit_obj, pos );
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
            lum = v3d_s_add( lum, v3d_s_mlf( txm_clr( obj->prp.txm, obj, pos ), intensity ) );
        }
        return lum;
    }

    f3_t reflectance = 0;

    /// reflections
    if( obj->prp.n > 1.0 )
    {
        v3d_s nor = obj_normal( obj, pos );
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
        else
        {
            lum = v3d_s_mlf( scene->background_color, reflectance );
        }
    }

    f3_t transmittance = 1.0 - reflectance; // only surface transmittance

    /// direct light
    if( scene->direct_samples > 0 )
    {
        ray_s surface = { .p = pos, .d = obj_normal( obj, surface.p ) };

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
            ray_cone_s fov_to_src = obj_fov( light_src, pos );
            m3d_s src_con = m3d_s_transposed( m3d_s_con_z( fov_to_src.ray.d ) );
            f3_t cyl_hgt = areal_coverage( fov_to_src.cos_rs );

            if( *( aware_t* )light_src->prp.txm != TYPEOF_txm_plain_s ) ERR( "Light sources with texture map are not yet supported." );
            cl_s color = v3d_s_mlf( txm_plain_clr( light_src->prp.txm ), light_src->prp.radiance );

            bcore_arr_sz_s* idx_arr = compound_s_in_fov_arr( &scene->matter, &fov_to_src );

            for( sz_t j = 0; j < scene->direct_samples; j++ )
            {
                out.d = m3d_s_mlv( &src_con, v3d_s_rsc( &rv, cyl_hgt ) );
                f3_t weight = v3d_s_mlv( out.d, surface.d );
                if( weight <= 0 ) continue;

                f3_t a = obj_fwd_hit( light_src, &out );
                if( a >= f3_inf ) continue;
                if( compound_s_idx_fwd_hit( &scene->matter, idx_arr, &out, NULL ) > a )
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
                f3_t a = compound_s_fwd_hit( &scene->matter, &out, &hit_obj );
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
                f3_t a = compound_s_fwd_hit( &scene->matter, &out, &hit_obj );

                if( a >= f3_inf || hit_obj == obj )
                {
                    cl_sum = v3d_s_add( cl_sum, v3d_s_mlf( ph.c, weight ) );
                }
            }
            cl_per = v3d_s_add( cl_per, v3d_s_mlf( cl_sum, 1.0 / scene->photon_samples ) );
        }

        cl_s texture = txm_clr( obj->prp.txm, obj, pos );
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
        cl_s color = v3d_s_mlf( txm_plain_clr( light_src->prp.txm ), light_src->prp.radiance );
        ray_s out;
        out.p = light_src->prp.pos;
        u2_t rv = 1234;
        for( sz_t i = 0; i < scene->photon_samples; i++ )
        {
            out.d = v3d_s_rsc( &rv, 2.0 );
            scene_s_send_photon( scene, &out, color, scene->trace_depth );
        }
    }
    st_s_print_fa( " #<sz_t> collected photons\n", scene->photon_map.size );
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
                lum = scene_s_lum( o->scene, hit_obj, &ray, offs, o->scene->trace_depth );
            }
            row->data[ i ] = cl_s_sat( lum, o->scene->gamma );
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
        bcore_flect_define_creator( typeof( "scene_s"    ), scene_s_create_self );
        bcore_flect_define_creator( typeof( "image_cps_s" ), image_cps_s_create_self );
    }

    return NULL;
}

/**********************************************************************************************************************/


