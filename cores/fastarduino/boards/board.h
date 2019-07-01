//   Copyright 2016-2019 Jean-Francois Poilpret
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

#ifndef BOARDS_BOARD_HH
#define BOARDS_BOARD_HH

#include <stdint.h>

// Arduino Boards
#if defined(ARDUINO_MEGA)
#include "mega.h"
#elif defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#include "uno.h"
#elif defined(ARDUINO_LEONARDO)
#include "leonardo.h"
#elif defined(ARDUINO_NANO)
#define HAS_8_ANALOG_INPUTS
#include "uno.h"

// Breadboards
#elif defined(BREADBOARD_ATTINYX4)
#include "attiny_x4.h"
#elif defined(BREADBOARD_ATTINYX5)
#include "attiny_x5.h"

#else
#error "board.h: board not supported"
#endif

// General constants used everywhere
static constexpr const uint32_t ONE_MHZ = 1'000'000UL;
static constexpr const uint32_t ONE_SECOND = 1'000'000UL;
static constexpr const uint32_t INST_PER_US = F_CPU / ONE_MHZ;
static constexpr const uint16_t ONE_MILLI_16 = 1000U;
static constexpr const uint32_t ONE_MILLI_32 = 1000UL;

#endif /* BOARDS_BOARD_HH */
