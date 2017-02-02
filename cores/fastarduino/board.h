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

#ifndef BOARD_HH
#define BOARD_HH

#include <avr/sfr_defs.h>
#include <avr/io.h>

#include "utilities.h"

// This internal macro is used by individual boards headers
#define _SELECT_REG(REG) REGISTER((uint8_t)(uint16_t)&REG)

// Useful class to handler register defined by constant address; those can be used in constexpr variables
// Arduino Boards
#if defined(ARDUINO_MEGA)
#include "boards/mega.h"
#elif defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#include "boards/uno.h"

// Breadboards
#elif defined(BREADBOARD_ATTINYX4)
#include "boards/attiny_x4.h"

#else
#error "board.h: board not supported"
#endif

#endif /* BOARD_HH */
