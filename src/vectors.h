/** Vectors and Related Objects */

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

#ifndef VECTORS_H
#define VECTORS_H

#include <math.h>
#include "bcore_std.h"

#include "quicktypes.h"

/**********************************************************************************************************************/

/**********************************************************************************************************************/
// consts and types
#define f3_inf INFINITY
///1E300 // pseudo-infinity
#define f3_mag 1E+30 // very large number
#define f3_eps 1E-6  // epsilon (used to simulate a miniscule shell-thickness of the surface to prevent near-degenerate conditions)

#ifndef M_PI
#define M_PI 3.141592654
#endif // M_PI

/**********************************************************************************************************************/
/// f3_t
static inline f3_t f3_sqr( f3_t v ) { return v * v; }
static inline f3_t f3_abs( f3_t v ) { return v < 0 ? -v : v; }
static inline f3_t f3_max( f3_t v1, f3_t v2  ) { return v1 > v2 ? v1 : v2; }
static inline f3_t f3_min( f3_t v1, f3_t v2  ) { return v1 < v2 ? v1 : v2; }

// Note: generators xsg, xsg2 show strong hyper-structures on polar coordinates

/// random generator (range -1, 1)
static inline f3_t f3_rnd0( u2_t* rv ) { return ( *rv = bcore_xsg1_u2( *rv ) ) * ( 2.0 / 0xFFFFFFFFu ) - 1.0; }

/// random generator (range 0, 1)
static inline f3_t f3_rnd1( u2_t* rv ) { return ( *rv = bcore_xsg1_u2( *rv ) ) * ( 1.0 / 0xFFFFFFFFu ); }

/**********************************************************************************************************************/

/// v2d_s (2D Vector)
typedef struct v2d_s { f3_t x; f3_t y; } v2d_s;
BCORE_DECLARE_FUNCTIONS_OBJ( v2d_s )

/// zero
static inline v2d_s v2d_s_zero() { return ( v2d_s ) { .x = 0, .y = 0 }; }

/// negation
static inline v2d_s v2d_s_neg( v2d_s o ) { return ( v2d_s ) { .x = -o.x , .y = -o.y }; }

/// o * o
static inline f3_t v2d_s_sqr( v2d_s o ) { return (o.x*o.x) + (o.y*o.y); }

/// return o 'op' s
static inline v2d_s v2d_s_add( v2d_s o, v2d_s s ) { return ( v2d_s ) { .x = (o.x+s.x), .y = (o.y+s.y) }; }
static inline v2d_s v2d_s_sub( v2d_s o, v2d_s s ) { return ( v2d_s ) { .x = (o.x-s.x), .y = (o.y-s.y) }; }
static inline v2d_s v2d_s_mlf( v2d_s o, f3_t f  ) { return ( v2d_s ) { .x = (o.x*f)  , .y = (o.y*f)   }; }

/// o = o 'op' s
static inline void v2d_s_o_add( v2d_s* o, v2d_s s ) { o->x += s.x; o->y += s.y; }
static inline void v2d_s_o_sub( v2d_s* o, v2d_s s ) { o->x -= s.x; o->y -= s.y; }
static inline void v2d_s_o_mlf( v2d_s* o, f3_t f  ) { o->x *= f  ; o->y *= f  ; }

/// o1 * o2
static inline f3_t v2d_s_mlv( v2d_s o, v2d_s m ) { return (o.x*m.x) + (o.y*m.y); }

/// ( o 'op' s ) * m
static inline f3_t v2d_s_add_mlv( v2d_s o, v2d_s s, v2d_s m ) { return ((o.x+s.x)*m.x) + ((o.y+s.y)*m.y); }
static inline f3_t v2d_s_sub_mlv( v2d_s o, v2d_s s, v2d_s m ) { return ((o.x-s.x)*m.x) + ((o.y-s.y)*m.y); }

static inline f3_t v2d_s_diff_sqr( v2d_s o, v2d_s v ) { return f3_sqr( o.x - v.x ) + f3_sqr( o.y - v.y ); }

/// max/min value
static inline f3_t v2d_s_max( v2d_s o ) { return ( o.x > o.y ) ? o.x : o.y; }
static inline f3_t v2d_s_min( v2d_s o ) { return ( o.x < o.y ) ? o.x : o.y; }

/// sets length of vector to abs(a) (negative a inverts vector's direction)
static inline v2d_s v2d_s_of_lenth( v2d_s o, f3_t a )
{
    f3_t r = sqrt( v2d_s_sqr( o ) );
    f3_t f = r > 0 ? ( a / r ) : 0;
    return ( v2d_s ){ .x=o.x*f, .y=o.y*f };
}

// canonic orthonormal to o
static inline v2d_s v2d_s_con( v2d_s o )
{
    v2d_s n0 = v2d_s_of_lenth( o, 1.0 );
    return ( v2d_s ) { .x = -n0.y, .y = n0.x };
}

/**********************************************************************************************************************/

/// v3d_s (3D Vector)
typedef struct v3d_s { f3_t x; f3_t y; f3_t z; } v3d_s;
BCORE_DECLARE_FUNCTIONS_OBJ( v3d_s )

/// zero
static inline v3d_s v3d_s_zero() { return ( v3d_s ) { .x = 0, .y = 0, .z = 0 }; }

/// negation
static inline v3d_s v3d_s_neg( v3d_s o ) { return ( v3d_s ) { .x = -o.x, .y = -o.y, .z = -o.z }; }

/// o * o
static inline f3_t v3d_s_sqr( v3d_s o ) { return (o.x*o.x) + (o.y*o.y) + (o.z*o.z); }

/// return o 'op' s
static inline v3d_s v3d_s_add( v3d_s o, v3d_s s ) { return ( v3d_s ) { .x = (o.x+s.x), .y = (o.y+s.y), .z = (o.z+s.z) }; }
static inline v3d_s v3d_s_sub( v3d_s o, v3d_s s ) { return ( v3d_s ) { .x = (o.x-s.x), .y = (o.y-s.y), .z = (o.z-s.z) }; }
static inline v3d_s v3d_s_mlf( v3d_s o, f3_t f  ) { return ( v3d_s ) { .x = (o.x*f)  , .y = (o.y*f)  , .z = (o.z*f) }; }

/// o x f (cross-product)
static inline v3d_s v3d_s_mlx( v3d_s o, v3d_s f ) { return ( v3d_s ) { .x = (o.y*f.z-o.z*f.y), .y = (o.z*f.x-o.x*f.z), .z = (o.x*f.y-o.y*f.x) }; }

/// o * diag_mat( f ) (f representing a diagonal matrix)
static inline v3d_s v3d_s_mld( v3d_s o, v3d_s f ) { return ( v3d_s ) { .x = o.x * f.x, .y = o.y * f.y, .z = o.z * f.z }; }

/// o = o 'op' s
static inline void v3d_s_o_add( v3d_s* o, v3d_s s ) { o->x += s.x; o->y += s.y; o->z += s.z; }
static inline void v3d_s_o_sub( v3d_s* o, v3d_s s ) { o->x -= s.x; o->y -= s.y; o->z -= s.z; }
static inline void v3d_s_o_mlf( v3d_s* o, f3_t f  ) { o->x *= f  ; o->y *= f  ; o->z *= f  ; }

/// o1 * o2
static inline f3_t v3d_s_mlv( v3d_s o, v3d_s m ) { return (o.x*m.x) + (o.y*m.y) + (o.z*m.z); }

/// ( o - s ) * m
static inline f3_t v3d_s_sub_mlv( v3d_s o, v3d_s s, v3d_s m ) { return ((o.x-s.x)*m.x) + ((o.y-s.y)*m.y) + ((o.z-s.z)*m.z); }

/// ( o - v )^2
static inline f3_t v3d_s_diff_sqr( v3d_s o, v3d_s v ) { return f3_sqr( o.x - v.x ) + f3_sqr( o.y - v.y ) + f3_sqr( o.z - v.z ); }

/// max/min value
static inline f3_t v3d_s_max( v3d_s o ) { f3_t v = ( o.x > o.y ) ? o.x : o.y; return ( v > o.z ) ? v : o.z; }
static inline f3_t v3d_s_min( v3d_s o ) { f3_t v = ( o.x < o.y ) ? o.x : o.y; return ( v < o.z ) ? v : o.z; }

/// sets length of vector to abs(a) (negative a inverts vector's direction)
static inline v3d_s v3d_s_of_length( v3d_s o, f3_t a )
{
    f3_t r_sqr = v3d_s_sqr( o );
    if( fabs( r_sqr - 1.0 ) < 1E-8 ) return o;
    f3_t f = r_sqr > 0 ? ( a / sqrt( r_sqr ) ) : 0;
    return ( v3d_s ){ .x=o.x*f, .y=o.y*f, .z=o.z*f };
}

/// makes vector v orthonormal to n inside the plane (o,v)
static inline v3d_s v3d_s_von( v3d_s o, v3d_s v )
{
    v3d_s o_n = v3d_s_of_length( o, 1.0 );
    v = v3d_s_sub( v, v3d_s_mlf( o_n, v3d_s_mlv( o_n, v ) ) );
    return v3d_s_of_length( v, 1.0 );
}

/// canonic orthonormal vector to o
static inline v3d_s v3d_s_con( v3d_s o )
{
    f3_t xx = o.x * o.x;
    f3_t yy = o.y * o.y;
    f3_t zz = o.z * o.z;
    v3d_s v;
    v.x = ( ( xx <= yy ) && ( xx <= zz ) ) ? 1 : 0;
    v.y = ( ( yy <= xx ) && ( yy <= zz ) ) ? 1 : 0;
    v.z = ( ( zz <= xx ) && ( zz <= yy ) ) ? 1 : 0;
    return v3d_s_von( o, v );
}

/// computes a seed value from vector
static inline u2_t v3d_s_random_seed( v3d_s o, u2_t rv )
{
    return o.x * bcore_lcg1_u2( rv ) + o.y * bcore_lcg2_u2( rv ) + o.z * bcore_lcg3_u2( rv );
}

/** Random generators with even distribution over a spherical cap of height h.
 *  z component points to cap. Method is derived from Archimedes's sphere-cylinder theorem.
 *  Thanks to H Kong (http://www.bogotobogo.com/Algorithms/uniform_distribution_sphere.php)
 *  for pointing this out.
 */
static inline v3d_s v3d_s_random_sphere_cap( u2_t* rv, f3_t h )
{
    v3d_s v;
    f3_t phi = 2.0 * M_PI * f3_rnd1( rv );
    v.z = 1.0 - f3_rnd1( rv ) * h;
    f3_t scale = sqrt( 1.0 - v.z * v.z );
    v.x = sin( phi ) * scale;
    v.y = cos( phi ) * scale;
    return v;
}

/// symmetric belt around unit-sphere (h indicates half-height of belt; h = 1: entire sphere)
static inline v3d_s v3d_s_random_sphere_belt( u2_t* rv, f3_t h )
{
    v3d_s v;
    f3_t phi = 2.0 * M_PI * f3_rnd1( rv );
    v.z = f3_rnd0( rv ) * h;
    f3_t scale = sqrt( 1.0 - v.z * v.z );
    v.x = sin( phi ) * scale;
    v.y = cos( phi ) * scale;
    return v;
}

/** Returns the orthogonal projection onto the plane with normale nor:
 *  o - nor * ( o * nor )
 */
static inline v3d_s v3d_s_orthogonal_projection( v3d_s o, v3d_s nor )
{
    f3_t f = v3d_s_mlv( o, nor );
    return ( v3d_s )
    {
        .x = o.x - nor.x * f,
        .y = o.y - nor.y * f,
        .z = o.z - nor.z * f
    };
}

/** Returns the reflected direction to dir according to surface given by normal 'nor'.
 *  Requirements: dir and nor must be normalized.
 *  The returned direction is normalized.
 */
static inline v3d_s v3d_s_reflection( v3d_s dir, v3d_s nor )
{
    return v3d_s_of_length( v3d_s_sub( dir, v3d_s_mlf( nor, 2.0 * v3d_s_mlv( dir, nor ) ) ), 1.0 );
}

/**********************************************************************************************************************/

/// m3d_s  (3x3 matrix)
typedef struct m3d_s { v3d_s x, y, z; } m3d_s;
BCORE_DECLARE_FUNCTIONS_OBJ( m3d_s )

// returns identity
static inline m3d_s m3d_s_ident()
{
    return ( m3d_s ) { .x = ( v3d_s ){ 1, 0, 0 }, .y = ( v3d_s ){ 0, 1, 0 }, .z = ( v3d_s ){ 0, 0, 1 } };
}

// o * v
static inline v3d_s m3d_s_mlv( const m3d_s* o, v3d_s v )
{
    return ( v3d_s )
    {
        .x = o->x.x * v.x + o->x.y * v.y + o->x.z * v.z,
        .y = o->y.x * v.x + o->y.y * v.y + o->y.z * v.z,
        .z = o->z.x * v.x + o->z.y * v.y + o->z.z * v.z
    };

}

// transposed( o ) * v
static inline v3d_s m3d_s_tmlv( const m3d_s* o, v3d_s v )
{
    return ( v3d_s )
    {
        .x = o->x.x * v.x + o->y.x * v.y + o->z.x * v.z,
        .y = o->x.y * v.x + o->y.y * v.y + o->z.y * v.z,
        .z = o->x.z * v.x + o->y.z * v.y + o->z.z * v.z
    };
}

static inline m3d_s m3d_s_mlm( const m3d_s* o, const m3d_s* a )
{
    return ( m3d_s ) { .x = m3d_s_mlv( o, a->x ), .y = m3d_s_mlv( o, a->y ), .z = m3d_s_mlv( o, a->z ) };
}

static inline m3d_s m3d_s_mlf( const m3d_s* o, f3_t f )
{
    return ( m3d_s ) { .x = v3d_s_mlf( o->x, f ), .y = v3d_s_mlf( o->y, f ), .z = v3d_s_mlf( o->z, f ) };
}

// returns rotation around x
static inline m3d_s m3d_s_rot_x( f3_t a )
{
    f3_t sa = sin( a ), ca = cos( a );
    return ( m3d_s ) { .x = ( v3d_s ){ 1, 0, 0 }, .y = ( v3d_s ){ 0, ca, -sa }, .z = ( v3d_s ){ 0, sa, ca } };
}

// returns rotation around y
static inline m3d_s m3d_s_rot_y( f3_t a )
{
    f3_t sa = sin( a ), ca = cos( a );
    return ( m3d_s ) { .x = ( v3d_s ){ ca, 0, sa }, .y = ( v3d_s ){ 0, 1, 0 }, .z = ( v3d_s ){ -sa, 0, ca } };
}

// returns rotation around z
static inline m3d_s m3d_s_rot_z( f3_t a )
{
    f3_t sa = sin( a ), ca = cos( a );
    return ( m3d_s ) { .x = ( v3d_s ){ ca, -sa, 0 }, .y = ( v3d_s ){ sa, ca, 0 }, .z = ( v3d_s ){ 0, 0, 1 } };
}

static inline m3d_s m3d_s_transposed( m3d_s o )
{
    return ( m3d_s ) { .x = ( v3d_s ){ o.x.x, o.y.x, o.z.x }, .y = ( v3d_s ){ o.x.y, o.y.y, o.z.y }, .z = ( v3d_s ){ o.x.z, o.y.z, o.z.z } };
}

/// creates a canonic orthonormal system from v with z-row parallel to v
static inline m3d_s m3d_s_con_z( v3d_s v )
{
    m3d_s m;
    m.z = v3d_s_of_length( v, 1.0 );
    m.x = v3d_s_con( v );
    m.y = v3d_s_mlx( m.z, m.x );
    return m;
}

/// creates a canonic orthonormal system from v with y-row parallel to v
static inline m3d_s m3d_s_con_y( v3d_s v )
{
    m3d_s m;
    m.y = v3d_s_of_length( v, 1.0 );
    m.z = v3d_s_con( v );
    m.x = v3d_s_mlx( m.y, m.z );
    return m;
}

/**********************************************************************************************************************/
/** ray_s
 *  A ray has a starting point 'p', and a direction 'd'.
 *  |d| == 1
 */

typedef struct ray_s { v3d_s p; v3d_s d; } ray_s;
BCORE_DECLARE_FUNCTIONS_OBJ( ray_s )

static inline ray_s ray_from_to( v3d_s src, v3d_s dst )
{
    return ( ray_s ) { .p = src, .d = v3d_s_of_length( v3d_s_sub( dst, src ), 1.0 ) };
}

static inline v3d_s ray_s_pos( const ray_s* o, f3_t offs )
{
    return v3d_s_add( o->p, v3d_s_mlf( o->d, offs ) );
}

/**********************************************************************************************************************/
/// ray_cone_s (defines a bundle of rays given by a principal ray and a radius at distance 1)

// cos_rs: cosine of angle between ray and surface

typedef struct ray_cone_s { ray_s ray; f3_t cos_rs; } ray_cone_s;
BCORE_DECLARE_FUNCTIONS_OBJ( ray_cone_s )

/// areal coverage (range 0...1: == height of unit-sphere-section)
static inline f3_t areal_coverage( f3_t cos_rs ) { return 1 - cos_rs; }

/**********************************************************************************************************************/
/// cl_s RGB Color expressed as 3D vector (x=red, y=green, z=blue)

#define TYPEOF_cl_s typeof( "cl_s" )
typedef v3d_s cl_s;
BCORE_DECLARE_FUNCTIONS_OBJ( cl_s )

/// saturated
static inline cl_s cl_s_sat( cl_s o, f3_t gamma )
{
    f3_t x = pow( o.x, gamma );
    f3_t y = pow( o.y, gamma );
    f3_t z = pow( o.z, gamma );

    // channel saturation
    x = x > 0.0 ? x < 1.0 ? x : 1.0 : 0.0;
    y = y > 0.0 ? y < 1.0 ? y : 1.0 : 0.0;
    z = z > 0.0 ? z < 1.0 ? z : 1.0 : 0.0;

    return ( cl_s ) { x, y, z };
}

static inline cl_s cl_black() { return ( cl_s ){ 0, 0, 0 }; }

/**********************************************************************************************************************/
/// row_cl_s    Row of cl_s

#define TYPEOF_row_cl_s typeof( "row_cl_s" )
typedef struct row_cl_s
{
    aware_t _;
    union
    {
        bcore_static_array_s arr;
        struct
        {
            cl_s* data;
            sz_t size, space;
        };
    };
} row_cl_s;

BCORE_DECLARE_FUNCTIONS_OBJ( row_cl_s )

void row_cl_s_set_size( row_cl_s* o, sz_t size, cl_s color );

static inline void row_cl_s_set_pixel( row_cl_s* o, sz_t x, cl_s cl )
{
    if( x < o->size ) o->data[ x ] = cl;
}

/**********************************************************************************************************************/
/// image_cl_s Image of cl_s

#define TYPEOF_image_cl_s typeof( "image_cl_s" )
typedef struct image_cl_s
{
    aware_t _;
    sz_t w, h; // width, height,
    union
    {
        bcore_static_array_s arr;
        struct
        {
            cl_s* data;
            sz_t size, space;
        };
    };
} image_cl_s;

BCORE_DECLARE_FUNCTIONS_OBJ( image_cl_s )

void image_cl_s_set_size( image_cl_s* o, sz_t w, sz_t h, cl_s color );

static inline void image_cl_s_set_pixel( image_cl_s* o, sz_t x, sz_t y, cl_s cl )
{
    if( x < o->w && y < o->h ) o->data[ y * o->w + x ] = cl;
}

static inline void image_cl_s_set_row( image_cl_s* o, sz_t y, const row_cl_s* row )
{
    for( sz_t i = 0; i < o->w; i++ )
    {
        if( i == row->size ) break;
        o->data[ y * o->w + i ] = row->data[ i ];
    }
}

static inline cl_s image_cl_s_get_pixel( const image_cl_s* o, sz_t x, sz_t y )
{
    return o->data[ y * o->w + x ];
}

static inline void image_cl_s_add_pixel( image_cl_s* o, sz_t x, sz_t y, cl_s cl )
{
    v3d_s_o_add( &o->data[ y * o->w + x ], cl );
}

/// finalizing the image: gamma & saturation
void image_cl_s_saturate( image_cl_s* o, f3_t gamma );

/**********************************************************************************************************************/

vd_t vectors_signal_handler( const bcore_signal_s* o );

#endif // VECTORS_H
