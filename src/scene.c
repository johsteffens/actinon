/** Rayflux Scene */

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
#include <stdio.h>

#include "bcore_threads.h"
#include "bcore_sinks.h"
#include "bcore_spect_inst.h"
#include "bcore_life.h"
#include "bcore_spect.h"
#include "bcore_spect_array.h"
#include "bcore_txt_ml.h"
#include "bcore_trait.h"
#include "bcore_sources.h"

#include "scene.h"
#include "objects.h"
#include "container.h"
#include "gmath.h"

/**********************************************************************************************************************/
/// globals

bl_t scene_s_overwrite_output_files_g = false;

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

void photon_map_s_clear( photon_map_s* o )
{
    bcore_array_aware_set_space( o, 0 );
}

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
    f3_t gradient_threshold;
    sz_t gradient_samples;
    sz_t gradient_cycles;

    cl_s background_color;

    v3d_s camera_position;
    v3d_s camera_view_direction;
    v3d_s camera_top_direction;
    f3_t  camera_focal_length;

    sz_t trace_depth;
    f3_t trace_min_intensity;

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
    "f3_t gradient_threshold = 0.1;"
    "sz_t gradient_samples = 10;"
    "sz_t gradient_cycles = 1;"
    "cl_s background_color;"

    "v3d_s camera_position;"
    "v3d_s camera_view_direction;"
    "v3d_s camera_top_direction;"
    "f3_t  camera_focal_length = 1.0;"

    "sz_t trace_depth         = 11;"
    "f3_t trace_min_intensity = 0;"
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
    scene_s_clear_photon_map( o );
}

sz_t scene_s_push( scene_s* o, const sr_s* object )
{
    tp_t type = sr_s_type( object );
    if( bcore_trait_is_of( type, typeof( "spect_obj" ) ) )
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
    else if( key == TYPEOF_create_image )
    {
        meval_s_expect_code( ev, CL_ROUND_BRACKET_OPEN  );
        sr_s obj = meval_s_eval( ev, sr_null() );
        if( sr_s_type( &obj ) != TYPEOF_st_s ) meval_s_err_fa( ev, "String expected." );

        const st_s* png_file = obj.o;
        scene_s_create_image_file( o, png_file->sc );

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

f3_t scene_s_hit( const scene_s* o, const ray_s* r, v3d_s* p_nor, vc_t* hit_obj )
{
    f3_t min_a = f3_inf;
    f3_t a;
    vc_t hit_obj_l;

    v3d_s nor;

    if( ( a = compound_s_ray_hit( &o->light, r, &nor, &hit_obj_l ) ) < min_a )
    {
        min_a = a;
        if( hit_obj ) *hit_obj = hit_obj_l;
        if( p_nor ) *p_nor = nor;
    }

    if( ( a = compound_s_ray_hit( &o->matter, r, &nor, &hit_obj_l ) ) < min_a )
    {
        min_a = a;
        if( hit_obj ) *hit_obj = hit_obj_l;
        if( p_nor ) *p_nor = nor;
    }

    return min_a;
}

/// sends a photon and registers it in photon map
void scene_s_send_photon( scene_s* scene, const ray_s* ray, cl_s color, sz_t depth )
{
    if( depth == 0 ) return;
    obj_hdr_s* hit_obj;
    v3d_s nor;
    f3_t offs = compound_s_ray_hit( &scene->matter, ray, &nor, ( vc_t* )&hit_obj );
    if( offs >= scene->photon_max_travel_distance ) return;
    v3d_s pos = ray_s_pos( ray, offs );

    if( v3d_s_mlv( ray->d, nor ) > 0 && hit_obj->prp.transparent ) // ray traveled through object
    {
        f3_t rf = hit_obj->prp.color.x > 0 ? pow( 0.5, offs / hit_obj->prp.color.x ) : 0.0;
        f3_t gf = hit_obj->prp.color.y > 0 ? pow( 0.5, offs / hit_obj->prp.color.y ) : 0.0;
        f3_t bf = hit_obj->prp.color.z > 0 ? pow( 0.5, offs / hit_obj->prp.color.z ) : 0.0;
        color.x *= rf;
        color.y *= gf;
        color.z *= bf;
    }

    f3_t reflectance = 0;
    f3_t transmittance = 1.0 - reflectance; // only surface transmittance

    /// reflections and refractions
    if( hit_obj->prp.refractive_index > 1.0 || hit_obj->prp.transparent )
    {
        ray_s out_r;
        ray_s out_t;
        out_r.p = pos;
        out_t.p = pos;
        compute_refraction( ray->d, nor, hit_obj->prp.refractive_index, &reflectance, &out_r.d, &transmittance, &out_t.d );
        scene_s_send_photon( scene, &out_r, v3d_s_mlf( color, reflectance ), depth - 1 );
        if( transmittance > 0 && hit_obj->prp.transparent )
        {
            scene_s_send_photon( scene, &out_t, v3d_s_mlf( color, transmittance ), depth - 1 );
        }
    }

    if( !hit_obj->prp.transparent )
    {
        color = v3d_s_mlf( color, transmittance );
        if( v3d_s_sqr( color ) > 0 )
        {
            cl_s cl = obj_color( hit_obj, pos );
            color.x *= cl.x;
            color.y *= cl.y;
            color.z *= cl.z;
            photon_map_s_push( &scene->photon_map, ( photon_s ) { .c = color, .p = pos } );
        }
    }
}

cl_s scene_s_lum( const scene_s* scene, const obj_hdr_s* obj, const ray_s* ray, f3_t offs, v3d_s nor, sz_t depth, f3_t intensity )
{
    cl_s lum = { 0, 0, 0 };
    if( depth == 0 || intensity < scene->trace_min_intensity ) return lum;

    v3d_s pos = ray_s_pos( ray, offs );

    if( obj->prp.radiance > 0 )
    {
        f3_t diff_sqr = v3d_s_diff_sqr( pos, obj->prp.pos );
        f3_t light_intensity = ( diff_sqr > 0 ) ? ( obj->prp.radiance / diff_sqr ) : f3_mag;
        return v3d_s_mlf( obj_color( obj, pos ), light_intensity * intensity );
    }

    f3_t reflectance = 0;
    f3_t transmittance = 1.0;
    bl_t transparent = obj->prp.transparent;

    /// reflections and refractions
    if( obj->prp.refractive_index > 1.0 )
    {
        ray_s out_r;
        ray_s out_t;
        out_r.p = pos;
        out_t.p = ray_s_pos( ray, offs + 2.0 * f3_eps );
        compute_refraction( ray->d, nor, obj->prp.refractive_index, &reflectance, &out_r.d, &transmittance, transparent ? &out_t.d : NULL );
        vc_t  hit_obj = NULL;
        v3d_s hit_nor;
        f3_t a;
        if ( ( a = scene_s_hit( scene, &out_r, &hit_nor, &hit_obj ) ) < f3_inf )
        {
            lum = scene_s_lum( scene, hit_obj, &out_r, a, hit_nor, depth - 1, reflectance * intensity );
        }
        else
        {
            lum = v3d_s_mlf( scene->background_color, reflectance * intensity );
        }

        if( transparent )
        {
            if( obj_side( obj, out_r.p ) * obj_side( obj, out_t.p ) == -1 ) // truly stepping through surface
            {
                if ( ( a = scene_s_hit( scene, &out_t, &hit_nor, &hit_obj ) ) < f3_inf )
                {
                    lum = v3d_s_add( lum, scene_s_lum( scene, hit_obj, &out_t, a, hit_nor, depth - 1, transmittance * intensity ) );
                }
                else
                {
                    lum = v3d_s_add( lum, v3d_s_mlf( scene->background_color, transmittance * intensity ) );
                }

                if( v3d_s_mlv( ray->d, nor ) > 0 ) // exiting object
                {
                    f3_t rf = obj->prp.color.x > 0 ? pow( 0.5, offs / obj->prp.color.x ) : 0.0;
                    f3_t gf = obj->prp.color.y > 0 ? pow( 0.5, offs / obj->prp.color.y ) : 0.0;
                    f3_t bf = obj->prp.color.z > 0 ? pow( 0.5, offs / obj->prp.color.z ) : 0.0;
                    lum.x *= rf;
                    lum.y *= gf;
                    lum.z *= bf;
                }
            }
        }
    }

    /// direct light
    if( scene->direct_samples > 0 && !transparent )
    {
        bl_t outside = obj_side( obj, pos ) == 1;
        ray_s surface = { .p = pos, .d = outside ? nor : v3d_s_neg( nor ) };

        /// random seed
        u2_t rv = surface.p.x * 32944792 + surface.p.y * 76403048 + surface.p.z * 24349373;

        /// peripheral light
        cl_s cl_per = { 0, 0, 0 };

        f3_t trans_intensity = intensity * transmittance;

        /// process sources with radiance directly  (light-sources)
        for( sz_t i = 0; i < scene->light.size; i++ )
        {
            cl_s cl_sum = { 0, 0, 0 };
            ray_s out = surface;
            obj_hdr_s* light_src = scene->light.data[ i ];
            ray_cone_s fov_to_src = obj_fov( light_src, pos );
            m3d_s src_con = m3d_s_transposed( m3d_s_con_z( fov_to_src.ray.d ) );
            f3_t cyl_hgt = areal_coverage( fov_to_src.cos_rs );
            cl_s color = obj_color( light_src, light_src->prp.pos );
            bcore_arr_sz_s* idx_arr = compound_s_in_fov_arr( &scene->matter, &fov_to_src );
            sz_t direct_samples = scene->direct_samples * trans_intensity;
            direct_samples = ( direct_samples == 0 ) ? 1 : direct_samples;
            for( sz_t j = 0; j < direct_samples; j++ )
            {
                out.d = m3d_s_mlv( &src_con, v3d_s_rsc( &rv, cyl_hgt ) );
                f3_t weight = v3d_s_mlv( out.d, surface.d );
                if( weight <= 0 ) continue;

                f3_t a = obj_ray_hit( light_src, &out, NULL );
                if( a >= f3_inf ) continue;
                if( compound_s_idx_ray_hit( &scene->matter, idx_arr, &out, NULL, NULL ) > a )
                {
                    v3d_s hit_pos = ray_s_pos( &out, a );

                    f3_t diff_sqr = v3d_s_diff_sqr( hit_pos, light_src->prp.pos );
                    f3_t local_intensity = ( diff_sqr > 0 ) ? ( light_src->prp.radiance / diff_sqr ) : f3_mag;

                    cl_sum = v3d_s_add( cl_sum, v3d_s_mlf( color, local_intensity * weight * trans_intensity ) );
                }
            }

            bcore_arr_sz_s_discard( idx_arr );

            // factor 2 arises from weight distribution across the half-sphere
            cl_per = v3d_s_add( cl_per, v3d_s_mlf( cl_sum, 2.0 * cyl_hgt / direct_samples ) );

        }

        // path tracing
        if( scene->path_samples && depth > 10 )
        {
            // via path tracing
            cl_s cl_sum = { 0, 0, 0 };
            ray_s out = surface;
            m3d_s out_con = m3d_s_transposed( m3d_s_con_z( surface.d ) );

            f3_t per_energy = v3d_s_sqr( cl_per );
            per_energy = per_energy > 0.01 ? per_energy : 0.01;

            sz_t path_samples = scene->path_samples * trans_intensity;
            path_samples = ( path_samples == 0 ) ? 1 : path_samples;

            for( sz_t i = 0; i < path_samples; i++ )
            {
                out.d = m3d_s_mlv( &out_con, v3d_s_rsc( &rv, 1.0 ) );
                f3_t weight = v3d_s_mlv( out.d, surface.d );
                if( weight <= 0 ) continue;
                vc_t  hit_obj = NULL;
                v3d_s hit_nor;
                f3_t a = compound_s_ray_hit( &scene->matter, &out, &hit_nor, &hit_obj );
                if( a < f3_inf && hit_obj )
                {
                    cl_s lum = scene_s_lum( scene, hit_obj, &out, a, hit_nor, depth - 10, weight * trans_intensity );
                    cl_sum = v3d_s_add( cl_sum, lum );
                }
                else
                {
                    cl_sum = v3d_s_add( cl_sum, v3d_s_mlf( scene->background_color, weight * trans_intensity ) );
                }
            }
            // factor 2 arises from weight distribution across the half-sphere
            cl_per = v3d_s_add( cl_per, v3d_s_mlf( cl_sum, 2.0 / path_samples ) );
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
                f3_t a = compound_s_ray_hit( &scene->matter, &out, NULL, &hit_obj );

                if( a >= f3_inf || hit_obj == obj )
                {
                    cl_sum = v3d_s_add( cl_sum, v3d_s_mlf( ph.c, weight * intensity * transmittance ) );
                }
            }
            cl_per = v3d_s_add( cl_per, v3d_s_mlf( cl_sum, 1.0 / scene->photon_samples ) );
        }

        cl_s cl = obj_color( obj, pos );
        cl_per.x *= cl.x;
        cl_per.y *= cl.y;
        cl_per.z *= cl.z;
        lum = v3d_s_add( lum, cl_per );
    }

    return lum;
}

/**********************************************************************************************************************/

void scene_s_clear_photon_map( scene_s* o )
{
    photon_map_s_clear( &o->photon_map );
}

void scene_s_create_photon_map( scene_s* o )
{
    scene_s_clear_photon_map( o );

    /// process sources with radiance directly  (light-sources)
    for( sz_t i = 0; i < o->light.size; i++ )
    {
        obj_hdr_s* light_src = o->light.data[ i ];
        cl_s color = v3d_s_mlf( obj_color( light_src, light_src->prp.pos ), light_src->prp.radiance );
        ray_s out;
        out.p = light_src->prp.pos;
        u2_t rv = 1234;
        for( sz_t i = 0; i < o->photon_samples; i++ )
        {
            out.d = v3d_s_rsc( &rv, 2.0 );
            scene_s_send_photon( o, &out, color, o->trace_depth );
        }
    }
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

// luminance at a given position
typedef struct lum_s
{
    v2d_s pos;
    cl_s  clr;
} lum_s;

DEFINE_FUNCTIONS_OBJ_FLAT( lum_s )
DEFINE_CREATE_SELF( lum_s,  "lum_s = bcore_inst { v2d_s pos; cl_s clr; }" )

tp_t lum_s_key( const lum_s* o )
{
    s2_t x = o->pos.x;
    s2_t y = o->pos.y;
    return bcore_tp_fold_u2( bcore_tp_fold_u2( bcore_tp_init(), x ), y );
}

typedef struct lum_arr_s
{
    aware_t _;
    union
    {
        bcore_static_array_s arr;
        struct
        {
            lum_s* data;
            sz_t size, space;
        };
    };
} lum_arr_s;

DEFINE_FUNCTIONS_OBJ_INST( lum_arr_s )
DEFINE_CREATE_SELF( lum_arr_s,  "lum_arr_s = bcore_inst { aware_t _; lum_s [] arr; }" )

void lum_arr_s_clear( lum_arr_s* o )
{
    bcore_array_aware_set_size( o, 0 );
}

void lum_arr_s_push( lum_arr_s* o, lum_s lum )
{
    if( o->space == o->size ) bcore_array_aware_set_space( o, o->space > 0 ? o->space * 2 : 256 );
    o->data[ o->size++ ] = lum;
}

typedef struct lum_map_s
{
    aware_t _;
    bcore_hmap_tp_sr_s map;
} lum_map_s;

DEFINE_FUNCTIONS_OBJ_INST( lum_map_s )
DEFINE_CREATE_SELF( lum_map_s,  "lum_map_s = bcore_inst { aware_t _; bcore_hmap_tp_sr_s map; }" )

void lum_map_s_clear( lum_map_s* o )
{
    bcore_hmap_tp_sr_s_clear( &o->map );
}

void lum_map_s_push( lum_map_s* o, lum_s lum )
{
    tp_t key = lum_s_key( &lum );
    sr_s* sr = bcore_hmap_tp_sr_s_get( &o->map, key );
    if( !sr ) sr = bcore_hmap_tp_sr_s_set( &o->map, key, sr_psd( bcore_array_s_get_typed( TYPEOF_lum_arr_s ), lum_arr_s_create() ) );
    bcore_array_q_push( sr, sr_twc( TYPEOF_lum_s, &lum ) );
}

void lum_map_s_push_arr( lum_map_s* o, const lum_arr_s* lum_arr )
{
    for( sz_t i = 0; i < lum_arr->size; i++ ) lum_map_s_push( o, lum_arr->data[ i ] );
}

lum_s lum_map_s_get_plain_avg( const lum_map_s* o, s2_t x, s2_t y )
{
    tp_t key = bcore_tp_fold_u2( bcore_tp_fold_u2( bcore_tp_init(), x ), y );
    sr_s* sr = bcore_hmap_tp_sr_s_get( &o->map, key );
    lum_s lum_sum = { .pos = { 0, 0 }, .clr = { 0, 0, 0 } };
    f3_t weight = 0;
    if( sr )
    {
        sz_t size = bcore_array_q_get_size( sr );
        for( sz_t i = 0; i < size; i++ )
        {
            sr_s lum_sr = bcore_array_q_get( sr, i );
            lum_s* lum = lum_sr.o;
            s2_t x_l = lum->pos.x;
            s2_t y_l = lum->pos.y;
            if( x == x_l && y == y_l )
            {
                lum_sum.pos = v2d_s_add( lum_sum.pos, lum->pos );
                lum_sum.clr = v3d_s_add( lum_sum.clr, lum->clr );
                weight += 1.0;
            }
            sr_down( lum_sr );
        }
    }

    f3_t f = ( weight > 0 ) ? 1.0 / weight : 1.0;
    return ( lum_s ) { .pos = v2d_s_mlf( lum_sum.pos, f ), .clr = v3d_s_mlf( lum_sum.clr, f ) };
}

/**********************************************************************************************************************/

typedef struct lum_machine_s
{
    const scene_s* scene;
    lum_arr_s* lum_arr;
    sz_t index;
    bcore_mutex_t mutex;
} lum_machine_s;

void lum_machine_s_init( lum_machine_s* o )
{
    bcore_memzero( o, sizeof( *o ) );
    bcore_mutex_init( &o->mutex );
}

void lum_machine_s_down( lum_machine_s* o )
{
    bcore_mutex_down( &o->mutex );
}

DEFINE_FUNCTION_CREATE( lum_machine_s )
DEFINE_FUNCTION_DISCARD( lum_machine_s )

lum_machine_s* lum_machine_s_plant( const scene_s* scene, lum_arr_s* lum_arr )
{
    lum_machine_s* o = lum_machine_s_create();
    o->scene = scene;
    o->lum_arr = lum_arr;
    return o;
}

sz_t lum_machine_s_get_index( lum_machine_s* o )
{
    bcore_mutex_lock( &o->mutex );
    sz_t index = o->index++;
    if( ( ( index + 1 ) %  5000 ) == 0 ) bcore_msg( "." );
    if( ( ( index + 1 ) % 50000 ) == 0 ) bcore_msg( "%5.1f%% ", ( 100.0 * index ) / o->lum_arr->size );
    bcore_mutex_unlock( &o->mutex );
    return index;
}

vd_t lum_machine_s_func( lum_machine_s* o )
{
    sz_t width = o->scene->image_width;
    sz_t height = o->scene->image_height;
    sz_t unit_sz = ( height >> 1 );
    f3_t unit_f = 1.0 / unit_sz;

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

    sz_t index;
    while( ( index = lum_machine_s_get_index( o ) ) < o->lum_arr->size )
    {
        lum_s* lum = &o->lum_arr->data[ index ];
        f3_t monitor_y = lum->pos.y;
        f3_t monitor_x = lum->pos.x;
        f3_t z = unit_f * ( ( height >> 1 ) - monitor_y );
        f3_t x = unit_f * ( monitor_x - ( width >> 1 ) );
        v3d_s d = { x, o->scene->camera_focal_length, z };
        d = v3d_s_of_length( d, 1.0 );

        ray_s ray;
        ray.p = o->scene->camera_position;
        ray.d = m3d_s_mlv( &camera_rotation, d );

        vc_t  hit_obj = NULL;
        v3d_s hit_nor;
        f3_t offs = scene_s_hit( o->scene, &ray, &hit_nor, &hit_obj );

        lum->clr = o->scene->background_color;

        if( offs < f3_inf && hit_obj )
        {
            lum->clr = scene_s_lum( o->scene, hit_obj, &ray, offs, hit_nor, o->scene->trace_depth, 1.0 );
        }
    }
    return NULL;
}

void lum_machine_s_run( const scene_s* scene, lum_arr_s* lum_arr )
{
    lum_machine_s* machine = lum_machine_s_plant( scene, lum_arr );
    sz_t threads = scene->threads > 0 ? scene->threads : 1;
    pthread_t* thread_arr = bcore_u_alloc( sizeof( pthread_t ), NULL, threads, NULL );
    for( sz_t i = 0; i < threads; i++ ) pthread_create( &thread_arr[ i ], NULL, ( vd_t(*)(vd_t) )lum_machine_s_func, machine );
    for( sz_t i = 0; i < threads; i++ ) pthread_join( thread_arr[ i ], NULL );
    bcore_free( thread_arr );
    lum_machine_s_discard( machine );
}

f3_t lum_dev( const scene_s* o, v3d_s ref, const lum_map_s* lum_map, s3_t x, s3_t y )
{
    if( x < 0 || x >= o->image_width  ) return 0;
    if( y < 0 || y >= o->image_height ) return 0;
    return v3d_s_sqr( v3d_s_sub( ref, lum_map_s_get_plain_avg( lum_map, x, y ).clr ) );
}

f3_t lum_sqr_grad( const scene_s* o, const lum_map_s* lum_map, s3_t x, s3_t y )
{
    f3_t g0 = 0;
    f3_t g1 = 0;
    v3d_s v = lum_map_s_get_plain_avg( lum_map, x, y ).clr;
    g1 = lum_dev( o, v, lum_map, x - 1, y - 1 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_dev( o, v, lum_map, x - 1, y + 0 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_dev( o, v, lum_map, x - 1, y + 1 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_dev( o, v, lum_map, x + 0, y - 1 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_dev( o, v, lum_map, x + 0, y + 1 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_dev( o, v, lum_map, x + 1, y - 1 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_dev( o, v, lum_map, x + 1, y + 0 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_dev( o, v, lum_map, x + 1, y + 1 ); g0 = g1 > g0 ? g1 : g0;
    return g0;
}

void scene_s_create_image_file_from_lum_map( const scene_s* o, const lum_map_s* lum_map, sc_t file )
{
    image_cl_s* image = image_cl_s_create();
    image_cl_s_set_size( image, o->image_width, o->image_height, cl_black() );
    image_cps_s* image_cps = image_cps_s_create();

    for( sz_t j = 0; j < o->image_height; j++ )
    {
        for( sz_t i = 0; i < o->image_width; i++ )
        {
            image_cl_s_set_pixel( image, i, j, lum_map_s_get_plain_avg( lum_map, i, j ).clr );
        }
    }
    image_cl_s_saturate( image, o->gamma );
    image_cps_s_copy_cl( image_cps, image );
    image_cps_s_write_pnm( image_cps, file );
    st_s_print_fa( "  Hash: #<tp_t>", image_cps_s_hash( image_cps ) );

    image_cps_s_discard( image_cps );
    image_cl_s_discard( image );
}

void scene_s_create_image_file( scene_s* o, sc_t file )
{
    if( bcore_source_file_s_exists( file ) && !scene_s_overwrite_output_files_g )
    {
        bcore_msg_fa( "File '#<sc_t>' exists. Overwrite it? [Y|N]:", file );
        char buf[ 2 ];
        if( fgets( buf, 2, stdin )[ 0 ] != 'Y' ) abort();
    }

    bcore_life_s* l = bcore_life_s_create();

    bcore_msg_fa( "Number of objects: #<sz_t>\n", scene_s_objects( o ) );

    if( o->photon_samples > 0 )
    {
        bcore_msg_fa( "Creating photon map:" );
        clock_t time = clock();
        scene_s_create_photon_map( o );
        time = clock() - time;
        bcore_msg_fa( " #<sz_t> photons;", o->photon_map.size );
        bcore_msg( " %5.3g cs;\n", ( f3_t )time / ( CLOCKS_PER_SEC ) );
    }

    lum_arr_s* lum_arr = bcore_life_s_push_aware( l, lum_arr_s_create() );

    st_s_print_fa( "Rendering ...\n" );
    clock_t time = clock();
    st_s_print_fa( "\tmain image: " );
    for( sz_t j = 0; j < o->image_height; j++ )
    {
        for( sz_t i = 0; i < o->image_width; i++ )
        {
            lum_arr_s_push( lum_arr, ( lum_s ) { .pos = { i + 0.5, j + 0.5 }, .clr = { 0, 0, 0 } } );
        }
    }

    lum_machine_s_run( o, lum_arr );
    lum_map_s* lum_map = bcore_life_s_push_aware( l, lum_map_s_create() );
    lum_map_s_push_arr( lum_map, lum_arr );

    u2_t rv = 1234;
    sz_t rnd_samples = o->gradient_samples;
    f3_t sqr_gradient_theshold = f3_sqr( o->gradient_threshold );

    scene_s_create_image_file_from_lum_map( o, lum_map, file );
    for( sz_t k = 0; k < o->gradient_cycles; k++ )
    {
        st_s_print_fa( "\n\tgradient pass #pl3 {#<sz_t>}: ", k + 1 );
        lum_arr_s_clear( lum_arr );
        for( s3_t j = 0; j < o->image_height; j++ )
        {
            for( s3_t i = 0; i < o->image_width; i++ )
            {
                if( lum_sqr_grad( o, lum_map, i, j ) > sqr_gradient_theshold )
                {
                    for( sz_t k = 0; k < rnd_samples; k++ )
                    {
                        f3_t dx = f3_rnd1( &rv );
                        f3_t dy = f3_rnd1( &rv );
                        lum_arr_s_push( lum_arr, ( lum_s ) { .pos = { i + dx, j + dy }, .clr = { 0, 0, 0 } } );
                    }
                }
            }
        }
        lum_machine_s_run( o, lum_arr );
        lum_map_s_push_arr( lum_map, lum_arr );
        scene_s_create_image_file_from_lum_map( o, lum_map, file );
    }
    time = clock() - time;
    bcore_msg( "\n%5.3g cs\n", ( f3_t )time / ( CLOCKS_PER_SEC ) );


    scene_s_clear_photon_map( o );
    bcore_life_s_discard( l );
}


/**********************************************************************************************************************/

vd_t scene_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "scene" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        bcore_flect_define_creator( typeof( "photon_s"     ), photon_s_create_self  );
        bcore_flect_define_creator( typeof( "photon_map_s" ), photon_map_s_create_self  );
        bcore_flect_define_creator( typeof( "scene_s"      ), scene_s_create_self );
        bcore_flect_define_creator( typeof( "image_cps_s"  ), image_cps_s_create_self );

        bcore_flect_define_creator( typeof( "lum_s"     ), lum_s_create_self     );
        bcore_flect_define_creator( typeof( "lum_arr_s" ), lum_arr_s_create_self );
        bcore_flect_define_creator( typeof( "lum_map_s" ), lum_map_s_create_self );
    }

    return NULL;
}

/**********************************************************************************************************************/


