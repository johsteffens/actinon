/** Distance Functions */

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

#ifndef DISTANCE_H
#define DISTANCE_H

#include "quicktypes.h"
#include "vectors.h"

typedef f3_t ( *distance_fp )( vc_t o, const v3d_s* pos );

/// header of distance object
typedef struct distance_hdr_s
{
    aware_t _;
    distance_fp fp_distance;
} distance_hdr_s;

/// distance function
static inline f3_t distance( vc_t o, const v3d_s* pos )
{
    return ( ( const distance_hdr_s* )o )->fp_distance( o, pos );
}

/**********************************************************************************************************************/

vd_t distance_signal( tp_t target, tp_t signal, vd_t object );

#endif // DISTANCE_H


