
#ifndef BOARD_HH
#define BOARD_HH

#include <avr/sfr_defs.h>
#include <avr/io.h>

#include "utilities.hh"

#define _SELECT_REG(REG) REGISTER((uint8_t)(uint16_t)&REG)


//TODO MAYBE ALSO CHANGE ALL Board methods to return REGISTER?
// Useful class to handler register defined by constant address; those can be used in constexpr variables
// Arduino Boards
#if defined(ARDUINO_MEGA)
#include <fastarduino/boards/Mega.hh>
#elif defined(ARDUINO_UNO)
#include <fastarduino/boards/Uno.hh>

// Breadboards
#elif defined(BREADBOARD_ATTINYX4)
#include <fastarduino/boards/ATtinyX4.hh>
#elif defined(BREADBOARD_ATMEGA328P)
#include <fastarduino/boards/ATmega328P.hh>

#else
#error "Board.hh: board not supported"
#endif

#endif /* BOARD_HH */
