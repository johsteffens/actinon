/** Closures */

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


#ifndef CLOSURES_H
#define CLOSURES_H

#include "bclos_spect_closure.h"
#include "bclos_quicktypes.h"
#include "bcore_quicktypes.h"

#include "quicktypes.h"

/**********************************************************************************************************************/

vd_t closures_signal( tp_t target, tp_t signal, vd_t object );

#endif // CLOSURES_H
