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

// Quickly obtainable types.

#ifndef QUICKTYPES_H
#define QUICKTYPES_H

#include "bcore_std.h"

/// This function computes and formats defines below to stdout
void quicktypes_to_stdout( tp_t (*hash)( sc_t name ) );

/** List of predefined types for quick access.
 *  Created via quicktypes_to_stdout( NULL );
 */

#define TYPEOF_v3d_s 0xE882C215ED7DEF80ull
#define TYPEOF_m3d_s 0x8E932846245F8D19ull
#define TYPEOF_spect_txm_s 0xFBED5C44E2235FE0ull
#define TYPEOF_txm_plain_s 0xDE52022D7BAD5DDDull
#define TYPEOF_txm_chess_s 0xC3AD2172884E128Dull
#define TYPEOF_scene_s 0x4004287ACF60A435ull
#define TYPEOF_spect_obj_s 0x0D7850F1CE75B7A2ull
#define TYPEOF_spect_obj 0x107E819E5675C418ull
#define TYPEOF_obj_plane_s 0x3CA9D7EF68D58921ull
#define TYPEOF_obj_sphere_s 0x1B66B59BF27F6AF4ull
#define TYPEOF_obj_squaroid_s 0x1C09C98B1CC4819Dull
#define TYPEOF_obj_distance_s 0x4512317AA9CB8D82ull
#define TYPEOF_obj_pair_inside_s 0xBB012DBE14809C26ull
#define TYPEOF_obj_pair_outside_s 0xBF9443641B89C009ull
#define TYPEOF_obj_neg_s 0x3FE037BB5EE2BAB9ull
#define TYPEOF_obj_scale_s 0x98B7FDE98C7910C9ull
#define TYPEOF_properties_s 0xBF0C82AF7675A3BEull
#define TYPEOF_compound_s 0x13D78EFEE85438FEull
#define TYPEOF_meval_s 0xDA4D919A7E8B2860ull
#define TYPEOF_mclosure_s 0x1994E566CD0CBBCFull
#define TYPEOF_arr_s 0x4FCE12B634EFD082ull
#define TYPEOF_map_s 0xA25336A4EBB6BC9Bull
#define TYPEOF_lum_s 0xB135A593475BA637ull
#define TYPEOF_lum_arr_s 0xCEF8C9E4E3A7481Full
#define TYPEOF_lum_map_s 0x336665326AE49E82ull
#define TYPEOF_clear 0xF531F89544A910A2ull
#define TYPEOF_push 0x6C80030E2762459Dull
#define TYPEOF_move 0x0D0F17A2C3F687B4ull
#define TYPEOF_rotate 0x075CB2F53173BD8Aull
#define TYPEOF_scale 0x6AACB9FBB71A1D91ull
#define TYPEOF_true 0x5B5C98EF514DBFA5ull
#define TYPEOF_false 0xB5FAE2C14238B978ull
#define TYPEOF_NOT 0x36F71619C2EB102Aull
#define TYPEOF_AND 0xFA069719A050BE66ull
#define TYPEOF_OR 0x091D3407B5B2F824ull
#define TYPEOF_XOR 0xD2B8F21A1B184EDEull
#define TYPEOF_CAT 0x0B7F0719AA40A747ull
#define TYPEOF_def 0xCA9A1A18F461E4CCull
#define TYPEOF_if 0x08B73007B55C3E26ull
#define TYPEOF_while 0xCE87A3885811296Eull
#define TYPEOF_else 0x7F2B6C605332DD30ull
#define TYPEOF_for 0xDCB27818FED9DA90ull
#define TYPEOF_in 0x08B73807B55C4BBEull
#define TYPEOF_of 0x08B06007B5567108ull
#define TYPEOF_vec 0x691201194EDD7385ull
#define TYPEOF_set_color 0xE77914DC9BB2A77Bull
#define TYPEOF_set_refractive_index 0x5E58CFFB07A0A9C0ull
#define TYPEOF_set_radiance 0x12AC2A68C421142Dull
#define TYPEOF_set_transparency 0x577A4D311AA5B072ull
#define TYPEOF_set_texture_field 0xB01B0A0B7745E03Cull
#define TYPEOF_create_image 0x5065BA8F5FA43FCBull

#endif // QUICKTYPES_H

