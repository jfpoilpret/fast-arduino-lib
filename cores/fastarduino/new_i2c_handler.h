//   Copyright 2016-2020 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/// @cond api

/**
 * @file 
 * Common I2C Manager API. This will automatically include the proper header,
 * based on target architecture, ATmega or ATtiny.
 */
#ifndef I2C_HANDLER_HH
#define I2C_HANDLER_HH

//FIXME add specific define for true async I2C or not
#ifdef TWCR
#include "new_i2c_handler_atmega.h"
#else
#include "new_i2c_handler_attiny.h"
#endif

#endif /* I2C_HANDLER_HH */

/// @endcond
