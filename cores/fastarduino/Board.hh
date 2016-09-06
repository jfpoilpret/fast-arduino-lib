
#ifndef BOARD_HH
#define BOARD_HH

#include <avr/sfr_defs.h>
#include <avr/io.h>

#include "utilities.hh"

// This internal macro is used by individual boards headers
#define _SELECT_REG(REG) REGISTER((uint8_t)(uint16_t)&REG)

// Useful class to handler register defined by constant address; those can be used in constexpr variables
// Arduino Boards
#if defined(ARDUINO_MEGA)
#include "boards/Mega.hh"
#elif defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#include "boards/Uno.hh"

// Breadboards
#elif defined(BREADBOARD_ATTINYX4)
#include "boards/ATtinyX4.hh"

#else
#error "Board.hh: board not supported"
#endif

#endif /* BOARD_HH */
