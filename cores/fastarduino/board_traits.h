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

// Arduino Boards
#if defined(ARDUINO_MEGA)
#include "boards/mega_traits.h"
#elif defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#include "boards/uno_traits.h"

// Breadboards
#elif defined(BREADBOARD_ATTINYX4)
#include "boards/attiny_x4_traits.h"

#else
#error "board_traits.h: board not supported"
#endif

#endif /* BOARD_TRAITS_HH */
