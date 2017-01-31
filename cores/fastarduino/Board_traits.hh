//   Copyright 2016-2017 Jean-Francois Poilpret
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

#ifndef BOARD_TRAITS_HH
#define BOARD_TRAITS_HH

#include <avr/sfr_defs.h>
#include <avr/io.h>

#include "utilities.hh"

// This internal macro is used by individual boards headers
#define _SELECT_REG(REG) REGISTER((uint8_t)(uint16_t)&REG)

// Useful class to handler register defined by constant address; those can be used in constexpr variables
// Arduino Boards
#if defined(ARDUINO_MEGA)
#include "boards/Mega_traits.hh"
#elif defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#include "boards/Uno_traits.hh"

// Breadboards
#elif defined(BREADBOARD_ATTINYX4)
#include "boards/ATtinyX4_traits.hh"

#else
#error "Board.hh: board not supported"
#endif

#endif /* BOARD_TRAITS_HH */
