//   Copyright 2016-2021 Jean-Francois Poilpret
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

#ifndef BOARDS_BOARD_TRAITS_HH
#define BOARDS_BOARD_TRAITS_HH

#include "io.h"

// Arduino Boards
#if defined(ARDUINO_MEGA)
#include "mega_traits.h"
#elif defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#include "uno_traits.h"
#elif defined(ARDUINO_LEONARDO)
#include "leonardo_traits.h"
#elif defined(ARDUINO_NANO)
#define HAS_8_ANALOG_INPUTS
#include "uno_traits.h"

// Breadboards
#elif defined(BREADBOARD_ATTINYX4)
#include "attiny_x4_traits.h"
#elif defined(BREADBOARD_ATTINYX5)
#include "attiny_x5_traits.h"
#elif defined(BREADBOARD_ATMEGAXX4P)
#include "atmega_xx4_traits.h"

#else
#error "board_traits.h: board not supported"
#endif

#endif /* BOARDS_BOARD_TRAITS_HH */
