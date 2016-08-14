
#ifndef BOARD_HH
#define BOARD_HH

#include <avr/sfr_defs.h>
#include <avr/io.h>

//TODO MAYBE ALSO CHANGE ALL Board methods to return REGISTER?
// Useful class to handler register defined by constant address; those can be used in constexpr variables
class REGISTER
{
public:
	constexpr REGISTER(const REGISTER& rhs):ADDR(rhs.ADDR) {}
	constexpr REGISTER(uint8_t ADDR):ADDR(ADDR) {}
	uint8_t io_addr() const
	{
		return ADDR - __SFR_OFFSET;
	}
	uint8_t mem_addr() const
	{
		return ADDR;
	}
	operator volatile uint8_t& () const
	{
		return *((volatile uint8_t*) (uint16_t) ADDR);
	}
	operator volatile uint16_t& () const
	{
		return *((volatile uint16_t*) (uint16_t) ADDR);
	}
private:	
	const uint8_t ADDR;
};

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
