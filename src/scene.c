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
    f3_t max_path_length;  // path rays longer than max_path_length obtain background color

    compound_s light;  // light sources
    compound_s matter; // passive objects

    s3_t experimental_level; // (default: 0 ) > 0 for experimental approaches

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
    "f3_t max_path_length     = 1E+30;"  // path rays longer than max_path_length obtain background color

    "compound_s light;"
    "compound_s matter;"

    "s3_t experimental_level = 0;" // (default: 0 ) > 0 for experimental approaches
"}";

DEFINE_FUNCTIONS_OBJ_INST( scene_s )
DEFINE_CREATE_SELF( scene_s, scene_s_def )

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
    else if( type == TYPEOF_map_s )
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
    else if( type == TYPEOF_arr_s )
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

f3_t scene_s_trans_hit( const scene_s* o, const ray_s* r, v3d_s* p_exit_nor, vc_t* exit_obj, vc_t* enter_obj )
{
    f3_t min_a = f3_inf;
    f3_t a;

    vc_t exit_obj_l;
    vc_t enter_obj_l;
    v3d_s nor;

    if( ( a = compound_s_ray_hit( &o->light, r, &nor, &enter_obj_l ) ) < min_a )
    {
        min_a = a;
        if( enter_obj ) *enter_obj = enter_obj_l;
        if( p_exit_nor ) *p_exit_nor = v3d_s_neg( nor );
    }

    if( ( a = compound_s_ray_trans_hit( &o->matter, r, &nor, &exit_obj_l, &enter_obj_l ) ) < min_a )
    {
        min_a = a;
        if( exit_obj ) *exit_obj = exit_obj_l;
        if( enter_obj ) *enter_obj = enter_obj_l;
        if( p_exit_nor ) *p_exit_nor = nor;
    }

    return min_a;
}

/**********************************************************************************************************************/

cl_s scene_s_lum( const scene_s* scene,
                  const ray_s* ray,
                  f3_t offs,
                  v3d_s exit_nor,
                  const obj_hdr_s* exit_obj,
                  const obj_hdr_s* enter_obj,
                  sz_t depth,
                  f3_t intensity )
{
    cl_s lum = { 0, 0, 0 };
    if( depth == 0 || intensity < scene->trace_min_intensity ) return lum;

    v3d_s pos = ray_s_pos( ray, offs );

    if( enter_obj && enter_obj->prp.radiance > 0 )
    {
        f3_t diff_sqr = v3d_s_diff_sqr( pos, enter_obj->prp.pos );
        f3_t light_intensity = ( diff_sqr > 0 ) ? ( enter_obj->prp.radiance / diff_sqr ) : f3_mag;
        return v3d_s_mlf( obj_color( enter_obj, pos ), light_intensity * intensity );
    }

    f3_t trans_refractive_index = 1.0;
    f3_t fresnel_reflectivity = 0;
    f3_t chromatic_reflectivity = 0;
    f3_t diffuse_reflectivity = 0;
    bl_t transparent = false;

    if( enter_obj )
    {
        trans_refractive_index = enter_obj->prp.refractive_index;
        fresnel_reflectivity   = enter_obj->prp.fresnel_reflectivity && enter_obj->prp.refractive_index != 1.0;
        chromatic_reflectivity = enter_obj->prp.chromatic_reflectivity;
        diffuse_reflectivity   = enter_obj->prp.diffuse_reflectivity;
        transparent            = v3d_s_sqr( enter_obj->prp.transparency ) > 0;
    }

    if( exit_obj )
    {
        trans_refractive_index /= exit_obj->prp.refractive_index;
        fresnel_reflectivity = 1.0;
        diffuse_reflectivity = chromatic_reflectivity = 0;
        transparent = true;
    }

    /// fresnel reflection
    if( fresnel_reflectivity > 0 && intensity >= scene->trace_min_intensity )
    {
        ray_s out;
        out.p = pos;
        f3_t reflectance = fresnel_reflection( ray->d, exit_nor, trans_refractive_index, &out.d ) * fresnel_reflectivity;
        v3d_s hit_exit_nor;
        vc_t  hit_exit_obj = NULL;
        vc_t  hit_enter_obj = NULL;
        f3_t a;
        cl_s lum_l = { 0, 0, 0 };
        if ( ( a = scene_s_trans_hit( scene, &out, &hit_exit_nor, &hit_exit_obj, &hit_enter_obj ) ) < f3_inf )
        {
            lum_l = scene_s_lum( scene, &out, a, hit_exit_nor, hit_exit_obj, hit_enter_obj, depth - 1, reflectance * intensity );
        }
        else
        {
            lum_l = v3d_s_mlf( scene->background_color, reflectance * intensity );
        }
        lum = v3d_s_add( lum, lum_l );

        intensity *= ( 1.0 - reflectance );
    }

    /// chromatic reflection
    if( chromatic_reflectivity > 0 && intensity >= scene->trace_min_intensity )
    {
        ray_s out;
        out.p = pos;
        out.d = v3d_s_reflection( ray->d, exit_nor );
        v3d_s hit_exit_nor;
        vc_t  hit_exit_obj = NULL, hit_enter_obj = NULL;
        f3_t a;
        cl_s lum_l = { 0, 0, 0 };
        if ( ( a = scene_s_trans_hit( scene, &out, &hit_exit_nor, &hit_exit_obj, &hit_enter_obj ) ) < f3_inf )
        {
            lum_l = scene_s_lum( scene, &out, a, hit_exit_nor, hit_exit_obj, hit_enter_obj, depth - 1, chromatic_reflectivity * intensity );
        }
        else
        {
            lum_l = v3d_s_mlf( scene->background_color, chromatic_reflectivity * intensity );
        }

        cl_s cl = obj_color( enter_obj, pos );
        lum_l.x *= cl.x;
        lum_l.y *= cl.y;
        lum_l.z *= cl.z;
        lum = v3d_s_add( lum, lum_l );

        intensity *= ( 1.0 - chromatic_reflectivity );
    }

    /// diffuse reflection
    if( intensity * diffuse_reflectivity >= scene->trace_min_intensity )
    {
        f3_t diffuse_intensity = intensity * diffuse_reflectivity;

        ray_s surface = { .p = pos, .d = v3d_s_neg( exit_nor ) };

        /// random seed
        u2_t rv = surface.p.x * 3294479285 +
                  surface.p.y * 7640304827 +
                  surface.p.z * 2434937345 +
                  surface.d.x * 3247146734 +
                  surface.d.y * 4304627463 +
                  surface.d.z * 5210473891;

        cl_s lum_l = { 0, 0, 0 };

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
            sz_t direct_samples = scene->direct_samples * diffuse_intensity;
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

                    cl_sum = v3d_s_add( cl_sum, v3d_s_mlf( color, local_intensity * weight * diffuse_intensity ) );
                }
            }

            bcore_arr_sz_s_discard( idx_arr );

            // factor 2 arises from weight distribution across the half-sphere
            lum_l = v3d_s_add( lum_l, v3d_s_mlf( cl_sum, 2.0 * cyl_hgt / direct_samples ) );

        }

        // path tracing
        if( scene->path_samples && depth > 10 )
        {
            cl_s cl_sum = { 0, 0, 0 };
            ray_s out = surface;
            m3d_s out_con = m3d_s_transposed( m3d_s_con_z( surface.d ) );

            f3_t per_energy = v3d_s_sqr( lum_l );
            per_energy = per_energy > 0.01 ? per_energy : 0.01;

            sz_t path_samples = scene->path_samples * diffuse_intensity;
            path_samples = ( path_samples == 0 ) ? 1 : path_samples;

            for( sz_t i = 0; i < path_samples; i++ )
            {
                out.d = m3d_s_mlv( &out_con, v3d_s_rsc( &rv, 1.0 ) );
                f3_t weight = v3d_s_mlv( out.d, surface.d );
                if( weight <= 0 ) continue;
                v3d_s hit_exit_nor;
                vc_t  hit_exit_obj = NULL;
                vc_t  hit_enter_obj = NULL;

                f3_t a = compound_s_ray_trans_hit( &scene->matter, &out, &hit_exit_nor, &hit_exit_obj, &hit_enter_obj );

                if( a < scene->max_path_length )
                {
                    cl_s lum = scene_s_lum( scene, &out, a, hit_exit_nor, hit_exit_obj, hit_enter_obj, depth - 10, weight * diffuse_intensity );
                    cl_sum = v3d_s_add( cl_sum, lum );
                }
                else
                {
                    cl_sum = v3d_s_add( cl_sum, v3d_s_mlf( scene->background_color, weight * diffuse_intensity ) );
                }
            }
            // factor 2 arises from weight distribution across the half-sphere
            lum_l = v3d_s_add( lum_l, v3d_s_mlf( cl_sum, 2.0 / path_samples ) );
        }

        cl_s cl = obj_color( enter_obj, pos );
        lum_l.x *= cl.x;
        lum_l.y *= cl.y;
        lum_l.z *= cl.z;
        lum = v3d_s_add( lum, lum_l );

        intensity *= ( 1.0 - diffuse_reflectivity );
    }

    /// refraction
    if( transparent && intensity >= scene->trace_min_intensity )
    {
        ray_s out;
        out.p = ray_s_pos( ray, offs + 2.0 * f3_eps );
        fresnel_refraction( ray->d, exit_nor, trans_refractive_index, &out.d );
        v3d_s hit_exit_nor;
        vc_t  hit_exit_obj = NULL;
        vc_t  hit_enter_obj = NULL;
        f3_t a;
        cl_s lum_l = { 0, 0, 0 };
        if ( ( a = scene_s_trans_hit( scene, &out, &hit_exit_nor, &hit_exit_obj, &hit_enter_obj ) ) < f3_inf )
        {
            lum_l = scene_s_lum( scene, &out, a, hit_exit_nor, hit_exit_obj, hit_enter_obj, depth - 1, intensity );
        }
        else
        {
            lum_l = v3d_s_mlf( scene->background_color, intensity );
        }
        lum = v3d_s_add( lum, lum_l );
    }

    /// exiting object
    if( exit_obj )
    {
        f3_t rf = offs > 0 ? pow( exit_obj->prp.transparency.x, offs ) : 1.0;
        f3_t gf = offs > 0 ? pow( exit_obj->prp.transparency.y, offs ) : 1.0;
        f3_t bf = offs > 0 ? pow( exit_obj->prp.transparency.z, offs ) : 1.0;
        lum.x *= rf;
        lum.y *= gf;
        lum.z *= bf;
    }

    return lum;
}

/**********************************************************************************************************************/

void scene_s_clear( scene_s* o )
{
    compound_s_clear( &o->light );
    compound_s_clear( &o->matter );
}

/**********************************************************************************************************************/

// luminance at a given position
typedef struct lum_s
{
    v2d_s pos;
    cl_s  clr;
    f3_t  weight;
} lum_s;

DEFINE_FUNCTIONS_OBJ_INST( lum_s )
DEFINE_CREATE_SELF( lum_s,  "lum_s = bcore_inst { v2d_s pos; cl_s clr; f3_t weight = 1.0; }" )

tp_t lum_s_key( const lum_s* o )
{
    s2_t x = o->pos.x;
    s2_t y = o->pos.y;
    return bcore_tp_fold_u2( bcore_tp_fold_u2( bcore_tp_init(), x ), y );
}

cl_s lum_s_avg_clr( const lum_s* o )
{
    return v3d_s_mlf( o->clr, 1.0 / o->weight );
}

lum_s lum_s_add( const lum_s* o1, const lum_s* o2 )
{
    lum_s sum;
    lum_s_init( &sum );
    sum.pos = v2d_s_add( o1->pos, o2->pos );
    sum.clr = v3d_s_add( o1->clr, o2->clr );
    sum.weight = o1->weight + o2->weight;
    return sum;
}

/**********************************************************************************************************************/

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

/// sets position and initializes other values with defaults
void lum_arr_s_push_pos( lum_arr_s* o, v2d_s pos )
{
    lum_s lum;
    lum_s_init( &lum );
    lum.pos = pos;
    lum_arr_s_push( o, lum );
}

/**********************************************************************************************************************/

/// image of lum_s
#define TYPEOF_lum_image_s typeof( "lum_image_s" )
typedef struct lum_image_s
{
    aware_t _;
    sz_t width;
    sz_t height;
    lum_arr_s arr;
} lum_image_s;

DEFINE_FUNCTIONS_OBJ_INST( lum_image_s )
DEFINE_CREATE_SELF( lum_image_s,  "lum_image_s = bcore_inst { aware_t _; sz_t width; sz_t height; lum_arr_s arr; }" )

void lum_image_s_reset( lum_image_s* o, sz_t width, sz_t height )
{
    bcore_array_aware_set_size( &o->arr, width * height );
    for( sz_t i = 0; i < o->arr.size; i++ )
    {
        o->arr.data[ i ].clr = ( cl_s ) { 0, 0, 0 };
        o->arr.data[ i ].pos = ( v2d_s ) { 0, 0 };
        o->arr.data[ i ].weight = 0;
    }
    o->width = width;
    o->height = height;
}

void lum_image_s_push( lum_image_s* o, lum_s lum )
{
    s2_t x = lum.pos.x / lum.weight;
    s2_t y = lum.pos.y / lum.weight;
    if( x >= 0 && x < o->width && y >= 0 && y < o->height )
    {
        sz_t idx = y * o->width + x;
        o->arr.data[ idx ] = lum_s_add( &o->arr.data[ idx ], &lum );
    }
}

void lum_image_s_push_arr( lum_image_s* o, const lum_arr_s* lum_arr )
{
    for( sz_t i = 0; i < lum_arr->size; i++ ) lum_image_s_push( o, lum_arr->data[ i ] );
}

lum_s lum_image_s_get_avg( const lum_image_s* o, s2_t x, s2_t y )
{
    lum_s lum = { .pos = { 0, 0 }, .clr = { 0, 0, 0 }, .weight = 0 };
    if( x >= 0 && x < o->width && y >= 0 && y < o->height )
    {
        sz_t idx = y * o->height + x;
        lum = o->arr.data[ idx ];
    }

    f3_t f = ( lum.weight > 0 ) ? 1.0 / lum.weight : 1.0;
    return ( lum_s ) { .pos = v2d_s_mlf( lum.pos, f ), .clr = v3d_s_mlf( lum.clr, f ), .weight = 1.0 };
}

f3_t lum_image_s_clr_dev( const lum_image_s* o, v3d_s ref, s3_t x, s3_t y )
{
    if( x < 0 || x >= o->width  ) return 0;
    if( y < 0 || y >= o->height ) return 0;
    return v3d_s_sqr( v3d_s_sub( ref, lum_image_s_get_avg( o, x, y ).clr ) );
}

f3_t lum_image_s_sqr_grad( const lum_image_s* o, s3_t x, s3_t y )
{
    f3_t g0 = 0;
    f3_t g1 = 0;
    v3d_s v = lum_image_s_get_avg( o, x, y ).clr;
    g1 = lum_image_s_clr_dev( o, v, x - 1, y - 1 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_image_s_clr_dev( o, v, x - 1, y + 0 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_image_s_clr_dev( o, v, x - 1, y + 1 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_image_s_clr_dev( o, v, x + 0, y - 1 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_image_s_clr_dev( o, v, x + 0, y + 1 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_image_s_clr_dev( o, v, x + 1, y - 1 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_image_s_clr_dev( o, v, x + 1, y + 0 ); g0 = g1 > g0 ? g1 : g0;
    g1 = lum_image_s_clr_dev( o, v, x + 1, y + 1 ); g0 = g1 > g0 ? g1 : g0;
    return g0;
}

void lum_image_s_create_image_file( const lum_image_s* o, sc_t file )
{
    image_cl_s* image = image_cl_s_create();
    image_cl_s_set_size( image, o->width, o->height, cl_black() );
    image_cps_s* image_cps = image_cps_s_create();

    for( sz_t j = 0; j < o->height; j++ )
    {
        for( sz_t i = 0; i < o->width; i++ )
        {
            image_cl_s_set_pixel( image, i, j, lum_image_s_get_avg( o, i, j ).clr );
        }
    }
    image_cps_s_copy_cl( image_cps, image );
    image_cps_s_write_pnm( image_cps, file );
    st_s_print_fa( " hash: #<tp_t>", image_cps_s_hash( image_cps ) );

    image_cps_s_discard( image_cps );
    image_cl_s_discard( image );
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

        cl_s out_clr = o->scene->background_color;

        v3d_s exit_nor;
        vc_t  exit_obj = NULL;
        vc_t  enter_obj = NULL;
        f3_t offs = scene_s_trans_hit( o->scene, &ray, &exit_nor, &exit_obj, &enter_obj );
        if( offs < f3_inf )
        {
            if( o->scene->experimental_level == 0 )
            {
                out_clr = scene_s_lum( o->scene, &ray, offs, exit_nor, exit_obj, enter_obj, o->scene->trace_depth, 1.0 );
            }
            else
            {
                bcore_err_fa( "Unsupported experimental level #<s3_t>\n", o->scene->experimental_level );
            }
        }

        lum->clr = cl_s_sat( out_clr, o->scene->gamma );
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

    lum_arr_s* lum_arr = bcore_life_s_push_aware( l, lum_arr_s_create() );

    st_s_print_fa( "Rendering ...\n" );
    clock_t time = clock();
    st_s_print_fa( "\tmain image: " );

    for( sz_t j = 0; j < o->image_height; j++ )
    {
        for( sz_t i = 0; i < o->image_width; i++ )
        {
            lum_arr_s_push_pos( lum_arr, ( v2d_s ){ i + 0.5, j + 0.5 } );
        }
    }

    lum_machine_s_run( o, lum_arr );
    lum_image_s* lum_image = bcore_life_s_push_aware( l, lum_image_s_create() );
    lum_image_s_reset( lum_image, o->image_width, o->image_height );
    lum_image_s_push_arr( lum_image, lum_arr );

    u2_t rv = 1234;
    sz_t rnd_samples = o->gradient_samples;
    f3_t sqr_gradient_theshold = f3_sqr( o->gradient_threshold );

    lum_image_s_create_image_file( lum_image, file );
    for( sz_t k = 0; k < o->gradient_cycles; k++ )
    {
        st_s_print_fa( "\n\tgradient pass #pl3 {#<sz_t>}: ", k + 1 );
        lum_arr_s_clear( lum_arr );
        for( s3_t j = 0; j < o->image_height; j++ )
        {
            for( s3_t i = 0; i < o->image_width; i++ )
            {
                if( lum_image_s_sqr_grad( lum_image, i, j ) > sqr_gradient_theshold )
                {
                    for( sz_t k = 0; k < rnd_samples; k++ )
                    {
                        f3_t dx = f3_rnd1( &rv );
                        f3_t dy = f3_rnd1( &rv );
                        lum_arr_s_push_pos( lum_arr, ( v2d_s ){ i + dx, j + dy } );
                    }
                }
            }
        }
        lum_machine_s_run( o, lum_arr );
        lum_image_s_push_arr( lum_image, lum_arr );
        lum_image_s_create_image_file( lum_image, file );

    }
    time = clock() - time;
    bcore_msg( "\n%5.3g cs\n", ( f3_t )time / ( CLOCKS_PER_SEC ) );

    bcore_life_s_discard( l );
}

/**********************************************************************************************************************/

vd_t scene_signal( tp_t target, tp_t signal, vd_t object )
{
    if( target != typeof( "all" ) && target != typeof( "scene" ) ) return NULL;

    if( signal == typeof( "init1" ) )
    {
        bcore_flect_define_creator( typeof( "scene_s"      ), scene_s_create_self );
        bcore_flect_define_creator( typeof( "image_cps_s"  ), image_cps_s_create_self );
        bcore_flect_define_creator( typeof( "lum_s"       ), lum_s_create_self     );
        bcore_flect_define_creator( typeof( "lum_arr_s"   ), lum_arr_s_create_self );
        bcore_flect_define_creator( typeof( "lum_image_s" ), lum_image_s_create_self );
    }

    return NULL;
}

/**********************************************************************************************************************/


